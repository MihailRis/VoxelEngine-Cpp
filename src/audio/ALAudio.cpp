#include "ALAudio.h"
#include "alutil.h"
#include <string>
#include <iostream>

using namespace audio;

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
    for (ALSource* source : allsources) {
        if (source->isPlaying()){
            alSourceStop(source->id);
            alCheckErrorsMacro();
        }
        alDeleteSources(1, &source->id); 
        alCheckErrorsMacro();
    }

    for (ALBuffer* buffer : allbuffers){
        alDeleteBuffers(1, &buffer->id); 
        alCheckErrorsMacro();
    }

    alcMakeContextCurrent(context);
    alcDestroyContext(context);
    if (!alcCloseDevice(device)) {
        std::cerr << "AL: device not closed!" << std::endl;
    }
    device = nullptr;
    context = nullptr;
}

bool ALSource::setBuffer(ALBuffer* buffer) {
    alSourcei(id, AL_BUFFER, buffer->id);
    return alCheckErrorsMacro();
}

bool ALSource::play(){
    alSourcePlay(id);
    return alCheckErrorsMacro();
}

bool ALSource::isPlaying() {
    int state;
    alGetSourcei(id, AL_SOURCE_STATE, &state);
    return state == AL_PLAYING;
}

bool ALSource::setPosition(glm::vec3 position) {
    alSource3f(id, AL_POSITION, position.x, position.y, position.z);
    return alCheckErrorsMacro();
}

bool ALSource::setVelocity(glm::vec3 velocity) {
    alSource3f(id, AL_VELOCITY, velocity.x, velocity.y, velocity.z);
    return alCheckErrorsMacro();
}

bool ALSource::setLoop(bool loop) {
    alSourcei(id, AL_LOOPING, AL_TRUE ? loop : AL_FALSE);
    return alCheckErrorsMacro();
}

bool ALSource::setGain(float gain) {
    alSourcef(id, AL_GAIN, gain);
    return alCheckErrorsMacro();
}


bool ALSource::setPitch(float pitch) {
    alSourcef(id, AL_PITCH, pitch);
    return alCheckErrorsMacro();
}

bool ALBuffer::load(int format, const char* data, int size, int freq) {
    alBufferData(id, format, data, size, freq);
    return alCheckErrorsMacro();
}

Sound* ALAudio::createSound(std::shared_ptr<PCM> pcm, bool keepPCM) {
    // TODO: implement
    return nullptr;
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
    if (!alCheckErrorsMacro()) {
        return nullptr;
    }
    std::cout << "AL: initialized" << std::endl;
    return new ALAudio(device, context);
}

ALSource* ALAudio::getFreeSource(){
    if (!freesources.empty()){
        ALSource* source = freesources.back();
        freesources.pop_back();
        return source;
    }
    if (allsources.size() == maxSources){
        std::cerr << "attempted to create new source, but limit is " << maxSources << std::endl;
        return nullptr;
    }
    ALuint id;
    alGenSources(1, &id);
    if (!alCheckErrorsMacro())
        return nullptr;

    ALSource* source = new ALSource(id);
    allsources.push_back(source);
    return source;
}

ALBuffer* ALAudio::getFreeBuffer(){
    if (!freebuffers.empty()){
        ALBuffer* buffer = freebuffers.back();
        freebuffers.pop_back();
        return buffer;
    }
    if (allbuffers.size() == maxBuffers){
        std::cerr << "attempted to create new ALbuffer, but limit is " << maxBuffers << std::endl;
        return nullptr;
    }
    ALuint id;
    alGenBuffers(1, &id);
    if (!alCheckErrorsMacro()) {
        return nullptr;
    }

    ALBuffer* buffer = new ALBuffer(id);
    allbuffers.push_back(buffer);
    return buffer;
}

void ALAudio::freeSource(ALSource* source){
    freesources.push_back(source);
}

void ALAudio::freeBuffer(ALBuffer* buffer){
    freebuffers.push_back(buffer);
}

std::vector<std::string> ALAudio::getAvailableDevices() const {
    std::vector<std::string> devicesVec;

    const ALCchar* devices;
    devices = alcGetString(device, ALC_DEVICE_SPECIFIER);
    if (!alCheckErrorsMacro()) {
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

    alListener3f(AL_POSITION, position.x, position.y, position.z);
    alCheckErrorsMacro();
    alListener3f(AL_VELOCITY, velocity.x, velocity.y, velocity.z);
    alCheckErrorsMacro();
    alListenerfv(AL_ORIENTATION, listenerOri);
    alCheckErrorsMacro();
}
