#include "Batch3D.h"

#include "Mesh.h"
#include "Texture.h"

#include <GL/glew.h>

#define VERTEX_SIZE 9

Batch3D::Batch3D(size_t capacity) : capacity(capacity), offset(0), color(1.0f, 1.0f, 1.0f, 1.0f){
	const int attrs[] = {
			3, 2, 4, 0 //null terminator
	};

	buffer = new float[capacity * VERTEX_SIZE];
	mesh = new Mesh(buffer, 0, attrs);
	index = 0;

	unsigned char pixels[] = {
			255, 255, 255, 255,
	};
	blank = new Texture(pixels, 1, 1);
	_texture = nullptr;
}

Batch3D::~Batch3D(){
	delete blank;
	delete[] buffer;
	delete mesh;
}

void Batch3D::begin(){
	_texture = nullptr;
	blank->bind();
}

void Batch3D::vertex(float x, float y, float z, float u, float v,
		float r, float g, float b, float a) {
	buffer[index++] = x;
	buffer[index++] = y;
	buffer[index++] = z;
	buffer[index++] = u;
	buffer[index++] = v;
	buffer[index++] = r;
	buffer[index++] = g;
	buffer[index++] = b;
	buffer[index++] = a;
}
void Batch3D::vertex(vec3 point,
		vec2 uvpoint,
		float r, float g, float b, float a) {
	buffer[index++] = point.x;
	buffer[index++] = point.y;
	buffer[index++] = point.z;
	buffer[index++] = uvpoint.x;
	buffer[index++] = uvpoint.y;
	buffer[index++] = r;
	buffer[index++] = g;
	buffer[index++] = b;
	buffer[index++] = a;
}

void Batch3D::texture(Texture* new_texture){
	if (_texture == new_texture)
		return;
	render();
	_texture = new_texture;
	if (new_texture == nullptr)
		blank->bind();
	else
		new_texture->bind();
}

void Batch3D::sprite(vec3 pos, vec3 up, vec3 right, float w, float h){
	const float r = color.r;
	const float g = color.g;
	const float b = color.b;
	const float a = color.a;
	if (index + 6*VERTEX_SIZE >= capacity)
		render();

	vertex(pos.x - right.x * w - up.x * h,
			pos.y - right.y * w - up.y * h,
			pos.z - right.z * w - up.z * h,
			0, 0,
			r,g,b,a);

	vertex(pos.x + right.x * w + up.x * h,
			pos.y + right.y * w + up.y * h,
			pos.z + right.z * w + up.z * h,
			1, 1,
			r,g,b,a);

	vertex(pos.x - right.x * w - up.x * h,
			pos.y + right.y * w + up.y * h,
			pos.z - right.z * w - up.z * h,
			0, 1,
			r,g,b,a);

	vertex(pos.x - right.x * w - up.x * h,
			pos.y - right.y * w - up.y * h,
			pos.z - right.z * w - up.z * h,
			0, 0,
			r,g,b,a);

	vertex(pos.x + right.x * w + up.x * h,
			pos.y - right.y * w - up.y * h,
			pos.z + right.z * w + up.z * h,
			1, 0,
			r,g,b,a);

	vertex(pos.x + right.x * w + up.x * h,
			pos.y + right.y * w + up.y * h,
			pos.z + right.z * w + up.z * h,
			1, 1,
			r,g,b,a);
}

void Batch3D::sprite(vec3 pos, vec3 up, vec3 right, float w, float h, int atlasRes, int index, vec4 tint){
	float scale = 1.0f / (float)atlasRes;
	float u = (index % atlasRes) * scale;
	float v = 1.0f - ((index / atlasRes) * scale) - scale;

	vertex(pos.x - right.x * w - up.x * h,
			pos.y - right.y * w - up.y * h,
			pos.z - right.z * w - up.z * h,
			u, v,
			tint.r,tint.g,tint.b,tint.a);

	vertex(pos.x + right.x * w + up.x * h,
			pos.y + right.y * w + up.y * h,
			pos.z + right.z * w + up.z * h,
			u+scale, v+scale,
			tint.r,tint.g,tint.b,tint.a);

	vertex(pos.x - right.x * w - up.x * h,
			pos.y + right.y * w + up.y * h,
			pos.z - right.z * w - up.z * h,
			u, v+scale,
			tint.r,tint.g,tint.b,tint.a);

	vertex(pos.x - right.x * w - up.x * h,
			pos.y - right.y * w - up.y * h,
			pos.z - right.z * w - up.z * h,
			u, v,
			tint.r,tint.g,tint.b,tint.a);

	vertex(pos.x + right.x * w + up.x * h,
			pos.y - right.y * w - up.y * h,
			pos.z + right.z * w + up.z * h,
			u+scale, v,
			tint.r,tint.g,tint.b,tint.a);

	vertex(pos.x + right.x * w + up.x * h,
			pos.y + right.y * w + up.y * h,
			pos.z + right.z * w + up.z * h,
			u+scale, v+scale,
			tint.r,tint.g,tint.b,tint.a);
}

void Batch3D::render() {
	mesh->reload(buffer, index / VERTEX_SIZE);
	mesh->draw(GL_TRIANGLES);
	index = 0;
}
