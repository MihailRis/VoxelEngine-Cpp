#ifndef CODERS_OGG_H_
#define CODERS_OGG_H_

#include <filesystem>

namespace audio {
    struct PCM;
}

namespace ogg {
    extern audio::PCM* load_pcm(const std::filesystem::path& file, bool headerOnly);
}

#endif // CODERS_OGG_H_
