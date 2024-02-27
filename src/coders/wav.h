#ifndef CODERS_WAV_H_
#define CODERS_WAV_H_

#include <filesystem>

namespace audio {
    struct PCM;
}

namespace wav {
    extern audio::PCM* load_pcm(const std::filesystem::path& file, bool headerOnly);
}

#endif // CODERS_WAV_H_
