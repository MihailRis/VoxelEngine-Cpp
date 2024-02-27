#ifndef AUDIO_AUDIO_H_
#define AUDIO_AUDIO_H_

#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "../typedefs.h"

namespace audio {
    using speakerid_t = int64_t;
    /// @brief duration unit is second
    using duration_t = float;

    /// @brief Pulse-code modulation data
    struct PCM {
        /// @brief May contain 8 bit and 16 bit PCM data
        std::vector<char> data;
        uint8_t channels;
        uint8_t bitsPerSample;
        uint sampleRate;

        constexpr inline size_t countSamples() const {
            return data.size() / channels / (bitsPerSample / 8);
        }

        constexpr inline duration_t getDuration() const {
            return countSamples() / static_cast<duration_t>(sampleRate);
        }
    };

    /// @brief Sound is an audio asset that supposed to support many 
    /// simultaneously playing instances with different sources.
    /// So it's audio data is stored in memory.
    class Sound {
    public:
        virtual ~Sound() {}

        /// @brief Get sound duration
        /// @return duration in seconds (>= 0.0)
        virtual duration_t getDuration() const = 0;

        /// @brief Get sound PCM data
        /// @return PCM data or nullptr
        virtual std::shared_ptr<PCM> getPCM() const = 0;

        /// @brief Create new sound instance
        /// @param priority instance priority. High priority instance can 
        /// take out speaker from low priority instance
        /// @return new speaker id with sound bound or 0 
        /// if all speakers are in use
        virtual speakerid_t newInstance(int priority) const = 0;
    };

    class Backend {
    public:
        virtual ~Backend() {};

        /// @brief Create new sound from PCM data
        /// @param pcm PCM data
        /// @param keepPCM store PCM data in sound to make it accessible with 
        /// Sound::getPCM
        /// @return new Sound instance 
        virtual Sound* createSound(std::shared_ptr<PCM> pcm, bool keepPCM) = 0;

        virtual void setListener(
            glm::vec3 position, 
            glm::vec3 velocity, 
            glm::vec3 lookAt, 
            glm::vec3 up
        ) = 0;
    };

    extern Backend* backend;

    /// @brief Initialize audio system or use no audio mode
    /// @param enabled try to initialize actual audio
    extern void initialize(bool enabled);

    /// @brief Configure 3D listener
    /// @param position listener position
    /// @param velocity listener velocity (used for Doppler effect)
    /// @param lookAt point the listener look at
    /// @param up camera up vector
    extern void setListener(
        glm::vec3 position, 
        glm::vec3 velocity, 
        glm::vec3 lookAt, 
        glm::vec3 up
    );
    /// @brief Finalize audio system
    extern void close();
};

#endif // AUDIO_AUDIO_H_
