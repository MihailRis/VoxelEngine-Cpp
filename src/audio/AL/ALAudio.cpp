#include "ALAudio.h"
#include "alutil.h"
#include <string>
#include <iostream>

using namespace audio;

ALSound::ALSound(ALAudio* al, uint buffer, std::shared_ptr<PCM> pcm, bool keepPCM) 
: al(al), buffer(buffer) 
{
    duration = pcm->getDuration();
    if (keepPCM) {
        this->pcm = pcm;
    }
}

ALSound::~ALSound() {
    al->freeBuffer(buffer);
    buffer = 0;
}

Speaker* ALSound::newInstance(int priority, int channel) const {
    uint source = al->getFreeSource();
    if (source == 0) {
        return nullptr;
    }
    AL_CHECK(alSourcei(source, AL_BUFFER, buffer));
    return new ALSpeaker(al, source, priority, channel);
}

ALStream::ALStream(ALAudio* al, std::shared_ptr<PCMStream> source, bool keepSource)
: al(al), source(source), keepSource(keepSource) {
}

ALStream::~ALStream() {
    bindSpeaker(0);
    source = nullptr;
}

std::shared_ptr<PCMStream> ALStream::getSource() const {
    if (keepSource) {
        return source;
    } else {
        return nullptr;
    }
}

bool ALStream::preloadBuffer(uint buffer, bool loop) {
    size_t read = source->readFully(this->buffer, BUFFER_SIZE, loop);
    if (!read)
        return false;
    ALenum format = AL::to_al_format(source->getChannels(), source->getBitsPerSample());
    AL_CHECK(alBufferData(buffer, format, this->buffer, read, source->getSampleRate()));
    return true;
}

Speaker* ALStream::createSpeaker(bool loop, int channel) {
    this->loop = loop;
    uint source = al->getFreeSource();
    if (source == 0) {
        return nullptr;
    }
    for (uint i = 0; i < ALStream::STREAM_BUFFERS; i++) {
        uint buffer = al->getFreeBuffer();
        if (!preloadBuffer(buffer, loop)) {
            break;
        }
        AL_CHECK(alSourceQueueBuffers(source, 1, &buffer));
    }
    return new ALSpeaker(al, source, PRIORITY_HIGH, channel);
}


void ALStream::bindSpeaker(speakerid_t speaker) {
    auto sp = audio::get(this->speaker);
    if (sp) {
        sp->stop();
    }
    this->speaker = speaker;
}

speakerid_t ALStream::getSpeaker() const {
    return speaker;
}

void ALStream::update(double delta) {
    if (this->speaker == 0) {
        return;
    }
    Speaker* speaker = audio::get(this->speaker);
    if (speaker == nullptr) {
        speaker = 0;
        return;
    }
    ALSpeaker* alspeaker = dynamic_cast<ALSpeaker*>(speaker);
    uint source = alspeaker->source;
    uint processed = AL::getSourcei(source, AL_BUFFERS_PROCESSED);

    while (processed--) {
        uint buffer;
        AL_CHECK(alSourceUnqueueBuffers(source, 1, &buffer));
        unusedBuffers.push(buffer);
    }

    uint preloaded = 0;
    if (!unusedBuffers.empty()) {
        uint buffer = unusedBuffers.front();
        if (preloadBuffer(buffer, loop)) {
            preloaded++;
            unusedBuffers.pop();
            AL_CHECK(alSourceQueueBuffers(source, 1, &buffer));
        }
    }
    if (speaker->isStopped() && !alspeaker->stopped) {
        if (preloaded) {
            speaker->play();
        } else {
            speaker->stop();
        }
    }
}

void ALStream::setTime(duration_t time) {
    // TODO: implement
}

ALSpeaker::ALSpeaker(ALAudio* al, uint source, int priority, int channel) 
: al(al), priority(priority), channel(channel), source(source) {
}

ALSpeaker::~ALSpeaker() {
    if (source) {
        stop();
    }
}

