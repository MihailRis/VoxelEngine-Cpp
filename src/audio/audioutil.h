#ifndef SRC_AUDIO_AUDIOUTIL_H_
#define SRC_AUDIO_AUDIOUTIL_H_

#include <string>
#include <type_traits>
#include <cstdint>

#ifdef __APPLE__
#include <OpenAL/al.h>
#else
#include <AL/al.h>
#endif

#define alCheckErrorsMacro() check_al_errors(__FILE__, __LINE__)

bool check_al_errors(const std::string& filename, const std::uint_fast32_t line);

bool load_wav_file_header(std::ifstream& file,
                          std::uint8_t& channels,
                          std::int32_t& sampleRate,
                          std::uint8_t& bitsPerSample,
                          ALsizei& size);

char* load_wav(const std::string& filename,
               std::uint8_t& channels,
               std::int32_t& sampleRate,
               std::uint8_t& bitsPerSample,
               ALsizei& size);

static inline ALenum to_al_format(short channels, short samples){
	bool stereo = (channels > 1);

	switch (samples) {
	case 16:
		if (stereo)
			return AL_FORMAT_STEREO16;
		else
			return AL_FORMAT_MONO16;
	case 8:
		if (stereo)
			return AL_FORMAT_STEREO8;
		else
			return AL_FORMAT_MONO8;
	default:
		return -1;
	}
}

#endif /* SRC_AUDIO_AUDIOUTIL_H_ */
