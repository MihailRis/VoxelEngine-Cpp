#include "ogg.h"

#include <iostream>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#include "../audio/audio.h"
#include "../typedefs.h"

static inline const char* vorbis_error_message(int code) {
    switch (code) {
        case 0: return "no error";
        case OV_EREAD: return "a read from media returned an error";
        case OV_ENOTVORBIS: return "bitstream does not contain any Vorbis data";
        case OV_EVERSION: return "vorbis version mismatch";
        case OV_EBADHEADER: return "invalid Vorbis bitstream header";
        case OV_EFAULT: return "internal logic fault";
        default:
            return "unknown";
    }
}

audio::PCM* ogg::load_pcm(const std::filesystem::path& file, bool headerOnly) {
    OggVorbis_File vf;
    int code;
    if ((code = ov_fopen(file.u8string().c_str(), &vf))) {
        throw std::runtime_error(vorbis_error_message(code));
    }
    std::vector<char> data;

    vorbis_info* info = ov_info(&vf, -1);
    uint channels = info->channels;
    uint sampleRate = info->rate;

    if (!headerOnly) {
        const int bufferSize = 4096;
        int section = 0;
        char buffer[bufferSize];

        bool eof = false;
        while (!eof) {
            long ret = ov_read(&vf, buffer, bufferSize, 0, 2, true, &section);
            if (ret == 0) {
                eof = true;
            } else if (ret < 0) {
                std::cerr << "ogg::load_pcm: " << vorbis_error_message(ret) << std::endl;
            } else {
                data.insert(data.end(), std::begin(buffer), std::end(buffer));
            }
        }
    }
    
    ov_clear(&vf);
    return new audio::PCM(std::move(data), channels, 16, sampleRate);
}
