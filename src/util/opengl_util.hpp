#pragma once

/**
 * @brief Проверяет, поддерживается ли указанное расширение OpenGL.
 *
 * @param extension Имя проверяемого расширения (например, "GL_KHR_debug").
 * @return true, если расширение поддерживается, иначе false.
 */
bool isGlExtensionSupported(const char *extension);
