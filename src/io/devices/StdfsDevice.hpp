#include "Device.hpp"

namespace io {
    class StdfsDevice : public Device {
    public:
        StdfsDevice(std::filesystem::path root) : root(std::move(root)) {}

        std::filesystem::path resolve(std::string_view path) override;
        void write(std::string_view path, const void* data, size_t size) override;
        void read(std::string_view path, void* data, size_t size) override;
        size_t size(std::string_view path) override;
        bool exists(std::string_view path) override;
        bool isdir(std::string_view path) override;
        bool isfile(std::string_view path) override;
        void mkdirs(std::string_view path) override;
        bool remove(std::string_view path) override;
        uint64_t removeAll(std::string_view path) override;
        std::unique_ptr<PathsGenerator> list(std::string_view path) override;
    private:
        std::filesystem::path root;
    };
}
