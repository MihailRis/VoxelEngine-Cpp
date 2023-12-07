#ifndef WINDOW_WINDOW_H_
#define WINDOW_WINDOW_H_

#include "../typedefs.h"
#include "../settings.h"

#include <stack>
#include <vector>

#include <glm/glm.hpp>

class GLFWwindow;
class ImageData;
struct DisplaySettings;
struct GLFWmonitor;

class Window {
	static GLFWwindow* window;
	static DisplaySettings* settings;
	static std::stack<glm::vec4> scissorStack;
	static glm::vec4 scissorArea;

	static bool tryToMaximize(GLFWwindow* window, GLFWmonitor* monitor);
	static int posX_;
	static int posY_;
	static uint width_;
	static uint height_;
public:
	static int initialize(DisplaySettings& settings);
	static void terminate();

	static void viewport(int x, int y, int width, int height);
	static void setCursorMode(int mode);
	static bool isShouldClose();
	static void setShouldClose(bool flag);
	static void swapBuffers();
	static void swapInterval(int interval);
	static void toggleFullscreen();
	static bool isFullscreen();
	static bool isMaximized();

	static void pushScissor(glm::vec4 area);
	static void popScissor();
	static void resetScissor();

	static void clear();
	static void clearDepth();
	static void setBgColor(glm::vec3 color);
	static double time();
	static DisplaySettings* getSettings();

	static glm::vec2 size() {
		return glm::vec2(width_, height_);
	}
	static uint width(){
		return width_;
	}
	static uint height(){
		return height_;
	}
	static void setWidth(uint w);
	static void setHeight(uint h);

	static ImageData* takeScreenshot();
};

#endif /* WINDOW_WINDOW_H_ */
