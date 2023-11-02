#ifndef WINDOW_WINDOW_H_
#define WINDOW_WINDOW_H_

#include "../typedefs.h"

class GLFWwindow;

class Window {
	static GLFWwindow* window;
public:
	static uint width;
	static uint height;
	static int initialize(uint width, uint height, const char* title, int samples);
	static void terminate();

	static void viewport(int x, int y, int width, int height);
	static void setCursorMode(int mode);
	static bool isShouldClose();
	static void setShouldClose(bool flag);
	static void swapBuffers();
	static void swapInterval(int interval);
};

#endif /* WINDOW_WINDOW_H_ */
