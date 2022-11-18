#include "VoxelRenderer.h"

#include <iostream>
#include <math.h>
#include "Mesh.h"
#include "../voxels/Chunk.h"
#include "../voxels/voxel.h"
#include "../voxels/Block.h"
#include "../lighting/Lightmap.h"

#define CDIV(X,A) (((X) < 0) ? ((X) / (A) - 1) : ((X) / (A)))
#define LOCAL_NEG(X, SIZE) (((X) < 0) ? ((SIZE)+(X)) : (X))
#define LOCAL(X, SIZE) ((X) >= (SIZE) ? ((X) - (SIZE)) : LOCAL_NEG(X, SIZE))
#define IS_CHUNK(X,Y,Z) (GET_CHUNK(X,Y,Z) != nullptr)
#define GET_CHUNK(X,Y,Z) (chunks[(CDIV(Z, CHUNK_D) + 1) * 3 + CDIV(X, CHUNK_W) + 1])

#define LIGHT(X,Y,Z, CHANNEL) (IS_CHUNK(X,Y,Z) ? GET_CHUNK(X,Y,Z)->lightmap->get(LOCAL(X, CHUNK_W), LOCAL(Y, CHUNK_H), LOCAL(Z, CHUNK_D), (CHANNEL)) : 0)
#define VOXEL(X,Y,Z) (GET_CHUNK(X,Y,Z)->voxels[(LOCAL(Y, CHUNK_H) * CHUNK_D + LOCAL(Z, CHUNK_D)) * CHUNK_W + LOCAL(X, CHUNK_W)])
#define IS_BLOCKED(X,Y,Z,GROUP) ((!IS_CHUNK(X, Y, Z)) || Block::blocks[VOXEL(X, Y, Z).id]->drawGroup == (GROUP))

#define VERTEX(INDEX, X,Y,Z, U,V, R,G,B,S) buffer.push_back(X);\
								  buffer.push_back(Y);\
								  buffer.push_back(Z);\
								  buffer.push_back(U);\
								  buffer.push_back(V);\
								  buffer.push_back(R);\
								  buffer.push_back(G);\
								  buffer.push_back(B);\
								  buffer.push_back(S);\
								  INDEX += CHUNK_VERTEX_SIZE;


#define SETUP_UV(INDEX) float u1 = ((INDEX) % 16) * uvsize;\
				float v1 = 1-((1 + (INDEX) / 16) * uvsize);\
				float u2 = u1 + uvsize;\
				float v2 = v1 + uvsize;

int chunk_attrs[] = {3,2,4, 0};

VoxelRenderer::VoxelRenderer() {
}

VoxelRenderer::~VoxelRenderer(){
}

