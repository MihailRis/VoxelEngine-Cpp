#include "Device.hpp"

#include "../io.hpp"

using namespace io;

SubDevice::SubDevice(
    std::shared_ptr<Device> parent,
    const std::string& path,
    bool createDirectory
)
    : parent(std::move(parent)), root(path) {
    if (createDirectory && !this->parent->exists(path)) {
        this->parent->mkdirs(path);
    }
}
