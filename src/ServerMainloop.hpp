#pragma once

#include <memory>

class Level;
class LevelController;
class Engine;

class ServerMainloop {
    Engine& engine;
    std::unique_ptr<LevelController> controller;
public:
    ServerMainloop(Engine& engine);
    ~ServerMainloop();

    void run();

    void setLevel(std::unique_ptr<Level> level);
};
