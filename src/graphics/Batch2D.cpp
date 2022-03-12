#include "Batch2D.h"
#include "Mesh.h"

Batch2D::Batch2D(size_t capacity) : capacity(capacity),
									offset(0),
									color(1.0f, 1.0f, 1.0f, 1.0f){
	const int attrs[] = {
			2, 2, 4, 0 //null terminator
	};

	buffer = new float[capacity];
	mesh = new Mesh(nullptr, 0, attrs);
}

Batch2D::~Batch2D(){
	delete buffer;
	delete mesh;
}