inline void _renderBlock(std::vector<float>& buffer, int x, int y, int z, const Chunk** chunks, unsigned int id, size_t& index){
	float l;
	float uvsize = 1.0f/16.0f;

	Block* block = Block::blocks[id];
	unsigned char group = block->drawGroup;

	if (!IS_BLOCKED(x,y+1,z,group)){
		l = 1.0f;

		SETUP_UV(block->textureFaces[3]);

		const float lr = LIGHT(x,y+1,z, 0) / 15.0f;
		const float lg = LIGHT(x,y+1,z, 1) / 15.0f;
		const float lb = LIGHT(x,y+1,z, 2) / 15.0f;
		const float ls = LIGHT(x,y+1,z, 3) / 15.0f;

		float lr0 = (LIGHT(x-1,y+1,z,0) + lr*30 + LIGHT(x-1,y+1,z-1,0) + LIGHT(x,y+1,z-1,0)) / 75.0f;
		float lr1 = (LIGHT(x-1,y+1,z,0) + lr*30 + LIGHT(x-1,y+1,z+1,0) + LIGHT(x,y+1,z+1,0)) / 75.0f;
		float lr2 = (LIGHT(x+1,y+1,z,0) + lr*30 + LIGHT(x+1,y+1,z+1,0) + LIGHT(x,y+1,z+1,0)) / 75.0f;
		float lr3 = (LIGHT(x+1,y+1,z,0) + lr*30 + LIGHT(x+1,y+1,z-1,0) + LIGHT(x,y+1,z-1,0)) / 75.0f;

		float lg0 = (LIGHT(x-1,y+1,z,1) + lg*30 + LIGHT(x-1,y+1,z-1,1) + LIGHT(x,y+1,z-1,1)) / 75.0f;
		float lg1 = (LIGHT(x-1,y+1,z,1) + lg*30 + LIGHT(x-1,y+1,z+1,1) + LIGHT(x,y+1,z+1,1)) / 75.0f;
		float lg2 = (LIGHT(x+1,y+1,z,1) + lg*30 + LIGHT(x+1,y+1,z+1,1) + LIGHT(x,y+1,z+1,1)) / 75.0f;
		float lg3 = (LIGHT(x+1,y+1,z,1) + lg*30 + LIGHT(x+1,y+1,z-1,1) + LIGHT(x,y+1,z-1,1)) / 75.0f;

		float lb0 = (LIGHT(x-1,y+1,z,2) + lb*30 + LIGHT(x-1,y+1,z-1,2) + LIGHT(x,y+1,z-1,2)) / 75.0f;
		float lb1 = (LIGHT(x-1,y+1,z,2) + lb*30 + LIGHT(x-1,y+1,z+1,2) + LIGHT(x,y+1,z+1,2)) / 75.0f;
		float lb2 = (LIGHT(x+1,y+1,z,2) + lb*30 + LIGHT(x+1,y+1,z+1,2) + LIGHT(x,y+1,z+1,2)) / 75.0f;
		float lb3 = (LIGHT(x+1,y+1,z,2) + lb*30 + LIGHT(x+1,y+1,z-1,2) + LIGHT(x,y+1,z-1,2)) / 75.0f;

		float ls0 = (LIGHT(x-1,y+1,z,3) + ls*30 + LIGHT(x-1,y+1,z-1,3) + LIGHT(x,y+1,z-1,3)) / 75.0f;
		float ls1 = (LIGHT(x-1,y+1,z,3) + ls*30 + LIGHT(x-1,y+1,z+1,3) + LIGHT(x,y+1,z+1,3)) / 75.0f;
		float ls2 = (LIGHT(x+1,y+1,z,3) + ls*30 + LIGHT(x+1,y+1,z+1,3) + LIGHT(x,y+1,z+1,3)) / 75.0f;
		float ls3 = (LIGHT(x+1,y+1,z,3) + ls*30 + LIGHT(x+1,y+1,z-1,3) + LIGHT(x,y+1,z-1,3)) / 75.0f;

		VERTEX(index, x-0.5f, y+0.5f, z-0.5f, u2,v1, lr0, lg0, lb0, ls0);
		VERTEX(index, x-0.5f, y+0.5f, z+0.5f, u2,v2, lr1, lg1, lb1, ls1);
		VERTEX(index, x+0.5f, y+0.5f, z+0.5f, u1,v2, lr2, lg2, lb2, ls2);

		VERTEX(index, x-0.5f, y+0.5f, z-0.5f, u2,v1, lr0, lg0, lb0, ls0);
		VERTEX(index, x+0.5f, y+0.5f, z+0.5f, u1,v2, lr2, lg2, lb2, ls2);
		VERTEX(index, x+0.5f, y+0.5f, z-0.5f, u1,v1, lr3, lg3, lb3, ls3);
	}
	if (!IS_BLOCKED(x,y-1,z,group)){
		l = 0.75f;

		SETUP_UV(block->textureFaces[2]);

		const float lr = LIGHT(x,y-1,z, 0) / 15.0f;
		const float lg = LIGHT(x,y-1,z, 1) / 15.0f;
		const float lb = LIGHT(x,y-1,z, 2) / 15.0f;
		const float ls = LIGHT(x,y-1,z, 3) / 15.0f;

		float lr0 = (LIGHT(x-1,y-1,z-1,0) + lr*30 + LIGHT(x-1,y-1,z,0) + LIGHT(x,y-1,z-1,0)) / 75.0f;
		float lr1 = (LIGHT(x+1,y-1,z+1,0) + lr*30 + LIGHT(x+1,y-1,z,0) + LIGHT(x,y-1,z+1,0)) / 75.0f;
		float lr2 = (LIGHT(x-1,y-1,z+1,0) + lr*30 + LIGHT(x-1,y-1,z,0) + LIGHT(x,y-1,z+1,0)) / 75.0f;
		float lr3 = (LIGHT(x+1,y-1,z-1,0) + lr*30 + LIGHT(x+1,y-1,z,0) + LIGHT(x,y-1,z-1,0)) / 75.0f;

		float lg0 = (LIGHT(x-1,y-1,z-1,1) + lg*30 + LIGHT(x-1,y-1,z,1) + LIGHT(x,y-1,z-1,1)) / 75.0f;
		float lg1 = (LIGHT(x+1,y-1,z+1,1) + lg*30 + LIGHT(x+1,y-1,z,1) + LIGHT(x,y-1,z+1,1)) / 75.0f;
		float lg2 = (LIGHT(x-1,y-1,z+1,1) + lg*30 + LIGHT(x-1,y-1,z,1) + LIGHT(x,y-1,z+1,1)) / 75.0f;
		float lg3 = (LIGHT(x+1,y-1,z-1,1) + lg*30 + LIGHT(x+1,y-1,z,1) + LIGHT(x,y-1,z-1,1)) / 75.0f;

		float lb0 = (LIGHT(x-1,y-1,z-1,2) + lb*30 + LIGHT(x-1,y-1,z,2) + LIGHT(x,y-1,z-1,2)) / 75.0f;
		float lb1 = (LIGHT(x+1,y-1,z+1,2) + lb*30 + LIGHT(x+1,y-1,z,2) + LIGHT(x,y-1,z+1,2)) / 75.0f;
		float lb2 = (LIGHT(x-1,y-1,z+1,2) + lb*30 + LIGHT(x-1,y-1,z,2) + LIGHT(x,y-1,z+1,2)) / 75.0f;
		float lb3 = (LIGHT(x+1,y-1,z-1,2) + lb*30 + LIGHT(x+1,y-1,z,2) + LIGHT(x,y-1,z-1,2)) / 75.0f;

		float ls0 = (LIGHT(x-1,y-1,z-1,3) + ls*30 + LIGHT(x-1,y-1,z,3) + LIGHT(x,y-1,z-1,3)) / 75.0f;
		float ls1 = (LIGHT(x+1,y-1,z+1,3) + ls*30 + LIGHT(x+1,y-1,z,3) + LIGHT(x,y-1,z+1,3)) / 75.0f;
		float ls2 = (LIGHT(x-1,y-1,z+1,3) + ls*30 + LIGHT(x-1,y-1,z,3) + LIGHT(x,y-1,z+1,3)) / 75.0f;
		float ls3 = (LIGHT(x+1,y-1,z-1,3) + ls*30 + LIGHT(x+1,y-1,z,3) + LIGHT(x,y-1,z-1,3)) / 75.0f;

		VERTEX(index, x-0.5f, y-0.5f, z-0.5f, u1,v1, lr0,lg0,lb0,ls0);
		VERTEX(index, x+0.5f, y-0.5f, z+0.5f, u2,v2, lr1,lg1,lb1,ls1);
		VERTEX(index, x-0.5f, y-0.5f, z+0.5f, u1,v2, lr2,lg2,lb2,ls2);

		VERTEX(index, x-0.5f, y-0.5f, z-0.5f, u1,v1, lr0,lg0,lb0,ls0);
		VERTEX(index, x+0.5f, y-0.5f, z-0.5f, u2,v1, lr3,lg3,lb3,ls3);
		VERTEX(index, x+0.5f, y-0.5f, z+0.5f, u2,v2, lr1,lg1,lb1,ls1);
	}

	if (!IS_BLOCKED(x+1,y,z,group)){
		l = 0.95f;

		SETUP_UV(block->textureFaces[1]);

		const float lr = LIGHT(x+1,y,z, 0) / 15.0f;
		const float lg = LIGHT(x+1,y,z, 1) / 15.0f;
		const float lb = LIGHT(x+1,y,z, 2) / 15.0f;
		const float ls = LIGHT(x+1,y,z, 3) / 15.0f;

		float lr0 = (LIGHT(x+1,y-1,z-1,0) + lr*30 + LIGHT(x+1,y,z-1,0) + LIGHT(x+1,y-1,z,0)) / 75.0f;
		float lr1 = (LIGHT(x+1,y+1,z-1,0) + lr*30 + LIGHT(x+1,y,z-1,0) + LIGHT(x+1,y+1,z,0)) / 75.0f;
		float lr2 = (LIGHT(x+1,y+1,z+1,0) + lr*30 + LIGHT(x+1,y,z+1,0) + LIGHT(x+1,y+1,z,0)) / 75.0f;
		float lr3 = (LIGHT(x+1,y-1,z+1,0) + lr*30 + LIGHT(x+1,y,z+1,0) + LIGHT(x+1,y-1,z,0)) / 75.0f;

		float lg0 = (LIGHT(x+1,y-1,z-1,1) + lg*30 + LIGHT(x+1,y,z-1,1) + LIGHT(x+1,y-1,z,1)) / 75.0f;
		float lg1 = (LIGHT(x+1,y+1,z-1,1) + lg*30 + LIGHT(x+1,y,z-1,1) + LIGHT(x+1,y+1,z,1)) / 75.0f;
		float lg2 = (LIGHT(x+1,y+1,z+1,1) + lg*30 + LIGHT(x+1,y,z+1,1) + LIGHT(x+1,y+1,z,1)) / 75.0f;
		float lg3 = (LIGHT(x+1,y-1,z+1,1) + lg*30 + LIGHT(x+1,y,z+1,1) + LIGHT(x+1,y-1,z,1)) / 75.0f;

		float lb0 = (LIGHT(x+1,y-1,z-1,2) + lb*30 + LIGHT(x+1,y,z-1,2) + LIGHT(x+1,y-1,z,2)) / 75.0f;
		float lb1 = (LIGHT(x+1,y+1,z-1,2) + lb*30 + LIGHT(x+1,y,z-1,2) + LIGHT(x+1,y+1,z,2)) / 75.0f;
		float lb2 = (LIGHT(x+1,y+1,z+1,2) + lb*30 + LIGHT(x+1,y,z+1,2) + LIGHT(x+1,y+1,z,2)) / 75.0f;
		float lb3 = (LIGHT(x+1,y-1,z+1,2) + lb*30 + LIGHT(x+1,y,z+1,2) + LIGHT(x+1,y-1,z,2)) / 75.0f;

		float ls0 = (LIGHT(x+1,y-1,z-1,3) + ls*30 + LIGHT(x+1,y,z-1,3) + LIGHT(x+1,y-1,z,3)) / 75.0f;
		float ls1 = (LIGHT(x+1,y+1,z-1,3) + ls*30 + LIGHT(x+1,y,z-1,3) + LIGHT(x+1,y+1,z,3)) / 75.0f;
		float ls2 = (LIGHT(x+1,y+1,z+1,3) + ls*30 + LIGHT(x+1,y,z+1,3) + LIGHT(x+1,y+1,z,3)) / 75.0f;
		float ls3 = (LIGHT(x+1,y-1,z+1,3) + ls*30 + LIGHT(x+1,y,z+1,3) + LIGHT(x+1,y-1,z,3)) / 75.0f;

		VERTEX(index, x+0.5f, y-0.5f, z-0.5f, u2,v1, lr0,lg0,lb0,ls0);
		VERTEX(index, x+0.5f, y+0.5f, z-0.5f, u2,v2, lr1,lg1,lb1,ls1);
		VERTEX(index, x+0.5f, y+0.5f, z+0.5f, u1,v2, lr2,lg2,lb2,ls2);

		VERTEX(index, x+0.5f, y-0.5f, z-0.5f, u2,v1, lr0,lg0,lb0,ls0);
		VERTEX(index, x+0.5f, y+0.5f, z+0.5f, u1,v2, lr2,lg2,lb2,ls2);
		VERTEX(index, x+0.5f, y-0.5f, z+0.5f, u1,v1, lr3,lg3,lb3,ls3);
	}
	if (!IS_BLOCKED(x-1,y,z,group)){
		l = 0.85f;

		SETUP_UV(block->textureFaces[0]);

		const float lr = LIGHT(x-1,y,z, 0) / 15.0f;
		const float lg = LIGHT(x-1,y,z, 1) / 15.0f;
		const float lb = LIGHT(x-1,y,z, 2) / 15.0f;
		const float ls = LIGHT(x-1,y,z, 3) / 15.0f;

		float lr0 = (LIGHT(x-1,y-1,z-1,0) + lr*30 + LIGHT(x-1,y,z-1,0) + LIGHT(x-1,y-1,z,0)) / 75.0f;
		float lr1 = (LIGHT(x-1,y+1,z+1,0) + lr*30 + LIGHT(x-1,y,z+1,0) + LIGHT(x-1,y+1,z,0)) / 75.0f;
		float lr2 = (LIGHT(x-1,y+1,z-1,0) + lr*30 + LIGHT(x-1,y,z-1,0) + LIGHT(x-1,y+1,z,0)) / 75.0f;
		float lr3 = (LIGHT(x-1,y-1,z+1,0) + lr*30 + LIGHT(x-1,y,z+1,0) + LIGHT(x-1,y-1,z,0)) / 75.0f;

		float lg0 = (LIGHT(x-1,y-1,z-1,1) + lg*30 + LIGHT(x-1,y,z-1,1) + LIGHT(x-1,y-1,z,1)) / 75.0f;
		float lg1 = (LIGHT(x-1,y+1,z+1,1) + lg*30 + LIGHT(x-1,y,z+1,1) + LIGHT(x-1,y+1,z,1)) / 75.0f;
		float lg2 = (LIGHT(x-1,y+1,z-1,1) + lg*30 + LIGHT(x-1,y,z-1,1) + LIGHT(x-1,y+1,z,1)) / 75.0f;
		float lg3 = (LIGHT(x-1,y-1,z+1,1) + lg*30 + LIGHT(x-1,y,z+1,1) + LIGHT(x-1,y-1,z,1)) / 75.0f;

		float lb0 = (LIGHT(x-1,y-1,z-1,2) + lb*30 + LIGHT(x-1,y,z-1,2) + LIGHT(x-1,y-1,z,2)) / 75.0f;
		float lb1 = (LIGHT(x-1,y+1,z+1,2) + lb*30 + LIGHT(x-1,y,z+1,2) + LIGHT(x-1,y+1,z,2)) / 75.0f;
		float lb2 = (LIGHT(x-1,y+1,z-1,2) + lb*30 + LIGHT(x-1,y,z-1,2) + LIGHT(x-1,y+1,z,2)) / 75.0f;
		float lb3 = (LIGHT(x-1,y-1,z+1,2) + lb*30 + LIGHT(x-1,y,z+1,2) + LIGHT(x-1,y-1,z,2)) / 75.0f;

		float ls0 = (LIGHT(x-1,y-1,z-1,3) + ls*30 + LIGHT(x-1,y,z-1,3) + LIGHT(x-1,y-1,z,3)) / 75.0f;
		float ls1 = (LIGHT(x-1,y+1,z+1,3) + ls*30 + LIGHT(x-1,y,z+1,3) + LIGHT(x-1,y+1,z,3)) / 75.0f;
		float ls2 = (LIGHT(x-1,y+1,z-1,3) + ls*30 + LIGHT(x-1,y,z-1,3) + LIGHT(x-1,y+1,z,3)) / 75.0f;
		float ls3 = (LIGHT(x-1,y-1,z+1,3) + ls*30 + LIGHT(x-1,y,z+1,3) + LIGHT(x-1,y-1,z,3)) / 75.0f;

		VERTEX(index, x-0.5f, y-0.5f, z-0.5f, u1,v1, lr0,lg0,lb0,ls0);
		VERTEX(index, x-0.5f, y+0.5f, z+0.5f, u2,v2, lr1,lg1,lb1,ls1);
		VERTEX(index, x-0.5f, y+0.5f, z-0.5f, u1,v2, lr2,lg2,lb2,ls2);

		VERTEX(index, x-0.5f, y-0.5f, z-0.5f, u1,v1, lr0,lg0,lb0,ls0);
		VERTEX(index, x-0.5f, y-0.5f, z+0.5f, u2,v1, lr3,lg3,lb3,ls3);
		VERTEX(index, x-0.5f, y+0.5f, z+0.5f, u2,v2, lr1,lg1,lb1,ls1);
	}

	if (!IS_BLOCKED(x,y,z+1,group)){
		l = 0.9f;

		SETUP_UV(block->textureFaces[5]);

		const float lr = LIGHT(x,y,z+1, 0) / 15.0f;
		const float lg = LIGHT(x,y,z+1, 1) / 15.0f;
		const float lb = LIGHT(x,y,z+1, 2) / 15.0f;
		const float ls = LIGHT(x,y,z+1, 3) / 15.0f;

		float lr0 = l*(LIGHT(x-1,y-1,z+1,0) + lr*30 + LIGHT(x,y-1,z+1,0) + LIGHT(x-1,y,z+1,0)) / 75.0f;
		float lr1 = l*(LIGHT(x+1,y+1,z+1,0) + lr*30 + LIGHT(x,y+1,z+1,0) + LIGHT(x+1,y,z+1,0)) / 75.0f;
		float lr2 = l*(LIGHT(x-1,y+1,z+1,0) + lr*30 + LIGHT(x,y+1,z+1,0) + LIGHT(x-1,y,z+1,0)) / 75.0f;
		float lr3 = l*(LIGHT(x+1,y-1,z+1,0) + lr*30 + LIGHT(x,y-1,z+1,0) + LIGHT(x+1,y,z+1,0)) / 75.0f;

		float lg0 = l*(LIGHT(x-1,y-1,z+1,1) + lg*30 + LIGHT(x,y-1,z+1,1) + LIGHT(x-1,y,z+1,1)) / 75.0f;
		float lg1 = l*(LIGHT(x+1,y+1,z+1,1) + lg*30 + LIGHT(x,y+1,z+1,1) + LIGHT(x+1,y,z+1,1)) / 75.0f;
		float lg2 = l*(LIGHT(x-1,y+1,z+1,1) + lg*30 + LIGHT(x,y+1,z+1,1) + LIGHT(x-1,y,z+1,1)) / 75.0f;
		float lg3 = l*(LIGHT(x+1,y-1,z+1,1) + lg*30 + LIGHT(x,y-1,z+1,1) + LIGHT(x+1,y,z+1,1)) / 75.0f;

		float lb0 = l*(LIGHT(x-1,y-1,z+1,2) + lb*30 + LIGHT(x,y-1,z+1,2) + LIGHT(x-1,y,z+1,2)) / 75.0f;
		float lb1 = l*(LIGHT(x+1,y+1,z+1,2) + lb*30 + LIGHT(x,y+1,z+1,2) + LIGHT(x+1,y,z+1,2)) / 75.0f;
		float lb2 = l*(LIGHT(x-1,y+1,z+1,2) + lb*30 + LIGHT(x,y+1,z+1,2) + LIGHT(x-1,y,z+1,2)) / 75.0f;
		float lb3 = l*(LIGHT(x+1,y-1,z+1,2) + lb*30 + LIGHT(x,y-1,z+1,2) + LIGHT(x+1,y,z+1,2)) / 75.0f;

		float ls0 = l*(LIGHT(x-1,y-1,z+1,3) + ls*30 + LIGHT(x,y-1,z+1,3) + LIGHT(x-1,y,z+1,3)) / 75.0f;
		float ls1 = l*(LIGHT(x+1,y+1,z+1,3) + ls*30 + LIGHT(x,y+1,z+1,3) + LIGHT(x+1,y,z+1,3)) / 75.0f;
		float ls2 = l*(LIGHT(x-1,y+1,z+1,3) + ls*30 + LIGHT(x,y+1,z+1,3) + LIGHT(x-1,y,z+1,3)) / 75.0f;
		float ls3 = l*(LIGHT(x+1,y-1,z+1,3) + ls*30 + LIGHT(x,y-1,z+1,3) + LIGHT(x+1,y,z+1,3)) / 75.0f;

		VERTEX(index, x-0.5f, y-0.5f, z+0.5f, u1,v1, lr0,lg0,lb0,ls0);
		VERTEX(index, x+0.5f, y+0.5f, z+0.5f, u2,v2, lr1,lg1,lb1,ls1);
		VERTEX(index, x-0.5f, y+0.5f, z+0.5f, u1,v2, lr2,lg2,lb2,ls2);

		VERTEX(index, x-0.5f, y-0.5f, z+0.5f, u1,v1, lr0,lg0,lb0,ls0);
		VERTEX(index, x+0.5f, y-0.5f, z+0.5f, u2,v1, lr3,lg3,lb3,ls3);
		VERTEX(index, x+0.5f, y+0.5f, z+0.5f, u2,v2, lr1,lg1,lb1,ls1);
	}
	if (!IS_BLOCKED(x,y,z-1,group)){
		l = 0.8f;

		SETUP_UV(block->textureFaces[4]);

		const float lr = LIGHT(x,y,z-1, 0) / 15.0f;
		const float lg = LIGHT(x,y,z-1, 1) / 15.0f;
		const float lb = LIGHT(x,y,z-1, 2) / 15.0f;
		const float ls = LIGHT(x,y,z-1, 3) / 15.0f;

		float lr0 = l*(LIGHT(x-1,y-1,z-1,0) + lr*30 + LIGHT(x,y-1,z-1,0) + LIGHT(x-1,y,z-1,0)) / 75.0f;
		float lr1 = l*(LIGHT(x-1,y+1,z-1,0) + lr*30 + LIGHT(x,y+1,z-1,0) + LIGHT(x-1,y,z-1,0)) / 75.0f;
		float lr2 = l*(LIGHT(x+1,y+1,z-1,0) + lr*30 + LIGHT(x,y+1,z-1,0) + LIGHT(x+1,y,z-1,0)) / 75.0f;
		float lr3 = l*(LIGHT(x+1,y-1,z-1,0) + lr*30 + LIGHT(x,y-1,z-1,0) + LIGHT(x+1,y,z-1,0)) / 75.0f;

		float lg0 = l*(LIGHT(x-1,y-1,z-1,1) + lg*30 + LIGHT(x,y-1,z-1,1) + LIGHT(x-1,y,z-1,1)) / 75.0f;
		float lg1 = l*(LIGHT(x-1,y+1,z-1,1) + lg*30 + LIGHT(x,y+1,z-1,1) + LIGHT(x-1,y,z-1,1)) / 75.0f;
		float lg2 = l*(LIGHT(x+1,y+1,z-1,1) + lg*30 + LIGHT(x,y+1,z-1,1) + LIGHT(x+1,y,z-1,1)) / 75.0f;
		float lg3 = l*(LIGHT(x+1,y-1,z-1,1) + lg*30 + LIGHT(x,y-1,z-1,1) + LIGHT(x+1,y,z-1,1)) / 75.0f;

		float lb0 = l*(LIGHT(x-1,y-1,z-1,2) + lb*30 + LIGHT(x,y-1,z-1,2) + LIGHT(x-1,y,z-1,2)) / 75.0f;
		float lb1 = l*(LIGHT(x-1,y+1,z-1,2) + lb*30 + LIGHT(x,y+1,z-1,2) + LIGHT(x-1,y,z-1,2)) / 75.0f;
		float lb2 = l*(LIGHT(x+1,y+1,z-1,2) + lb*30 + LIGHT(x,y+1,z-1,2) + LIGHT(x+1,y,z-1,2)) / 75.0f;
		float lb3 = l*(LIGHT(x+1,y-1,z-1,2) + lb*30 + LIGHT(x,y-1,z-1,2) + LIGHT(x+1,y,z-1,2)) / 75.0f;

		float ls0 = l*(LIGHT(x-1,y-1,z-1,3) + ls*30 + LIGHT(x,y-1,z-1,3) + LIGHT(x-1,y,z-1,3)) / 75.0f;
		float ls1 = l*(LIGHT(x-1,y+1,z-1,3) + ls*30 + LIGHT(x,y+1,z-1,3) + LIGHT(x-1,y,z-1,3)) / 75.0f;
		float ls2 = l*(LIGHT(x+1,y+1,z-1,3) + ls*30 + LIGHT(x,y+1,z-1,3) + LIGHT(x+1,y,z-1,3)) / 75.0f;
		float ls3 = l*(LIGHT(x+1,y-1,z-1,3) + ls*30 + LIGHT(x,y-1,z-1,3) + LIGHT(x+1,y,z-1,3)) / 75.0f;

		VERTEX(index, x-0.5f, y-0.5f, z-0.5f, u2,v1, lr0,lg0,lb0,ls0);
		VERTEX(index, x-0.5f, y+0.5f, z-0.5f, u2,v2, lr1,lg1,lb1,ls1);
		VERTEX(index, x+0.5f, y+0.5f, z-0.5f, u1,v2, lr2,lg2,lb2,ls2);

		VERTEX(index, x-0.5f, y-0.5f, z-0.5f, u2,v1, lr0,lg0,lb0,ls0);
		VERTEX(index, x+0.5f, y+0.5f, z-0.5f, u1,v2, lr2,lg2,lb2,ls2);
		VERTEX(index, x+0.5f, y-0.5f, z-0.5f, u1,v1, lr3,lg3,lb3,ls3);
	}
}

