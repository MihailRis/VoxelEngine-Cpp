#pragma once

#include <filesystem>

namespace audio {
    struct PCM;
    class PCMStream;
}

namespace wav {
    std::unique_ptr<audio::PCM> load_pcm(
        const std::filesystem::path& file, bool headerOnly
    );
    std::unique_ptr<audio::PCMStream> create_stream(
        const std::filesystem::path& file
    );
}
