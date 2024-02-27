#include "audio.h"

#include <iostream>

#include "ALAudio.h"
#include "NoAudio.h"

audio::Backend* audio::backend = nullptr;

void audio::initialize(bool enabled) {
    if (enabled) {
        audio::backend = ALAudio::create();
    }
    if (audio::backend == nullptr) {
        std::cerr << "could not to initialize audio" << std::endl;
        audio::backend = NoAudio::create();
    }
}

void audio::setListener(
    glm::vec3 position, 
    glm::vec3 velocity, 
    glm::vec3 lookAt, 
    glm::vec3 up
) {
    audio::backend->setListener(position, velocity, lookAt, up);
}

void audio::close() {
    delete audio::backend;
    audio::backend = nullptr;
}
