#pragma once

class Engine;

class TestMainloop {
    Engine& engine;
public:
    TestMainloop(Engine& engine);

    void run();
};
