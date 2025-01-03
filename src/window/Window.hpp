#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <stack>
#include <vector>

#include "graphics/core/commons.hpp"
#include "typedefs.hpp"

class ImageData;
struct DisplaySettings;
struct GLFWwindow;
struct GLFWmonitor;

class Window {
    static GLFWwindow* window;
    static DisplaySettings* settings;
    static std::stack<glm::vec4> scissorStack;
    static glm::vec4 scissorArea;
    static bool fullscreen;
    static int framerate;
    static double prevSwap;
    static CursorShape cursor;

    static bool tryToMaximize(GLFWwindow* window, GLFWmonitor* monitor);
public:
    static int posX;
    static int posY;
    static uint width;
    static uint height;
    static int initialize(DisplaySettings* settings);
    static void terminate();

    static void viewport(int x, int y, int width, int height);
    static void setCursorMode(int mode);
    static bool isShouldClose();
    static void setShouldClose(bool flag);
    static void swapBuffers();
    static void setFramerate(int interval);
    static void toggleFullscreen();
    static bool isFullscreen();
    static bool isMaximized();
    static bool isFocused();
    static bool isIconified();

    static void pushScissor(glm::vec4 area);
    static void popScissor();
    static void resetScissor();

    static void setCursor(CursorShape shape);

    static void clear();
    static void clearDepth();
    static void setBgColor(glm::vec3 color);
    static void setBgColor(glm::vec4 color);
    static double time();
    static const char* getClipboardText();
    static void setClipboardText(const char* text);
    static DisplaySettings* getSettings();
    static void setIcon(const ImageData* image);

    static glm::vec2 size() {
        return glm::vec2(width, height);
    }

    static std::unique_ptr<ImageData> takeScreenshot();
};
