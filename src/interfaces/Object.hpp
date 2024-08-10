#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <iostream>

class Level;

class Object {
private:

public:
    uint64_t objectUID;    
    bool shouldUpdate = true;

public:
    ~Object() { destroyed(); }

public:
    virtual void spawned() {  }
    virtual void update(float delta) { }
    virtual void destroyed() {  }
};