void ALSpeaker::update(const Channel* channel, float masterVolume) {
    float gain = this->volume * channel->getVolume()*masterVolume;
    AL_CHECK(alSourcef(source, AL_GAIN, gain));
    
    if (!paused) {
        if (isPaused() && !channel->isPaused()) {
            play();
        } else if (isPlaying() && channel->isPaused()) {
            AL_CHECK(alSourcePause(source));
        }
    }
}

int ALSpeaker::getChannel() const {
    return channel;
}

State ALSpeaker::getState() const {
    int state = AL::getSourcei(source, AL_SOURCE_STATE, AL_STOPPED);
    switch (state) {
        case AL_PLAYING: return State::playing;
        case AL_PAUSED: return State::paused;
        default: return State::stopped;
    }
}

float ALSpeaker::getVolume() const {
    return AL::getSourcef(source, AL_GAIN);
}

void ALSpeaker::setVolume(float volume) {
    this->volume = volume;
}

float ALSpeaker::getPitch() const {
    return AL::getSourcef(source, AL_PITCH);
}

void ALSpeaker::setPitch(float pitch) {
    AL_CHECK(alSourcef(source, AL_PITCH, pitch));
}

bool ALSpeaker::isLoop() const {
    return AL::getSourcei(source, AL_LOOPING) == AL_TRUE;
}

void ALSpeaker::setLoop(bool loop) {
    AL_CHECK(alSourcei(source, AL_LOOPING, loop ? AL_TRUE : AL_FALSE));
}

void ALSpeaker::play() {
    paused = false;
    stopped = false;
    AL_CHECK(alSourcePlay(source));
}

void ALSpeaker::pause() {
    paused = true;
    AL_CHECK(alSourcePause(source));
}

void ALSpeaker::stop() {
    stopped = true;
    if (source) {
        AL_CHECK(alSourceStop(source));
        al->freeSource(source);
    }
}

duration_t ALSpeaker::getTime() const {
    return static_cast<duration_t>(AL::getSourcef(source, AL_SEC_OFFSET));
}

void ALSpeaker::setTime(duration_t time) {
    AL_CHECK(alSourcef(source, AL_SEC_OFFSET, static_cast<float>(time)));
}

void ALSpeaker::setPosition(glm::vec3 pos) {
    AL_CHECK(alSource3f(source, AL_POSITION, pos.x, pos.y, pos.z));
}

glm::vec3 ALSpeaker::getPosition() const {
    return AL::getSource3f(source, AL_POSITION);
}

void ALSpeaker::setVelocity(glm::vec3 vel) {
    AL_CHECK(alSource3f(source, AL_VELOCITY, vel.x, vel.y, vel.z));
}

glm::vec3 ALSpeaker::getVelocity() const {
    return AL::getSource3f(source, AL_VELOCITY);
}

void ALSpeaker::setRelative(bool relative) {
    AL_CHECK(alSourcei(source, AL_SOURCE_RELATIVE, relative ? AL_TRUE : AL_FALSE));    
}

bool ALSpeaker::isRelative() const {
    return AL::getSourcei(source, AL_SOURCE_RELATIVE) == AL_TRUE;
}

int ALSpeaker::getPriority() const {
    return priority;
}


ALAudio::ALAudio(ALCdevice* device, ALCcontext* context)
: device(device), context(context)
{
    ALCint size;
    alcGetIntegerv(device, ALC_ATTRIBUTES_SIZE, 1, &size);
    std::vector<ALCint> attrs(size);
    alcGetIntegerv(device, ALC_ALL_ATTRIBUTES, size, &attrs[0]);
    for (size_t i = 0; i < attrs.size(); ++i){
       if (attrs[i] == ALC_MONO_SOURCES) {
          std::cout << "AL: max mono sources: " << attrs[i+1] << std::endl;
          maxSources = attrs[i+1];
       }
    }
    auto devices = getAvailableDevices();
    std::cout << "AL devices:" << std::endl;
    for (auto& name : devices) {
        std::cout << "  " << name << std::endl;
    }
}

