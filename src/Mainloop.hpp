#pragma once

class Engine;

class Mainloop {
    Engine& engine;
public:
    Mainloop(Engine& engine);

    void run();
};
