#ifndef WINDOW_WINDOW_H_
#define WINDOW_WINDOW_H_

#include "../typedefs.h"
#include "../settings.h"

#include <stack>
#include <vector>

#include <glm/glm.hpp>

using VkInstance = struct VkInstance_T*;
using VkSurfaceKHR = struct VkSurfaceKHR_T*;

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
public:
	static int posX;
	static int posY;
	static uint width;
	static uint height;
	static bool isResized;
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
	static bool isFocused();

	static void pushScissor(glm::vec4 area);
	static void popScissor();
	static void resetScissor();

	static void clear();
	static void clearDepth();
	static void setBgColor(glm::vec3 color);
	static double time();
	static DisplaySettings* getSettings();

	static std::vector<const char*> getRequiredExtensions();
	static VkSurfaceKHR createWindowSurface(VkInstance instance);

	static glm::vec2 size() {
		return glm::vec2(width, height);
	}

	static ImageData* takeScreenshot();
};

#endif /* WINDOW_WINDOW_H_ */
