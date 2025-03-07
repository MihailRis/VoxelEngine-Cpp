#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <stack>
#include <vector>

#include "graphics/core/commons.hpp"
#include "typedefs.hpp"

class ImageData;
class Input;
struct DisplaySettings;

namespace Window {
    extern uint width;
    extern uint height;

    std::unique_ptr<Input> initialize(DisplaySettings* settings);
    void terminate();

    void viewport(int x, int y, int width, int height);
    void setCursorMode(int mode);
    bool isShouldClose();
    void setShouldClose(bool flag);
    void swapBuffers();
    void setFramerate(int interval);
    void toggleFullscreen();
    bool isFullscreen();
    bool isMaximized();
    bool isFocused();
    bool isIconified();

    void pushScissor(glm::vec4 area);
    void popScissor();
    void resetScissor();

    void setCursor(CursorShape shape);

    void clear();
    void clearDepth();
    void setBgColor(glm::vec3 color);
    void setBgColor(glm::vec4 color);
    double time();
    void setClipboardText(const char* text);
    DisplaySettings* getSettings();
    void setIcon(const ImageData* image);

    inline glm::vec2 size() {
        return glm::vec2(width, height);
    }

    std::unique_ptr<ImageData> takeScreenshot();
};
