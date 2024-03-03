#ifndef CODERS_WAV_H_
#define CODERS_WAV_H_

#include <filesystem>

namespace audio {
    struct PCM;
    class PCMStream;
}

namespace wav {
    extern audio::PCM* load_pcm(const std::filesystem::path& file, bool headerOnly);
    extern audio::PCMStream* create_stream(const std::filesystem::path& file);
}

#endif // CODERS_WAV_H_
