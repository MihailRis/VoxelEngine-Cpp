#ifndef OBJECT_H
#define OBJECT_H

#include "stdlib.h"
#include <iostream>

class Level;

class Object {
private:
    Level* level;

public:
    int64_t objectUID;    
    bool shouldUpdate = true;

public:
    virtual ~Object() { destroyed(); }

public:
    virtual void spawned() { }
    virtual void update(float delta) { }
    virtual void destroyed() { }

public:
    Level* getLevel() { return level; }
    void setLevel(Level* lvl) { level = lvl; }
};

#endif /* OBJECT_H */