ALAudio::~ALAudio() {
    for (uint source : allsources) {
        int state = AL::getSourcei(source, AL_SOURCE_STATE);
        if (state == AL_PLAYING || state == AL_PAUSED) {
            AL_CHECK(alSourceStop(source));
        }
        AL_CHECK(alDeleteSources(1, &source));
    }

    for (uint buffer : allbuffers){
        AL_CHECK(alDeleteBuffers(1, &buffer));
    }

    AL_CHECK(alcMakeContextCurrent(context));
    alcDestroyContext(context);
    if (!alcCloseDevice(device)) {
        std::cerr << "AL: device not closed!" << std::endl;
    }
    device = nullptr;
    context = nullptr;
}

Sound* ALAudio::createSound(std::shared_ptr<PCM> pcm, bool keepPCM) {
    auto format = AL::to_al_format(pcm->channels, pcm->bitsPerSample);
    uint buffer = getFreeBuffer();
    AL_CHECK(alBufferData(buffer, format, pcm->data.data(), pcm->data.size(), pcm->sampleRate));
    return new ALSound(this, buffer, pcm, keepPCM);
}

Stream* ALAudio::openStream(std::shared_ptr<PCMStream> stream, bool keepSource) {
    return new ALStream(this, stream, keepSource);
}

ALAudio* ALAudio::create() {
    ALCdevice* device = alcOpenDevice(nullptr);
    if (device == nullptr)
        return nullptr;
    ALCcontext* context = alcCreateContext(device, nullptr);
    if (!alcMakeContextCurrent(context)){
        alcCloseDevice(device);
        return nullptr;
    }
    AL_CHECK();
    std::cout << "AL: initialized" << std::endl;
    return new ALAudio(device, context);
}

uint ALAudio::getFreeSource(){
    if (!freesources.empty()){
        uint source = freesources.back();
        freesources.pop_back();
        return source;
    }
    if (allsources.size() == maxSources){
        std::cerr << "attempted to create new source, but limit is " << maxSources << std::endl;
        return 0;
    }
    ALuint id;
    alGenSources(1, &id);
    if (!AL_GET_ERROR())
        return 0;
    allsources.push_back(id);
    return id;
}

uint ALAudio::getFreeBuffer(){
    if (!freebuffers.empty()){
        uint buffer = freebuffers.back();
        freebuffers.pop_back();
        return buffer;
    }
    ALuint id;
    alGenBuffers(1, &id);
    if (!AL_GET_ERROR()) {
        return 0;
    }

    allbuffers.push_back(id);
    return id;
}

void ALAudio::freeSource(uint source){
    freesources.push_back(source);
}

void ALAudio::freeBuffer(uint buffer){
    freebuffers.push_back(buffer);
}

std::vector<std::string> ALAudio::getAvailableDevices() const {
    std::vector<std::string> devicesVec;

    const ALCchar* devices;
    devices = alcGetString(device, ALC_DEVICE_SPECIFIER);
    if (!AL_GET_ERROR()) {
        return devicesVec;
    }

    const char* ptr = devices;
    do {
        devicesVec.push_back(std::string(ptr));
        ptr += devicesVec.back().size() + 1;
    }
    while (ptr[0]);

    return devicesVec;
}

void ALAudio::setListener(glm::vec3 position, glm::vec3 velocity, glm::vec3 at, glm::vec3 up){
    ALfloat listenerOri[] = { at.x, at.y, at.z, up.x, up.y, up.z };

    AL_CHECK(alListener3f(AL_POSITION, position.x, position.y, position.z));
    AL_CHECK(alListener3f(AL_VELOCITY, velocity.x, velocity.y, velocity.z));
    AL_CHECK(alListenerfv(AL_ORIENTATION, listenerOri));
}

void ALAudio::update(double delta) {
}
