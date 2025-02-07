#pragma once

#include <memory>

#include "io/fwd.hpp"

namespace audio {
    struct PCM;
    class PCMStream;
}

namespace ogg {
    std::unique_ptr<audio::PCM> load_pcm(
        const io::path& file, bool headerOnly
    );
    std::unique_ptr<audio::PCMStream> create_stream(
        const io::path& file
    );
}
