#include <gtest/gtest.h>
#include <filesystem>

#include "files/files.hpp"
#include "files/compatibility.hpp"

TEST(compatibility, convert) {
    auto infile = std::filesystem::u8path(
        "voxels_0_1.bin");
    auto outfile = std::filesystem::u8path(
        "output_0_1.bin");
    auto input = files::read_bytes_buffer(infile);
    auto output = compatibility::convertRegion2to3(input, REGION_LAYER_VOXELS);
    files::write_bytes(outfile, output.data(), output.size());
}
