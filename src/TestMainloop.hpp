#pragma once

#include <memory>

class Level;
class LevelController;
class Engine;

class TestMainloop {
    Engine& engine;
    std::unique_ptr<LevelController> controller;
public:
    TestMainloop(Engine& engine);
    ~TestMainloop();

    void run();

    void setLevel(std::unique_ptr<Level> level);
};