inline void _renderBlockShadeless(std::vector<float>& buffer, int x, int y, int z, const Chunk** chunks, unsigned int id, size_t& index){
	float l;
	float uvsize = 1.0f/16.0f;

	Block* block = Block::blocks[id];
	unsigned char group = block->drawGroup;

	if (!IS_BLOCKED(x,y+1,z,group)){
		SETUP_UV(block->textureFaces[3]);

		VERTEX(index, x-0.5f, y+0.5f, z-0.5f, u2,v1, 1,1,1,0);
		VERTEX(index, x-0.5f, y+0.5f, z+0.5f, u2,v2, 1,1,1,0);
		VERTEX(index, x+0.5f, y+0.5f, z+0.5f, u1,v2, 1,1,1,0);

		VERTEX(index, x-0.5f, y+0.5f, z-0.5f, u2,v1, 1,1,1,0);
		VERTEX(index, x+0.5f, y+0.5f, z+0.5f, u1,v2, 1,1,1,0);
		VERTEX(index, x+0.5f, y+0.5f, z-0.5f, u1,v1, 1,1,1,0);
	}
	if (!IS_BLOCKED(x,y-1,z,group)){
		SETUP_UV(block->textureFaces[2]);

		VERTEX(index, x-0.5f, y-0.5f, z-0.5f, u1,v1, 1,1,1,0);
		VERTEX(index, x+0.5f, y-0.5f, z+0.5f, u2,v2, 1,1,1,0);
		VERTEX(index, x-0.5f, y-0.5f, z+0.5f, u1,v2, 1,1,1,0);

		VERTEX(index, x-0.5f, y-0.5f, z-0.5f, u1,v1, 1,1,1,0);
		VERTEX(index, x+0.5f, y-0.5f, z-0.5f, u2,v1, 1,1,1,0);
		VERTEX(index, x+0.5f, y-0.5f, z+0.5f, u2,v2, 1,1,1,0);
	}

	if (!IS_BLOCKED(x+1,y,z,group)){
		SETUP_UV(block->textureFaces[1]);

		VERTEX(index, x+0.5f, y-0.5f, z-0.5f, u2,v1, 1,1,1,0);
		VERTEX(index, x+0.5f, y+0.5f, z-0.5f, u2,v2, 1,1,1,0);
		VERTEX(index, x+0.5f, y+0.5f, z+0.5f, u1,v2, 1,1,1,0);

		VERTEX(index, x+0.5f, y-0.5f, z-0.5f, u2,v1, 1,1,1,0);
		VERTEX(index, x+0.5f, y+0.5f, z+0.5f, u1,v2, 1,1,1,0);
		VERTEX(index, x+0.5f, y-0.5f, z+0.5f, u1,v1, 1,1,1,0);
	}
	if (!IS_BLOCKED(x-1,y,z,group)){
		SETUP_UV(block->textureFaces[0]);

		VERTEX(index, x-0.5f, y-0.5f, z-0.5f, u1,v1, 1,1,1,0);
		VERTEX(index, x-0.5f, y+0.5f, z+0.5f, u2,v2, 1,1,1,0);
		VERTEX(index, x-0.5f, y+0.5f, z-0.5f, u1,v2, 1,1,1,0);

		VERTEX(index, x-0.5f, y-0.5f, z-0.5f, u1,v1, 1,1,1,0);
		VERTEX(index, x-0.5f, y-0.5f, z+0.5f, u2,v1, 1,1,1,0);
		VERTEX(index, x-0.5f, y+0.5f, z+0.5f, u2,v2, 1,1,1,0);
	}

	if (!IS_BLOCKED(x,y,z+1,group)){
		SETUP_UV(block->textureFaces[5]);

		VERTEX(index, x-0.5f, y-0.5f, z+0.5f, u1,v1, 1,1,1,0);
		VERTEX(index, x+0.5f, y+0.5f, z+0.5f, u2,v2, 1,1,1,0);
		VERTEX(index, x-0.5f, y+0.5f, z+0.5f, u1,v2, 1,1,1,0);

		VERTEX(index, x-0.5f, y-0.5f, z+0.5f, u1,v1, 1,1,1,0);
		VERTEX(index, x+0.5f, y-0.5f, z+0.5f, u2,v1, 1,1,1,0);
		VERTEX(index, x+0.5f, y+0.5f, z+0.5f, u2,v2, 1,1,1,0);
	}
	if (!IS_BLOCKED(x,y,z-1,group)){
		SETUP_UV(block->textureFaces[4]);

		VERTEX(index, x-0.5f, y-0.5f, z-0.5f, u2,v1, 1,1,1,0);
		VERTEX(index, x-0.5f, y+0.5f, z-0.5f, u2,v2, 1,1,1,0);
		VERTEX(index, x+0.5f, y+0.5f, z-0.5f, u1,v2, 1,1,1,0);

		VERTEX(index, x-0.5f, y-0.5f, z-0.5f, u2,v1, 1,1,1,0);
		VERTEX(index, x+0.5f, y+0.5f, z-0.5f, u1,v2, 1,1,1,0);
		VERTEX(index, x+0.5f, y-0.5f, z-0.5f, u1,v1, 1,1,1,0);
	}
}


