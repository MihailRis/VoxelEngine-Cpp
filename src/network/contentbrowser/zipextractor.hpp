#include <iostream>
#include <memory>
#include <string>
#include <archive.h>
#include <archive_entry.h>
#include <filesystem>

class ArchiveExtractor {
public:
    ArchiveExtractor(const std::string& filename)
        : filename(filename) {
        a = archive_read_new();
        archive_read_support_format_all(a);
        archive_read_support_filter_all(a);
        ext = archive_write_disk_new();
        setExtractionOptions();
    }

    ~ArchiveExtractor() {
        if (a) {
            archive_read_close(a);
            archive_read_free(a);
        }
        if (ext) {
            archive_write_close(ext);
            archive_write_free(ext);
        }
    }

    void extract() {
        if (archive_read_open_filename(a, filename.c_str(), 10240) != ARCHIVE_OK) {
            throw std::runtime_error("Failed to open archive: " + filename);
        }

        struct archive_entry* entry;
        while (true) {
            int r = archive_read_next_header(a, &entry);
            if (r == ARCHIVE_EOF) break;
            if (r < ARCHIVE_OK) {
                throw std::runtime_error(archive_error_string(a));
            }

            std::string outputPath = getOutputPath(entry);
            archive_entry_set_pathname(entry, outputPath.c_str());

            r = archive_write_header(ext, entry);
            if (r < ARCHIVE_OK) {
                throw std::runtime_error(archive_error_string(ext));
            } else if (archive_entry_size(entry) > 0) {
                copyData(a, ext);
            }
            r = archive_write_finish_entry(ext);
            if (r < ARCHIVE_OK) {
                throw std::runtime_error(archive_error_string(ext));
            }
        }
    }

private:
    std::string filename;
    struct archive* a;
    struct archive* ext;

    void setExtractionOptions() {
        int flags = ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM | 
                    ARCHIVE_EXTRACT_ACL | ARCHIVE_EXTRACT_FFLAGS;
        archive_write_disk_set_options(ext, flags);
        archive_write_disk_set_standard_lookup(ext);
    }

    void copyData(struct archive* ar, struct archive* aw) {
        const void* buff;
        size_t size;
        la_int64_t offset;

        while (true) {
            int r = archive_read_data_block(ar, &buff, &size, &offset);
            if (r == ARCHIVE_EOF) return;
            if (r < ARCHIVE_OK) {
                throw std::runtime_error(archive_error_string(ar));
            }
            r = archive_write_data_block(aw, buff, size, offset);
            if (r < ARCHIVE_OK) {
                throw std::runtime_error(archive_error_string(aw));
            }
        }
    }

    std::string getOutputPath(struct archive_entry* entry) {
        std::string entryPath = archive_entry_pathname(entry);
        std::filesystem::path archivePath(filename);
        std::filesystem::path outputDir = archivePath.parent_path();
        return (outputDir / entryPath).string();
    }
};
