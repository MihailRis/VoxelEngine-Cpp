#include "audio.h"

#include <iostream>
#include <stdexcept>

#include "NoAudio.h"
#include "AL/ALAudio.h"

#include "../coders/wav.h"
#include "../coders/ogg.h"

namespace audio {
    static speakerid_t nextId = 1;
    static Backend* backend;
    static std::unordered_map<speakerid_t, std::unique_ptr<Speaker>> speakers;
    static std::unordered_map<speakerid_t, std::shared_ptr<Stream>> streams;
}

using namespace audio;

size_t PCMStream::readFully(char* buffer, size_t bufferSize, bool loop) {
    if (!isOpen()) {
        return 0;
    }
    long bytes = 0;
    size_t size = 0;
    do {
        do {
            bytes = read(buffer, bufferSize);
            if (bytes < 0) {
                return size;
            }
            size += bytes;
            bufferSize -= bytes;
            buffer += bytes;
        } while (bytes > 0 && bufferSize > 0);

        if (bufferSize == 0) {
            break;
        }

        if (loop) {
            seek(0);
        }
        if (bufferSize == 0) {
            return size;
        }
    } while (loop);
    return size;
}

/// @brief pcm source that does not initialize buffer
class PCMVoidSource : public PCMStream {
    size_t totalSamples;
    size_t remain;
    uint sampleRate;
    bool seekable;
    bool closed = false;
public:
    PCMVoidSource(size_t totalSamples, uint sampleRate, bool seekable) 
    : totalSamples(totalSamples), 
      remain(totalSamples), 
      sampleRate(sampleRate),
      seekable(seekable) 
    {}

    size_t read(char* buffer, size_t bufferSize) override {
        if (closed) {
            return 0;
        }
        if (!seekable) {
            return bufferSize;
        }
        size_t n = std::min(bufferSize, totalSamples);
        remain -= n;
        return n;
    }

    void close() override {
        closed = true;
    }

    bool isOpen() const override {
        return !closed;
    }

    size_t getTotalSamples() const override {
        return totalSamples;
    }

    duration_t getTotalDuration() const override {
        return static_cast<duration_t>(totalSamples) / 
               static_cast<duration_t>(sampleRate);
    }

    uint getChannels() const override {
        return 1;
    }

    uint getSampleRate() const override {
        return sampleRate;
    }

    uint getBitsPerSample() const override {
        return 8;
    }

    bool isSeekable() const override {
        return seekable;
    }

    void seek(size_t position) override {
        if (closed || !seekable) {
            return;
        }
        position %= totalSamples;
        remain = totalSamples - position;
    }
};

void audio::initialize(bool enabled) {
    if (enabled) {
        backend = ALAudio::create();
    }
    if (backend == nullptr) {
        std::cerr << "could not to initialize audio" << std::endl;
        backend = NoAudio::create();
    }
}

PCM* audio::loadPCM(const fs::path& file, bool headerOnly) {
    if (!fs::exists(file)) {
        throw std::runtime_error("file not found '"+file.u8string()+"'");
    }
    std::string ext = file.extension().u8string();
    if (ext == ".wav" || ext == ".WAV") {
        return wav::load_pcm(file, headerOnly);
    } else if (ext == ".ogg" || ext == ".OGG") {
        return ogg::load_pcm(file, headerOnly);
    }
    throw std::runtime_error("unsupported audio format");
}

Sound* audio::loadSound(const fs::path& file, bool keepPCM) {
    std::shared_ptr<PCM> pcm(loadPCM(file, !keepPCM && backend->isDummy()));
    return createSound(pcm, keepPCM);
}

Sound* audio::createSound(std::shared_ptr<PCM> pcm, bool keepPCM) {
    return backend->createSound(pcm, keepPCM);
}

PCMStream* audio::openPCMStream(const fs::path& file) {
    std::string ext = file.extension().u8string();
    if (ext == ".wav" || ext == ".WAV") {
        return wav::create_stream(file);
    } else if (ext == ".ogg" || ext == ".OGG") {
        return ogg::create_stream(file);
    }
    throw std::runtime_error("unsupported audio stream format");
}

