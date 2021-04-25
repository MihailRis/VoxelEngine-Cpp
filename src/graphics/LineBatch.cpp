/*
 * LineBatch.cpp
 *
 *  Created on: Jun 25, 2020
 *      Author: MihailRis
 */

#include "LineBatch.h"
#include "Mesh.h"

#include <GL/glew.h>

#define LB_VERTEX_SIZE (3+4)

LineBatch::LineBatch(size_t capacity) : capacity(capacity) {
	int attrs[] = {3,4, 0};
	buffer = new float[capacity * LB_VERTEX_SIZE * 2];
	mesh = new Mesh(buffer, 0, attrs);
	index = 0;
}

LineBatch::~LineBatch(){
	delete[] buffer;
	delete mesh;
}

void LineBatch::line(float x1, float y1, float z1, float x2, float y2, float z2,
		float r, float g, float b, float a) {
	buffer[index] = x1;
	buffer[index+1] = y1;
	buffer[index+2] = z1;
	buffer[index+3] = r;
	buffer[index+4] = g;
	buffer[index+5] = b;
	buffer[index+6] = a;
	index += LB_VERTEX_SIZE;

	buffer[index] = x2;
	buffer[index+1] = y2;
	buffer[index+2] = z2;
	buffer[index+3] = r;
	buffer[index+4] = g;
	buffer[index+5] = b;
	buffer[index+6] = a;
	index += LB_VERTEX_SIZE;
}

void LineBatch::box(float x, float y, float z, float w, float h, float d,
		float r, float g, float b, float a) {
	w *= 0.5f;
	h *= 0.5f;
	d *= 0.5f;

	line(x-w, y-h, z-d, x+w, y-h, z-d, r,g,b,a);
	line(x-w, y+h, z-d, x+w, y+h, z-d, r,g,b,a);
	line(x-w, y-h, z+d, x+w, y-h, z+d, r,g,b,a);
	line(x-w, y+h, z+d, x+w, y+h, z+d, r,g,b,a);

	line(x-w, y-h, z-d, x-w, y+h, z-d, r,g,b,a);
	line(x+w, y-h, z-d, x+w, y+h, z-d, r,g,b,a);
	line(x-w, y-h, z+d, x-w, y+h, z+d, r,g,b,a);
	line(x+w, y-h, z+d, x+w, y+h, z+d, r,g,b,a);

	line(x-w, y-h, z-d, x-w, y-h, z+d, r,g,b,a);
	line(x+w, y-h, z-d, x+w, y-h, z+d, r,g,b,a);
	line(x-w, y+h, z-d, x-w, y+h, z+d, r,g,b,a);
	line(x+w, y+h, z-d, x+w, y+h, z+d, r,g,b,a);
}

void LineBatch::render(){
	if (index == 0)
		return;
	mesh->reload(buffer, index / LB_VERTEX_SIZE);
	mesh->draw(GL_LINES);
	index = 0;
}