inline void _renderXBlock(std::vector<float>& buffer, int x, int y, int z, const Chunk** chunks, voxel vox, size_t& index){
	Block* block = Block::blocks[vox.id];

	int rand = ((x * z + y) xor (z * y - x)) * (z + y);

	float xs = (float)(char)rand / 512;
	float zs = (float)(char)(rand >> 8) / 512;

	if (block->model != 2){
		return;
	}

	float uvsize = 1.0f/16.0f;

	float lr = LIGHT(x,y,z, 0);
	float lg = LIGHT(x,y,z, 1);
	float lb = LIGHT(x,y,z, 2);
	float ls = LIGHT(x,y,z, 3);

	float lr0 = (LIGHT(x,y-1,z,0) + lr*3) / 60.0f;
	float lr1 = (LIGHT(x,y+1,z,0) + lr*3) / 60.0f;

	float lg0 = (LIGHT(x,y-1,z,1) + lg*3) / 60.0f;
	float lg1 = (LIGHT(x,y+1,z,1) + lg*3) / 60.0f;

	float lb0 = (LIGHT(x,y-1,z,2) + lb*3) / 60.0f;
	float lb1 = (LIGHT(x,y+1,z,2) + lb*3) / 60.0f;

	float ls0 = (LIGHT(x,y-1,z,3) + ls*3) / 60.0f;
	float ls1 = (LIGHT(x,y+1,z,3) + ls*3) / 60.0f;

	{SETUP_UV(block->textureFaces[1]);

	VERTEX(index, x-0.3535f+xs, y-0.5f, z-0.3535f+zs, u2,v1, lr0,lg0,lb0,ls0);
	VERTEX(index, x-0.3535f+xs, y+0.5f, z-0.3535f+zs, u2,v2, lr1,lg1,lb1,ls1);
	VERTEX(index, x+0.3535f+xs, y+0.5f, z+0.3535f+zs, u1,v2, lr1,lg1,lb1,ls1);

	VERTEX(index, x-0.3535f+xs, y-0.5f, z-0.3535f+zs, u2,v1, lr0,lg0,lb0,ls0);
	VERTEX(index, x+0.3535f+xs, y+0.5f, z+0.3535f+zs, u1,v2, lr1,lg1,lb1,ls1);
	VERTEX(index, x+0.3535f+xs, y-0.5f, z+0.3535f+zs, u1,v1, lr0,lg0,lb0,ls0);}

	{SETUP_UV(block->textureFaces[0]);

	VERTEX(index, x-0.3535f+xs, y-0.5f, z-0.3535f+zs, u1,v1, lr0,lg0,lb0,ls0);
	VERTEX(index, x+0.3535f+xs, y+0.5f, z+0.3535f+zs, u2,v2, lr1,lg1,lb1,ls1);
	VERTEX(index, x-0.3535f+xs, y+0.5f, z-0.3535f+zs, u1,v2, lr1,lg1,lb1,ls1);

	VERTEX(index, x-0.3535f+xs, y-0.5f, z-0.3535f+zs, u1,v1, lr0,lg0,lb0,ls0);
	VERTEX(index, x+0.3535f+xs, y-0.5f, z+0.3535f+zs, u2,v1, lr0,lg0,lb0,ls0);
	VERTEX(index, x+0.3535f+xs, y+0.5f, z+0.3535f+zs, u2,v2, lr1,lg1,lb1,ls1);}

	{SETUP_UV(block->textureFaces[5]);

	VERTEX(index, x-0.3535f+xs, y-0.5f, z+0.3535f+zs, u1,v1, lr0,lg0,lb0,ls0);
	VERTEX(index, x+0.3535f+xs, y+0.5f, z-0.3535f+zs, u2,v2, lr1,lg1,lb1,ls1);
	VERTEX(index, x-0.3535f+xs, y+0.5f, z+0.3535f+zs, u1,v2, lr1,lg1,lb1,ls1);

	VERTEX(index, x-0.3535f+xs, y-0.5f, z+0.3535f+zs, u1,v1, lr0,lg0,lb0,ls0);
	VERTEX(index, x+0.3535f+xs, y-0.5f, z-0.3535f+zs, u2,v1, lr0,lg0,lb0,ls0);
	VERTEX(index, x+0.3535f+xs, y+0.5f, z-0.3535f+zs, u2,v2, lr1,lg1,lb1,ls1);}

	{SETUP_UV(block->textureFaces[4]);

	VERTEX(index, x-0.3535f+xs, y-0.5f, z+0.3535f+zs, u2,v1, lr0,lg0,lb0,ls0);
	VERTEX(index, x-0.3535f+xs, y+0.5f, z+0.3535f+zs, u2,v2, lr1,lg1,lb1,ls1);
	VERTEX(index, x+0.3535f+xs, y+0.5f, z-0.3535f+zs, u1,v2, lr1,lg1,lb1,ls1);

	VERTEX(index, x-0.3535f+xs, y-0.5f, z+0.3535f+zs, u2,v1, lr0,lg0,lb0,ls0);
	VERTEX(index, x+0.3535f+xs, y+0.5f, z-0.3535f+zs, u1,v2, lr1,lg1,lb1,ls1);
	VERTEX(index, x+0.3535f+xs, y-0.5f, z-0.3535f+zs, u1,v1, lr0,lg0,lb0,ls0);}
}

