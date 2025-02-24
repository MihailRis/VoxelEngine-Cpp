#pragma once

#include <functional>
#include <unordered_map>

#include "Device.hpp"

namespace io {
    class ZipFileDevice : public Device {
        struct Entry {
            uint16_t versionMadeBy;
            uint16_t versionNeeded;
            uint16_t flags;
            uint16_t compressionMethod;
            uint16_t modTime;
            uint16_t modDate;
            uint32_t crc32;
            uint32_t compressedSize;
            uint32_t uncompressedSize;
            uint16_t diskNumberStart;
            uint16_t internalAttributes;
            uint32_t externalAttributes;
            uint32_t localHeaderOffset;
            std::string fileName;
            size_t blobOffset = 0;
            bool isDirectory = false;
        };
    public:
        using FileSeparateFunc = std::function<std::unique_ptr<std::istream>()>;

        /// @param file ZIP file seekable istream
        /// @param separateFunc Optional function that creates new seekable 
        /// istream for the ZIP file.
        ZipFileDevice(
            std::unique_ptr<std::istream> file,
            FileSeparateFunc separateFunc = nullptr
        );

        std::filesystem::path resolve(std::string_view path) override;
        std::unique_ptr<std::ostream> write(std::string_view path) override;
        std::unique_ptr<std::istream> read(std::string_view path) override;
        size_t size(std::string_view path) override;
        io::file_time_type lastWriteTime(std::string_view path) override;
        bool exists(std::string_view path) override;
        bool isdir(std::string_view path) override;
        bool isfile(std::string_view path) override;
        bool mkdir(std::string_view path) override;
        bool mkdirs(std::string_view path) override;
        bool remove(std::string_view path) override;
        uint64_t removeAll(std::string_view path) override;
        std::unique_ptr<PathsGenerator> list(std::string_view path) override;
    private:
        std::unique_ptr<std::istream> file;
        FileSeparateFunc separateFunc;
        std::unordered_map<std::string, Entry> entries;

        Entry readEntry();
        void findBlob(Entry& entry);
    };

    void write_zip(const path& folder, const path& file);
}
