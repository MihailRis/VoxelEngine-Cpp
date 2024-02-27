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

    class Speaker;

    enum class State {
        playing,
        paused,
        stopped
    };

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
        /// @return new speaker with sound bound or nullptr 
        /// if all speakers are in use
        virtual Speaker* newInstance(int priority) const = 0;
    };

    /// @brief Audio source controller interface
    class Speaker {
    public:
        virtual ~Speaker() {}

        /// @brief Get current speaker state
        /// @return speaker state
        virtual State getState() const = 0;

        /// @brief Get speaker audio gain
        /// @return speaker audio gain value
        virtual float getVolume() const = 0;

        /// @brief Set speaker audio gain (must be positive)
        /// @param volume new gain value
        virtual void setVolume(float volume) = 0;

        /// @brief Get speaker pitch multiplier
        /// @return pitch multiplier
        virtual float getPitch() const = 0;

        /// @brief Set speaker pitch multiplier
        /// @param pitch new pitch multiplier (must be positive)
        virtual void setPitch(float pitch) = 0;

        /// @brief Play, replay or resume audio
        virtual void play() = 0;

        /// @brief Pause playing audio and keep speaker alive
        virtual void pause() = 0;

        /// @brief Stop and destroy speaker
        virtual void stop() = 0;
        
        /// @brief Get current time position of playing audio
        /// @return time position in seconds
        virtual duration_t getTime() const = 0;

        /// @brief Set playing audio time position
        /// @param time time position in seconds
        virtual void setTime(duration_t time) = 0;

        /// @brief Set speaker 3D position in the world
        /// @param pos new position
        virtual void setPosition(glm::vec3 pos) = 0;

        /// @brief Get speaker 3D position in the world
        /// @return position
        virtual glm::vec3 getPosition() const = 0;

        /// @brief Set speaker movement velocity used for Doppler effect
        /// @param vel velocity vector
        virtual void setVelocity(glm::vec3 vel) = 0;

        /// @brief Get speaker movement velocity used for Doppler effect
        /// @return velocity vector
        virtual glm::vec3 getVelocity() const = 0;

        /// @brief Get speaker priority
        /// @return speaker priority value
        virtual int getPriority() const = 0;
    };

    class Backend {
    public:
        virtual ~Backend() {};

        virtual Sound* createSound(std::shared_ptr<PCM> pcm, bool keepPCM) = 0;

        virtual void setListener(
            glm::vec3 position, 
            glm::vec3 velocity, 
            glm::vec3 lookAt, 
            glm::vec3 up
        ) = 0;

        virtual void update(double delta) = 0;
    };

    /// @brief Initialize audio system or use no audio mode
    /// @param enabled try to initialize actual audio
    extern void initialize(bool enabled);

    /// @brief Create new sound from PCM data
    /// @param pcm PCM data
    /// @param keepPCM store PCM data in sound to make it accessible with 
    /// Sound::getPCM
    /// @return new Sound instance 
    extern Sound* createSound(std::shared_ptr<PCM> pcm, bool keepPCM);

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
    
    /// @brief Update audio streams and sound instanced
    /// @param delta time since the last update (seconds)
    extern void update(double delta);
    
    /// @brief Finalize audio system
    extern void close();
};

#endif // AUDIO_AUDIO_H_