const float* VoxelRenderer::render(Chunk* chunk, const Chunk** chunks, size_t& size){
	buffer.clear();
	size_t index = 0;
	for (int y = 0; y < CHUNK_H; y++){
		for (int z = 0; z < CHUNK_D; z++){
			for (int x = 0; x < CHUNK_W; x++){
				voxel vox = chunk->voxels[(y * CHUNK_D + z) * CHUNK_W + x];
				if (vox.id == 0)
					continue;
				if (vox.id == 9 || vox.id == 4 || vox.id == 12 || vox.id == 13)
					continue;
				Block* block = Block::blocks[vox.id];
				if (block->emission[0] || block->emission[1] || block->emission[2]){
					continue;
				}
				_renderBlock(buffer, x, y, z, chunks, vox.id, index);
			}
		}
	}

	for (int y = 0; y < CHUNK_H; y++){
		for (int z = 0; z < CHUNK_D; z++){
			for (int x = 0; x < CHUNK_W; x++){
				voxel vox = chunk->voxels[(y * CHUNK_D + z) * CHUNK_W + x];
				if (vox.id == 0)
					continue;
				Block* block = Block::blocks[vox.id];
				if (block->emission[0] || block->emission[1] || block->emission[2]){
					_renderBlockShadeless(buffer, x, y, z, chunks, vox.id, index);
				}
			}
		}
	}

	for (int y = 0; y < CHUNK_H; y++){
		for (int z = 0; z < CHUNK_D; z++){
			for (int x = 0; x < CHUNK_W; x++){
				voxel vox = chunk->voxels[(y * CHUNK_D + z) * CHUNK_W + x];
				if (vox.id == 0)
					continue;
				if (vox.id != 9)
					continue;
				_renderBlock(buffer, x, y, z, chunks, vox.id, index);
			}
		}
	}

	for (int y = 0; y < CHUNK_H; y++){
		for (int z = 0; z < CHUNK_D; z++){
			for (int x = 0; x < CHUNK_W; x++){
				voxel vox = chunk->voxels[(y * CHUNK_D + z) * CHUNK_W + x];
				if (vox.id == 0)
					continue;
				if (Block::blocks[vox.id]->model != BLOCK_MODEL_X_SPRITE)
					continue;
				_renderXBlock(buffer, x, y, z, chunks, vox, index);
			}
		}
	}

	for (int y = 0; y < CHUNK_H; y++){
		for (int z = 0; z < CHUNK_D; z++){
			for (int x = 0; x < CHUNK_W; x++){
				voxel vox = chunk->voxels[(y * CHUNK_D + z) * CHUNK_W + x];
				if (vox.id == 0)
					continue;
				if (vox.id != 4)
					continue;
				_renderBlock(buffer, x, y, z, chunks, vox.id, index);
			}
		}
	}
	size = buffer.size();
	return &buffer[0];
}
