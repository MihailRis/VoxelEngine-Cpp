#include "Batch2D.h"
#include "Mesh.h"
#include "Texture.h"

#include <GL/glew.h>

Batch2D::Batch2D(size_t capacity) : capacity(capacity),
									offset(0),
									color(1.0f, 1.0f, 1.0f, 1.0f){
	const int attrs[] = {
			2, 2, 4, 0 //null terminator
	};

	buffer = new float[capacity * 8];
	mesh = new Mesh(buffer, 0, attrs);
	index = 0;

	unsigned char pixels[] = {
			255, 255, 255, 255,
	};
	blank = new Texture(pixels, 1, 1);
	_texture = nullptr;
}

Batch2D::~Batch2D(){
	delete blank;
	delete buffer;
	delete mesh;
}

void Batch2D::begin(){
	_texture = nullptr;
	blank->bind();
}

void Batch2D::vertex(float x, float y,
		float u, float v,
		float r, float g, float b, float a) {
	buffer[index++] = x;
	buffer[index++] = y;
	buffer[index++] = u;
	buffer[index++] = v;
	buffer[index++] = r;
	buffer[index++] = g;
	buffer[index++] = b;
	buffer[index++] = a;
}

void Batch2D::texture(Texture* new_texture){
	if (_texture == new_texture)
		return;
	_texture = new_texture;
	if (new_texture == nullptr)
		blank->bind();
	else
		new_texture->bind();
}

void Batch2D::rect(float x, float y, float w, float h){
	const float r = color.r;
	const float g = color.g;
	const float b = color.b;
	const float a = color.a;

	vertex(x, y, 0, 0, r,g,b,a);
	vertex(x+w, y+h, 1, 1, r,g,b,a);
	vertex(x, y+h, 0, 1, r,g,b,a);

	vertex(x, y, 0, 0, r,g,b,a);
	vertex(x+w, y, 1, 0, r,g,b,a);
	vertex(x+w, y+h, 1, 1, r,g,b,a);
}

void Batch2D::rect(float x, float y, float w, float h,
					float u, float v, float tx, float ty,
					float r, float g, float b, float a){
	vertex(x, y, u, v, r,g,b,a);
	vertex(x+w, y+h, u+tx, v+ty, r,g,b,a);
	vertex(x, y+h, u, v+ty, r,g,b,a);

	vertex(x, y, u, v, r,g,b,a);
	vertex(x+w, y, u+tx, v, r,g,b,a);
	vertex(x+w, y+h, u+tx, v+ty, r,g,b,a);
}

void Batch2D::render() {
	mesh->reload(buffer, index / 8);
	mesh->draw(GL_TRIANGLES);
	index = 0;
}
