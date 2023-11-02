#include "Mesh.h"
#include <GL/glew.h>

int Mesh::meshesCount = 0;

Mesh::Mesh(const float* buffer, size_t vertices, const vattr* attrs) : vertices(vertices){
	meshesCount++;
	vertexSize = 0;
	for (int i = 0; attrs[i].size; i++){
		vertexSize += attrs[i].size;
	}

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	if (buffer){
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexSize * vertices, buffer, GL_STATIC_DRAW);
	} else {
		glBufferData(GL_ARRAY_BUFFER, 0, {}, GL_STATIC_DRAW);
	}


	// attributes
	int offset = 0;
	for (int i = 0; attrs[i].size; i++){
		int size = attrs[i].size;
		glVertexAttribPointer(i, size, GL_FLOAT, GL_FALSE, vertexSize * sizeof(float), (GLvoid*)(offset * sizeof(float)));
		glEnableVertexAttribArray(i);
		offset += size;
	}

	glBindVertexArray(0);
}

Mesh::~Mesh(){
	meshesCount--;
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
}

void Mesh::reload(const float* buffer, size_t vertices){
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexSize * vertices, buffer, GL_STATIC_DRAW);
	this->vertices = vertices;
}

void Mesh::draw(unsigned int primitive){
	glBindVertexArray(vao);
	glDrawArrays(primitive, 0, vertices);
	glBindVertexArray(0);
}

void Mesh::draw() {
	draw(GL_TRIANGLES);
}