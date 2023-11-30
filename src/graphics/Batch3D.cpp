#include "Batch3D.h"

#include "Mesh.h"
#include "Texture.h"

#include <GL/glew.h>
#include "../typedefs.h"

#define VERTEX_SIZE 9

using glm::vec2;
using glm::vec3;
using glm::vec4;

Batch3D::Batch3D(size_t capacity) 
	: capacity(capacity), 
	  offset(0), 
	  color(1.0f, 1.0f, 1.0f, 1.0f) {
	const vattr attrs[] = {
		{3}, {2}, {4}, {0}
	};

	buffer = new float[capacity * VERTEX_SIZE];
	mesh = new Mesh(buffer, 0, attrs);
	index = 0;

	ubyte pixels[] = {
		255, 255, 255, 255,
	};
	blank = new Texture(pixels, 1, 1, GL_RGBA);
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
void Batch3D::vertex(vec3 coord, float u, float v,
					 float r, float g, float b, float a) {
	buffer[index++] = coord.x;
	buffer[index++] = coord.y;
	buffer[index++] = coord.z;
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

void Batch3D::face(const vec3& coord, float w, float h,
	const vec3& axisX,
	const vec3& axisY,
	const UVRegion& region,
	const vec4& tint) {
	if (index + VERTEX_SIZE * 4 > capacity) {
		render();
	}
	vertex(coord, region.u1, region.v1, 
		   tint.r, tint.g, tint.b, tint.a);
	vertex(coord + axisX * w, region.u2, region.v1, 
	 	   tint.r, tint.g, tint.b, tint.a);
	vertex(coord + axisX * w + axisY * h, region.u2, region.v2, 
		   tint.r, tint.g, tint.b, tint.a);

	vertex(coord, region.u1, region.v1, 
		   tint.r, tint.g, tint.b, tint.a);
	vertex(coord + axisX * w + axisY * h, region.u2, region.v2,
		   tint.r, tint.g, tint.b, tint.a);
	vertex(coord + axisY * h, region.u1, region.v2, 
		   tint.r, tint.g, tint.b, tint.a);
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

inline vec4 do_tint(float value) {
	return vec4(value, value, value, 1.0f);
}

void Batch3D::blockCube(const vec3& size, const UVRegion(&texfaces)[6], ubyte group) {
	face(vec3(0.0f, 0.0f, 0.0f), size.x, size.y, vec3(1, 0, 0), vec3(0, 1, 0), texfaces[5], do_tint(1.0));
	face(vec3(size.x, 0.0f, -size.z), size.x, size.y, vec3(-1, 0, 0), vec3(0, 1, 0), texfaces[4], vec4(1.0f));
	face(vec3(0.0f, size.y, 0.0f), size.x, size.z, vec3(1, 0, 0), vec3(0, 0, -1), texfaces[3], vec4(1.0f));
	face(vec3(0.0f, 0.0f, -size.z), size.x, size.z, vec3(1, 0, 0), vec3(0, 0, 1), texfaces[2], vec4(1.0f));
	face(vec3(0.0f, 0.0f, -size.z), size.z, size.y, vec3(0, 0, 1), vec3(0, 1, 0), texfaces[0], vec4(1.0f));
	face(vec3(size.x, 0.0f, 0.0f), size.z, size.y, vec3(0, 0, -1), vec3(0, 1, 0), texfaces[1], vec4(1.0f));
}

void Batch3D::render() {
	mesh->reload(buffer, index / VERTEX_SIZE);
	mesh->draw();
	index = 0;
}
