#include "ZipFileDevice.hpp"

#include <vector>

#include "debug/Logger.hpp"
#include "io/memory_istream.hpp"
#include "io/deflate_istream.hpp"
#include "util/data_io.hpp"
#include "util/Buffer.hpp"

static debug::Logger logger("zip-file");

using namespace io;

static constexpr uint32_t EOCD_SIGNATURE = 0x06054b50;
static constexpr uint32_t CENTRAL_DIR_SIGNATURE = 0x02014b50;
static constexpr uint32_t LOCAL_FILE_SIGNATURE = 0x04034b50;
static constexpr uint32_t COMPRESSION_NONE = 0;
static constexpr uint32_t COMPRESSION_DEFLATE = 8;

template<typename T>
static T read_int(std::unique_ptr<std::istream>& file) {
    T value = 0;
    file->read(reinterpret_cast<char*>(&value), sizeof(value));
    return dataio::le2h(value);
}

template<typename T>
static void read_int(std::unique_ptr<std::istream>& file, T& value) {
    file->read(reinterpret_cast<char*>(&value), sizeof(value));
    value = dataio::le2h(value);
}

ZipFileDevice::Entry ZipFileDevice::readEntry() {
    // Read entry info
    Entry entry {};
    read_int(file, entry.versionMadeBy);
    read_int(file, entry.versionNeeded);
    read_int(file, entry.flags);
    read_int(file, entry.compressionMethod);
    read_int(file, entry.modTime);
    read_int(file, entry.modDate);
    read_int(file, entry.crc32);
    read_int(file, entry.compressedSize);
    read_int(file, entry.uncompressedSize);
    auto fileNameLength = read_int<uint16_t>(file);
    auto extraFieldLength = read_int<uint16_t>(file);
    auto fileCommentLength = read_int<uint16_t>(file);
    read_int(file, entry.diskNumberStart);
    read_int(file, entry.internalAttributes);
    read_int(file, entry.externalAttributes);
    read_int(file, entry.localHeaderOffset);

    entry.fileName.resize(fileNameLength, '\0');
    file->read(entry.fileName.data(), fileNameLength);

    // Skip extra field and file comment
    file->seekg(extraFieldLength + fileCommentLength, std::ios::cur);

    if (entry.diskNumberStart == 0xFF) {
        throw std::runtime_error("zip64 is not supported");
    }

    for (size_t i = 0; i < entry.fileName.length(); i++) {
        if (entry.fileName[i] == '\\') {
            entry.fileName[i] = '/';
        }
    }
    if (entry.fileName[entry.fileName.length() - 1] == '/') {
        entry.isDirectory = true;
        entry.fileName = entry.fileName.substr(0, entry.fileName.length() - 1);
    }
    return entry;
}

void ZipFileDevice::findBlob(Entry& entry) {
    file->seekg(entry.localHeaderOffset);
    if (read_int<uint32_t>(file) != LOCAL_FILE_SIGNATURE) {
        throw std::runtime_error("invalid local file signature");
    }
    read_int<uint16_t>(file); // version
    read_int<uint16_t>(file); // flags
    read_int<uint16_t>(file); // compression method
    read_int<uint16_t>(file); // last modification time
    read_int<uint16_t>(file); // last modification date
    read_int<uint32_t>(file); // crc32
    read_int<uint32_t>(file); // compressed size
    read_int<uint32_t>(file); // uncompressed size
    auto nameLength = read_int<uint16_t>(file);
    auto extraFieldLength = read_int<uint16_t>(file);

    // Skip extra field and file comment
    file->seekg(nameLength + extraFieldLength, std::ios::cur);
    entry.blobOffset = file->tellg();
}

ZipFileDevice::ZipFileDevice(
    std::unique_ptr<std::istream> filePtr, FileSeparateFunc separateFunc
)
    : file(std::move(filePtr)), separateFunc(std::move(separateFunc)) {
    // Searching for EOCD
    file->seekg(0, std::ios::end);
    std::streampos fileSize = file->tellg();

    bool foundEOCD = false;
    for (int pos = static_cast<int>(fileSize)-4; pos >= 0; --pos) {
        file->seekg(pos);
        if (read_int<uint32_t>(file) == EOCD_SIGNATURE) {
            foundEOCD = true;
            break;
        }
    }
    if (!foundEOCD) {
        throw std::runtime_error("EOCD not found, ZIP file is invalid");
    }

    // Reading EOCD
    read_int<uint16_t>(file); // diskNumber
    read_int<uint16_t>(file); // centralDirDisk
    read_int<uint16_t>(file); // numEntriesThisDisk
    auto totalEntries = read_int<uint16_t>(file);
    read_int<uint32_t>(file); // centralDirSize
    auto centralDirOffset = read_int<uint32_t>(file);
    read_int<uint16_t>(file); // commentLength

    file->seekg(centralDirOffset);

    for (uint16_t i = 0; i < totalEntries; i++) {
        if (read_int<uint32_t>(file) != CENTRAL_DIR_SIGNATURE) {
            logger.error() << "invalid central directory entry";
            break;
        }
        // Read entry info
        Entry entry = readEntry();
        entries[entry.fileName] = std::move(entry);
    }

    for (auto& [_, entry] : entries) {
        findBlob(entry);
    }
}

