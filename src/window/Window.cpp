#include <iostream>
#include "Window.h"
#include "Events.h"
#include "../graphics/ImageData.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

using glm::vec4;

GLFWwindow* Window::window = nullptr;
DisplaySettings* Window::settings = nullptr;
std::stack<vec4> Window::scissorStack;
vec4 Window::scissorArea;
uint Window::width = 0;
uint Window::height = 0;
int Window::posX = 0;
int Window::posY = 0;

void cursor_position_callback(GLFWwindow*, double xpos, double ypos) {
	if (Events::_cursor_started) {
		Events::deltaX += xpos - Events::x;
		Events::deltaY += ypos - Events::y;
	}
	else {
		Events::_cursor_started = true;
	}
	Events::x = xpos;
	Events::y = ypos;
}

void mouse_button_callback(GLFWwindow*, int button, int action, int) {
	if (action == GLFW_PRESS) {
		Events::_keys[_MOUSE_BUTTONS + button] = true;
		Events::_frames[_MOUSE_BUTTONS + button] = Events::_current;
	}
	else if (action == GLFW_RELEASE) {
		Events::_keys[_MOUSE_BUTTONS + button] = false;
		Events::_frames[_MOUSE_BUTTONS + button] = Events::_current;
	}
}

void key_callback(GLFWwindow*, int key, int scancode, int action, int /*mode*/) {
	if (key == GLFW_KEY_UNKNOWN) return;
	if (action == GLFW_PRESS) {
		Events::_keys[key] = true;
		Events::_frames[key] = Events::_current;
		Events::pressedKeys.push_back(key);
	}
	else if (action == GLFW_RELEASE) {
		Events::_keys[key] = false;
		Events::_frames[key] = Events::_current;
	}
	else if (action == GLFW_REPEAT) {
		Events::pressedKeys.push_back(key);
	}
}

void scroll_callback(GLFWwindow*, double xoffset, double yoffset) {
	Events::scroll += yoffset;
}

bool Window::isMaximized() {
	return glfwGetWindowAttrib(window, GLFW_MAXIMIZED);
}

void window_size_callback(GLFWwindow*, int width, int height) {
	glViewport(0, 0, width, height);
	Window::width = width;
	Window::height = height;
	if (!Window::isFullscreen() && !Window::isMaximized()) {
		Window::getSettings()->width = width;
		Window::getSettings()->height = height;
	}
	Window::resetScissor();
}

void character_callback(GLFWwindow* window, unsigned int codepoint){
	Events::codepoints.push_back(codepoint);
}

int Window::initialize(DisplaySettings& settings){
	Window::settings = &settings;
	Window::width = settings.width;
	Window::height = settings.height;

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
	glfwWindowHint(GLFW_SAMPLES, settings.samples);

	window = glfwCreateWindow(width, height, settings.title, nullptr, nullptr);
	if (window == nullptr){
		std::cerr << "Failed to create GLFW Window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK){
		std::cerr << "Failed to initialize GLEW" << std::endl;
		return -1;
	}
	glViewport(0,0, width, height);

	glClearColor(0.0f,0.0f,0.0f, 1);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Events::initialize();
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetWindowSizeCallback(window, window_size_callback);
	glfwSetCharCallback(window, character_callback);
	glfwSetScrollCallback(window, scroll_callback);
	if (settings.fullscreen) {
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, GLFW_DONT_CARE);
	}
	glfwSwapInterval(settings.swapInterval);
	return 0;
}

void Window::clear() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Window::setBgColor(glm::vec3 color) {
	glClearColor(color.r, color.g, color.b, 1.0f);
}

void Window::viewport(int x, int y, int width, int height){
	glViewport(x, y, width, height);
}

void Window::setCursorMode(int mode){
	glfwSetInputMode(window, GLFW_CURSOR, mode);
}

void Window::resetScissor() {
	scissorArea = vec4(0.0f, 0.0f, width, height);
	scissorStack = std::stack<vec4>();
	glDisable(GL_SCISSOR_TEST);
}

