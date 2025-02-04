#pragma once

#include <string>
#include <filesystem>

#include "../path.hpp"

namespace io {
    class Device {
    public:
        virtual ~Device() = default;

        virtual std::filesystem::path resolve(std::string_view path) = 0;

        virtual void write(std::string_view path, const void* data, size_t size) = 0;
        virtual void read(std::string_view path, void* data, size_t size) = 0;

        virtual size_t size(std::string_view path) = 0;

        virtual bool exists(std::string_view path) = 0;
        virtual bool isdir(std::string_view path) = 0;
        virtual bool isfile(std::string_view path) = 0;
        virtual void mkdirs(std::string_view path) = 0;
        virtual bool remove(std::string_view path) = 0;
        virtual uint64_t removeAll(std::string_view path) = 0;
        virtual std::unique_ptr<PathsGenerator> list(std::string_view path) = 0;
    };

    class SubDevice : public Device {
    public:
        SubDevice(
            std::shared_ptr<Device> parent,
            const std::string& path,
            bool createDirectory = true
        );

        std::filesystem::path resolve(std::string_view path) override {
            return parent->resolve((root / path).pathPart());
        }

        void write(std::string_view path, const void* data, size_t size) override {
            parent->write((root / path).pathPart(), data, size);
        }

        void read(std::string_view path, void* data, size_t size) override {
            parent->read((root / path).pathPart(), data, size);
        }

        size_t size(std::string_view path) override {
            return parent->size((root / path).pathPart());
        }

        bool exists(std::string_view path) override {
            return parent->exists((root / path).pathPart());
        }

        bool isdir(std::string_view path) override {
            return parent->isdir((root / path).pathPart());
        }

        bool isfile(std::string_view path) override {
            return parent->isfile((root / path).pathPart());
        }

        void mkdirs(std::string_view path) override {
            parent->mkdirs((root / path).pathPart());
        }

        bool remove(std::string_view path) override {
            return parent->remove((root / path).pathPart());
        }

        uint64_t removeAll(std::string_view path) override {
            return parent->removeAll((root / path).pathPart());
        }

        std::unique_ptr<PathsGenerator> list(std::string_view path) override {
            return parent->list((root / path).pathPart());
        }
    private:
        std::shared_ptr<Device> parent;
        path root;
    };
}