Stream* audio::openStream(const fs::path& file, bool keepSource) {
    if (!keepSource && backend->isDummy()) {
        auto header = loadPCM(file, true);
        // using void source sized as audio instead of actual audio file
        return openStream(
            std::make_shared<PCMVoidSource>(header->totalSamples, header->sampleRate, header->seekable), 
            keepSource
        );
    }
    return openStream(
        std::shared_ptr<PCMStream>(openPCMStream(file)),
        keepSource
    );
}

Stream* audio::openStream(std::shared_ptr<PCMStream> stream, bool keepSource) {
    return backend->openStream(stream, keepSource);
}


void audio::setListener(
    glm::vec3 position, 
    glm::vec3 velocity, 
    glm::vec3 lookAt, 
    glm::vec3 up
) {
    backend->setListener(position, velocity, lookAt, up);
}

void remove_lower_priority_speaker(int priority) {
    for (auto it = speakers.begin(); it != speakers.end();) {
        if (it->second->getPriority() < priority && it->second->isPaused()) {
            it->second->stop();
            it = speakers.erase(it);
            return;
        }
        it++;
    }
    for (auto it = speakers.begin(); it != speakers.end();) {
        if (it->second->getPriority() < priority) {
            it->second->stop();
            it = speakers.erase(it);
            return;
        }
        it++;
    }
}

speakerid_t audio::play(
    Sound* sound,
    glm::vec3 position,
    bool relative,
    float volume,
    float pitch,
    bool loop,
    int priority
) {
    Speaker* speaker = sound->newInstance(priority);
    if (speaker == nullptr) {
        remove_lower_priority_speaker(priority);
        speaker = sound->newInstance(priority);
    }
    if (speaker == nullptr) {
        return 0;
    }
    speakerid_t id = nextId++;
    speakers.emplace(id, speaker);
    speaker->setPosition(position);
    speaker->setVolume(volume);
    speaker->setPitch(pitch);
    speaker->setLoop(loop);
    speaker->setRelative(relative);
    speaker->play();
    return id;
}

speakerid_t audio::play(
    std::shared_ptr<Stream> stream,
    glm::vec3 position,
    bool relative,
    float volume,
    float pitch,
    bool loop
) {
    Speaker* speaker = stream->createSpeaker(loop);
    if (speaker == nullptr) {
        remove_lower_priority_speaker(PRIORITY_HIGH);
        speaker = stream->createSpeaker(loop);
    }
    if (speaker == nullptr) {
        return 0;
    }
    speakerid_t id = nextId++;
    streams.emplace(id, stream);
    speakers.emplace(id, speaker);
    stream->bindSpeaker(id);

    speaker->setPosition(position);
    speaker->setVolume(volume);
    speaker->setPitch(pitch);
    speaker->setLoop(false);
    speaker->setRelative(relative);
    speaker->play();
    return id;
}

speakerid_t audio::playStream(
    const fs::path& file,
    glm::vec3 position,
    bool relative,
    float volume,
    float pitch,
    bool loop
) {
    std::shared_ptr<Stream> stream (openStream(file, false));
    return play(stream, position, relative, volume, pitch, loop);
}

Speaker* audio::get(speakerid_t id) {
    auto found = speakers.find(id);
    if (found == speakers.end()) {
        return nullptr;
    }
    return found->second.get();
}

void audio::update(double delta) {
    backend->update(delta);

    for (auto& entry : streams) {
        entry.second->update(delta);
    }

    for (auto it = speakers.begin(); it != speakers.end();) {
        if (it->second->isStoppedManually()) {
            streams.erase(it->first);
            it = speakers.erase(it);
        } else {
            it++;
        }
    }
}

void audio::close() {
    speakers.clear();
    delete backend;
    backend = nullptr;
}