std::filesystem::path ZipFileDevice::resolve(std::string_view path) {
    throw std::runtime_error("unable to resolve filesystem path");
}

std::unique_ptr<std::ostream> ZipFileDevice::write(std::string_view path) {
    return nullptr;
}

std::unique_ptr<std::istream> ZipFileDevice::read(std::string_view path) {
    const auto& found = entries.find(std::string(path));
    if (found == entries.end()) {
        throw std::runtime_error("could not to open file zip://" + std::string(path));
    }
    auto& entry = found->second;
    if (entry.isDirectory) {
        throw std::runtime_error("zip://" + std::string(path) + " is directory");
    }
    if (entry.blobOffset == 0) {
        findBlob(entry);
    }
    std::unique_ptr<std::istream> srcStream;
    if (separateFunc) {
        // Create new istream for concurrent data reading
        srcStream = separateFunc();
        srcStream->seekg(entry.blobOffset);
    } else {
        // Read compressed data to memory if istream cannot be separated
        file->seekg(entry.blobOffset);
        util::Buffer<char> buffer(entry.compressedSize);
        file->read(buffer.data(), buffer.size());
        srcStream = std::make_unique<memory_istream>(std::move(buffer));
    }
    if (entry.compressionMethod == COMPRESSION_NONE) {
        return srcStream;
    } else if (entry.compressionMethod == COMPRESSION_DEFLATE) {
        return std::make_unique<deflate_istream>(std::move(srcStream));
    } else {
        throw std::runtime_error(
            "unsupported compression method [" +
            std::to_string(entry.compressionMethod) + "]"
        );
    }
}

size_t ZipFileDevice::size(std::string_view path) {
    const auto& found = entries.find(std::string(path));
    if (found == entries.end()) {
        return false;
    }
    return found->second.uncompressedSize;
}

bool ZipFileDevice::exists(std::string_view path) {
    return entries.find(std::string(path)) != entries.end();
}

bool ZipFileDevice::isdir(std::string_view path) {
    const auto& found = entries.find(std::string(path));
    if (found == entries.end()) {
        return false;
    }
    return found->second.isDirectory;
}

bool ZipFileDevice::isfile(std::string_view path) {
    const auto& found = entries.find(std::string(path));
    if (found == entries.end()) {
        return false;
    }
    return !found->second.isDirectory;
}

bool ZipFileDevice::mkdir(std::string_view path) {
    return false;
}

bool ZipFileDevice::mkdirs(std::string_view path) {
    return false;
}

bool ZipFileDevice::remove(std::string_view path) {
    return false;
}

uint64_t ZipFileDevice::removeAll(std::string_view path) {
    return 0;
}

class ListPathsGenerator : public PathsGenerator {
public:
    ListPathsGenerator(std::vector<std::string> names)
        : names(std::move(names)) {};

    bool next(path& dst) override {
        if (current == names.size()) {
            return false;
        }
        dst = names[current++];
        return true;
    }
private:
    std::vector<std::string> names;
    size_t current = 0;
};

std::unique_ptr<PathsGenerator> ZipFileDevice::list(std::string_view path) {
    std::vector<std::string> names;
    auto folder = std::string(path) + "/";
    size_t folderLen = folder.length();
    for (const auto& [name, entry] : entries) {
        if (name.find(folder) == 0) {
            size_t pos = name.find('/', folderLen);
            if (pos == std::string::npos) {
                names.push_back(name.substr(folderLen, pos - folderLen));
            }
            if (pos == name.length() - 1) {
                names.push_back(name.substr(folderLen, pos - folderLen));
            }
        }
    }
    return std::make_unique<ListPathsGenerator>(std::move(names));
}
