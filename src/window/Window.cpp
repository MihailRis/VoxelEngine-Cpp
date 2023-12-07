#include <iostream>
#include "Window.h"
#include "Events.h"
#include "../graphics/ImageData.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

using glm::vec4;
using std::cout;
using std::cerr;
using std::endl;

GLFWwindow* Window::window = nullptr;
DisplaySettings* Window::settings = nullptr;
std::stack<vec4> Window::scissorStack;
vec4 Window::scissorArea;
uint Window::width_ = 0;
uint Window::height_ = 0;
int Window::posX_ = 0;
int Window::posY_ = 0;

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
		// Unsafe assignments! (no checks)
		Events::_keys[_MOUSE_KEYS_OFFSET + button] = true; 
		Events::_frames[_MOUSE_KEYS_OFFSET + button] = Events::_current;
	}
	else if (action == GLFW_RELEASE) {
		// Unsafe assignments! (no checks)
		Events::_keys[_MOUSE_KEYS_OFFSET + button] = false;
		Events::_frames[_MOUSE_KEYS_OFFSET + button] = Events::_current;
	}
}

void key_callback(GLFWwindow*, int key, int scancode, int action, int /*mode*/) {
	if (key == GLFW_KEY_UNKNOWN) return;
	if (action == GLFW_PRESS) {
		// Unsafe assignments! (no checks)
		Events::_keys[key] = true;
		Events::_frames[key] = Events::_current;
		Events::pressedKeys.push_back(key);
	}
	else if (action == GLFW_RELEASE) {
		// Unsafe assignments! (no checks)
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
	Window::setWidth(width);
	Window::setHeight(height);
	if (!Window::isFullscreen() && !Window::isMaximized()) {
		Window::getSettings()->width = width;
		Window::getSettings()->height = height;
	}
	Window::resetScissor();
}

void character_callback(GLFWwindow* window, unsigned int codepoint){
	Events::codepoints.push_back(codepoint);
}

const char* glfwErrorName(int error) {
	switch (error) {
		case GLFW_NO_ERROR: return "no error";
		case GLFW_NOT_INITIALIZED: return "not initialized";
		case GLFW_NO_CURRENT_CONTEXT: return "no current context";
		case GLFW_INVALID_ENUM: return "invalid enum";
		case GLFW_INVALID_VALUE: return "invalid value";
		case GLFW_OUT_OF_MEMORY: return "out of memory";
		case GLFW_API_UNAVAILABLE: return "api unavailable";
		case GLFW_VERSION_UNAVAILABLE: return "version unavailable";
		case GLFW_PLATFORM_ERROR: return "platform error";
		case GLFW_FORMAT_UNAVAILABLE: return "format unavailable";
		case GLFW_NO_WINDOW_CONTEXT: return "no window context";
		default: return "unknown error";
	}
}

void error_callback(int error, const char* description) {
	cerr << "GLFW error [0x" << std::hex << error << "]: ";
	cerr << glfwErrorName(error) << endl;
	if (description) {
		cerr << description << endl;
	}
}

int Window::initialize(DisplaySettings& settings){
	Window::settings = &settings;
	Window::width_ = settings.width;
	Window::height_ = settings.height;

	glfwSetErrorCallback(error_callback);
	if (glfwInit() == GLFW_FALSE) {
		cerr << "Failed to initialize GLFW" << endl;
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
	glfwWindowHint(GLFW_SAMPLES, settings.samples);

	window = glfwCreateWindow(width_, height_, settings.title.c_str(), nullptr, nullptr);
	if (window == nullptr){
		cerr << "Failed to create GLFW Window" << endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;
	GLenum glewErr = glewInit();
	if (glewErr != GLEW_OK){
		cerr << "Failed to initialize GLEW: " << std::endl;
		cerr << glewGetErrorString(glewErr) << std::endl;
		return -1;
	}

	glViewport(0,0, width_, height_);
	glClearColor(0.0f,0.0f,0.0f, 1);
	glEnable(GL_BLEND);
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
	const GLubyte* vendor = glGetString(GL_VENDOR);
	const GLubyte* renderer = glGetString(GL_RENDERER);
	cout << "GL Vendor: " << (char*)vendor << endl;
	cout << "GL Renderer: " << (char*)renderer << endl;
	return 0;
}

void Window::clear() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Window::clearDepth() {
	glClear(GL_DEPTH_BUFFER_BIT);
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
	scissorArea = vec4(0.0f, 0.0f, width_, height_);
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
		glScissor(area.x, Window::height_-area.w, 
				  std::max(0, int(area.z-area.x)), 
				  std::max(0, int(area.w-area.y)));
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
		glScissor(area.x, Window::height_-area.w, 
				  std::max(0, int(area.z-area.x)), 
				  std::max(0, int(area.w-area.y)));
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
		glfwGetWindowPos(window, &posX_, &posY_);
		glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, GLFW_DONT_CARE);
	}
	else {
		glfwSetWindowMonitor(window, nullptr, posX_, posY_, settings->width, settings->height, GLFW_DONT_CARE);
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
	ubyte* data = new ubyte[width_ * height_ * 3];
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, width_, height_, GL_RGB, GL_UNSIGNED_BYTE, data);
	return new ImageData(ImageFormat::rgb888, width_, height_, data);
}

bool Window::tryToMaximize(GLFWwindow* window, GLFWmonitor* monitor) {
	glm::ivec4 windowFrame(0);
	glm::ivec4 workArea(0);
	glfwGetWindowFrameSize(window, &windowFrame.x, &windowFrame.y, &windowFrame.z, &windowFrame.w);
	glfwGetMonitorWorkarea(monitor, &workArea.x, &workArea.y, &workArea.z, &workArea.w);
	if (Window::width_ > (uint)workArea.z) Window::width_ = (uint)workArea.z;
	if (Window::height_ > (uint)workArea.w) Window::height_ = (uint)workArea.w;
	if (Window::width_ >= (uint)(workArea.z - (windowFrame.x + windowFrame.z)) &&
		Window::height_ >= (uint)(workArea.w - (windowFrame.y + windowFrame.w))) {
		glfwMaximizeWindow(window);
		return true;
	}
	glfwSetWindowSize(window, Window::width_, Window::height_);
	glfwSetWindowPos(window, workArea.x + (workArea.z - Window::width_) / 2, 
							 workArea.y + (workArea.w - Window::height_) / 2 + windowFrame.y / 2);
	return false;
}

void Window::setWidth(uint w){
	if (w > 0) { Window::width_ = w;}
}

void Window::setHeight(uint h){
	if (h > 0) { Window::height_ = h;}
}