void Window::pushScissor(vec4 area) {
	if (scissorStack.empty()) {
		glEnable(GL_SCISSOR_TEST);
	}
	scissorStack.push(scissorArea);

	area.z += area.x;
	area.w += area.y;

	area.x = fmax(area.x, scissorArea.x);
	area.y = fmax(area.y, scissorArea.y);

	area.z = fmin(area.z, scissorArea.z);
	area.w = fmin(area.w, scissorArea.w);

	if (area.z < 0.0f || area.w < 0.0f) {
		glScissor(0, 0, 0, 0);
	} else {
		glScissor(area.x, Window::height-area.w, area.z-area.x, area.w-area.y);
	}
	scissorArea = area;
}

void Window::popScissor() {
	if (scissorStack.empty()) {
		std::cerr << "warning: extra Window::popScissor call" << std::endl;
		return;
	}
	vec4 area = scissorStack.top();
	scissorStack.pop();
	if (area.z < 0.0f || area.w < 0.0f) {
		glScissor(0, 0, 0, 0);
	} else {
		glScissor(area.x, Window::height-area.w, area.z-area.x, area.w-area.y);
	}
	if (scissorStack.empty()) {
		glDisable(GL_SCISSOR_TEST);
	}
	scissorArea = area;
}

void Window::terminate(){
	Events::finalize();
	glfwTerminate();
}

bool Window::isShouldClose(){
	return glfwWindowShouldClose(window);
}

void Window::setShouldClose(bool flag){
	glfwSetWindowShouldClose(window, flag);
}

void Window::swapInterval(int interval){
	glfwSwapInterval(interval);
}

void Window::toggleFullscreen(){
	settings->fullscreen = !settings->fullscreen;

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);

	if (Events::_cursor_locked) Events::toggleCursor();

	if (settings->fullscreen) {
		glfwGetWindowPos(window, &posX, &posY);
		glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, GLFW_DONT_CARE);
	}
	else {
		glfwSetWindowMonitor(window, nullptr, posX, posY, settings->width, settings->height, GLFW_DONT_CARE);
		glfwSetWindowAttrib(window, GLFW_MAXIMIZED, GLFW_FALSE);
	}

	double xPos, yPos;
	glfwGetCursorPos(window, &xPos, &yPos);
	Events::x = xPos;
	Events::y = yPos;
}

bool Window::isFullscreen() {
	return settings->fullscreen;
}

void Window::swapBuffers(){
	glfwSwapBuffers(window);
	Window::resetScissor();
}

double Window::time() {
	return glfwGetTime();
}

DisplaySettings* Window::getSettings() {
	return settings;
}

ImageData* Window::takeScreenshot() {
	ubyte* data = new ubyte[width * height * 3];
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
	return new ImageData(ImageFormat::rgb888, width, height, data);
}

bool Window::tryToMaximize(GLFWwindow* window, GLFWmonitor* monitor) {
	glm::ivec4 windowFrame(0);
	glm::ivec4 workArea(0);
	glfwGetWindowFrameSize(window, &windowFrame.x, &windowFrame.y, &windowFrame.z, &windowFrame.w);
	glfwGetMonitorWorkarea(monitor, &workArea.x, &workArea.y, &workArea.z, &workArea.w);
	if (Window::width > (uint)workArea.z) Window::width = (uint)workArea.z;
	if (Window::height > (uint)workArea.w) Window::height = (uint)workArea.w;
	if (Window::width >= (uint)(workArea.z - (windowFrame.x + windowFrame.z)) &&
		Window::height >= (uint)(workArea.w - (windowFrame.y + windowFrame.w))) {
		glfwMaximizeWindow(window);
		return true;
	}
	glfwSetWindowSize(window, Window::width, Window::height);
	glfwSetWindowPos(window, workArea.x + (workArea.z - Window::width) / 2, 
							 workArea.y + (workArea.w - Window::height) / 2 + windowFrame.y / 2);
	return false;
}