#include "VoxelRenderer.h"
#include "Mesh.h"
#include "../voxels/Chunk.h"
#include "../voxels/voxel.h"
#include "../voxels/Block.h"
#include "../lighting/Lightmap.h"

#define VERTEX_SIZE (3 + 2 + 4)

#define CDIV(X,A) (((X) < 0) ? ((X) / (A) - 1) : ((X) / (A)))
#define LOCAL_NEG(X, SIZE) (((X) < 0) ? ((SIZE)+(X)) : (X))
#define LOCAL(X, SIZE) ((X) >= (SIZE) ? ((X) - (SIZE)) : LOCAL_NEG(X, SIZE))
#define IS_CHUNK(X,Y,Z) (GET_CHUNK(X,Y,Z) != nullptr)
#define GET_CHUNK(X,Y,Z) (chunks[((CDIV(Y, CHUNK_H)+1) * 3 + CDIV(Z, CHUNK_D) + 1) * 3 + CDIV(X, CHUNK_W) + 1])

#define LIGHT(X,Y,Z, CHANNEL) (IS_CHUNK(X,Y,Z) ? GET_CHUNK(X,Y,Z)->lightmap->get(LOCAL(X, CHUNK_W), LOCAL(Y, CHUNK_H), LOCAL(Z, CHUNK_D), (CHANNEL)) : 0)
#define VOXEL(X,Y,Z) (GET_CHUNK(X,Y,Z)->voxels[(LOCAL(Y, CHUNK_H) * CHUNK_D + LOCAL(Z, CHUNK_D)) * CHUNK_W + LOCAL(X, CHUNK_W)])
#define IS_BLOCKED(X,Y,Z,GROUP) ((!IS_CHUNK(X, Y, Z)) || Block::blocks[VOXEL(X, Y, Z).id]->drawGroup == (GROUP))

#define VERTEX(INDEX, X,Y,Z, U,V, R,G,B,S) buffer[INDEX+0] = (X);\
								  buffer[INDEX+1] = (Y);\
								  buffer[INDEX+2] = (Z);\
								  buffer[INDEX+3] = (U);\
								  buffer[INDEX+4] = (V);\
								  buffer[INDEX+5] = (R);\
								  buffer[INDEX+6] = (G);\
								  buffer[INDEX+7] = (B);\
								  buffer[INDEX+8] = (S);\
								  INDEX += VERTEX_SIZE;


#define SETUP_UV(INDEX) float u1 = ((INDEX) % 16) * uvsize;\
				float v1 = 1-((1 + (INDEX) / 16) * uvsize);\
				float u2 = u1 + uvsize;\
				float v2 = v1 + uvsize;

int chunk_attrs[] = {3,2,4, 0};

VoxelRenderer::VoxelRenderer(size_t capacity) : capacity(capacity) {
	buffer = new float[capacity * VERTEX_SIZE * 6];
}

VoxelRenderer::~VoxelRenderer(){
	delete[] buffer;
}

Mesh* VoxelRenderer::render(Chunk* chunk, const Chunk** chunks){
	size_t index = 0;
	for (int y = 0; y < CHUNK_H; y++){
		for (int z = 0; z < CHUNK_D; z++){
			for (int x = 0; x < CHUNK_W; x++){
				voxel vox = chunk->voxels[(y * CHUNK_D + z) * CHUNK_W + x];
				unsigned int id = vox.id;

				if (!id){
					continue;
				}

				float l;
				float uvsize = 1.0f/16.0f;

				Block* block = Block::blocks[id];
				unsigned char group = block->drawGroup;

				if (!IS_BLOCKED(x,y+1,z,group)){
					l = 1.0f;

					SETUP_UV(block->textureFaces[3]);

					float lr = LIGHT(x,y+1,z, 0) / 15.0f;
					float lg = LIGHT(x,y+1,z, 1) / 15.0f;
					float lb = LIGHT(x,y+1,z, 2) / 15.0f;
					float ls = LIGHT(x,y+1,z, 3) / 15.0f;

					float lr0 = (LIGHT(x-1,y+1,z,0) + lr*30 + LIGHT(x-1,y+1,z-1,0) + LIGHT(x,y+1,z-1,0)) / 5.0f / 15.0f;
					float lr1 = (LIGHT(x-1,y+1,z,0) + lr*30 + LIGHT(x-1,y+1,z+1,0) + LIGHT(x,y+1,z+1,0)) / 5.0f / 15.0f;
					float lr2 = (LIGHT(x+1,y+1,z,0) + lr*30 + LIGHT(x+1,y+1,z+1,0) + LIGHT(x,y+1,z+1,0)) / 5.0f / 15.0f;
					float lr3 = (LIGHT(x+1,y+1,z,0) + lr*30 + LIGHT(x+1,y+1,z-1,0) + LIGHT(x,y+1,z-1,0)) / 5.0f / 15.0f;

					float lg0 = (LIGHT(x-1,y+1,z,1) + lg*30 + LIGHT(x-1,y+1,z-1,1) + LIGHT(x,y+1,z-1,1)) / 5.0f / 15.0f;
					float lg1 = (LIGHT(x-1,y+1,z,1) + lg*30 + LIGHT(x-1,y+1,z+1,1) + LIGHT(x,y+1,z+1,1)) / 5.0f / 15.0f;
					float lg2 = (LIGHT(x+1,y+1,z,1) + lg*30 + LIGHT(x+1,y+1,z+1,1) + LIGHT(x,y+1,z+1,1)) / 5.0f / 15.0f;
					float lg3 = (LIGHT(x+1,y+1,z,1) + lg*30 + LIGHT(x+1,y+1,z-1,1) + LIGHT(x,y+1,z-1,1)) / 5.0f / 15.0f;

					float lb0 = (LIGHT(x-1,y+1,z,2) + lb*30 + LIGHT(x-1,y+1,z-1,2) + LIGHT(x,y+1,z-1,2)) / 5.0f / 15.0f;
					float lb1 = (LIGHT(x-1,y+1,z,2) + lb*30 + LIGHT(x-1,y+1,z+1,2) + LIGHT(x,y+1,z+1,2)) / 5.0f / 15.0f;
					float lb2 = (LIGHT(x+1,y+1,z,2) + lb*30 + LIGHT(x+1,y+1,z+1,2) + LIGHT(x,y+1,z+1,2)) / 5.0f / 15.0f;
					float lb3 = (LIGHT(x+1,y+1,z,2) + lb*30 + LIGHT(x+1,y+1,z-1,2) + LIGHT(x,y+1,z-1,2)) / 5.0f / 15.0f;

					float ls0 = (LIGHT(x-1,y+1,z,3) + ls*30 + LIGHT(x-1,y+1,z-1,3) + LIGHT(x,y+1,z-1,3)) / 5.0f / 15.0f;
					float ls1 = (LIGHT(x-1,y+1,z,3) + ls*30 + LIGHT(x-1,y+1,z+1,3) + LIGHT(x,y+1,z+1,3)) / 5.0f / 15.0f;
					float ls2 = (LIGHT(x+1,y+1,z,3) + ls*30 + LIGHT(x+1,y+1,z+1,3) + LIGHT(x,y+1,z+1,3)) / 5.0f / 15.0f;
					float ls3 = (LIGHT(x+1,y+1,z,3) + ls*30 + LIGHT(x+1,y+1,z-1,3) + LIGHT(x,y+1,z-1,3)) / 5.0f / 15.0f;

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

					float lr = LIGHT(x,y-1,z, 0) / 15.0f;
					float lg = LIGHT(x,y-1,z, 1) / 15.0f;
					float lb = LIGHT(x,y-1,z, 2) / 15.0f;
					float ls = LIGHT(x,y-1,z, 3) / 15.0f;

					float lr0 = (LIGHT(x-1,y-1,z-1,0) + lr*30 + LIGHT(x-1,y-1,z,0) + LIGHT(x,y-1,z-1,0)) / 5.0f / 15.0f;
					float lr1 = (LIGHT(x+1,y-1,z+1,0) + lr*30 + LIGHT(x+1,y-1,z,0) + LIGHT(x,y-1,z+1,0)) / 5.0f / 15.0f;
					float lr2 = (LIGHT(x-1,y-1,z+1,0) + lr*30 + LIGHT(x-1,y-1,z,0) + LIGHT(x,y-1,z+1,0)) / 5.0f / 15.0f;
					float lr3 = (LIGHT(x+1,y-1,z-1,0) + lr*30 + LIGHT(x+1,y-1,z,0) + LIGHT(x,y-1,z-1,0)) / 5.0f / 15.0f;

					float lg0 = (LIGHT(x-1,y-1,z-1,1) + lg*30 + LIGHT(x-1,y-1,z,1) + LIGHT(x,y-1,z-1,1)) / 5.0f / 15.0f;
					float lg1 = (LIGHT(x+1,y-1,z+1,1) + lg*30 + LIGHT(x+1,y-1,z,1) + LIGHT(x,y-1,z+1,1)) / 5.0f / 15.0f;
					float lg2 = (LIGHT(x-1,y-1,z+1,1) + lg*30 + LIGHT(x-1,y-1,z,1) + LIGHT(x,y-1,z+1,1)) / 5.0f / 15.0f;
					float lg3 = (LIGHT(x+1,y-1,z-1,1) + lg*30 + LIGHT(x+1,y-1,z,1) + LIGHT(x,y-1,z-1,1)) / 5.0f / 15.0f;

					float lb0 = (LIGHT(x-1,y-1,z-1,2) + lb*30 + LIGHT(x-1,y-1,z,2) + LIGHT(x,y-1,z-1,2)) / 5.0f / 15.0f;
					float lb1 = (LIGHT(x+1,y-1,z+1,2) + lb*30 + LIGHT(x+1,y-1,z,2) + LIGHT(x,y-1,z+1,2)) / 5.0f / 15.0f;
					float lb2 = (LIGHT(x-1,y-1,z+1,2) + lb*30 + LIGHT(x-1,y-1,z,2) + LIGHT(x,y-1,z+1,2)) / 5.0f / 15.0f;
					float lb3 = (LIGHT(x+1,y-1,z-1,2) + lb*30 + LIGHT(x+1,y-1,z,2) + LIGHT(x,y-1,z-1,2)) / 5.0f / 15.0f;

					float ls0 = (LIGHT(x-1,y-1,z-1,3) + ls*30 + LIGHT(x-1,y-1,z,3) + LIGHT(x,y-1,z-1,3)) / 5.0f / 15.0f;
					float ls1 = (LIGHT(x+1,y-1,z+1,3) + ls*30 + LIGHT(x+1,y-1,z,3) + LIGHT(x,y-1,z+1,3)) / 5.0f / 15.0f;
					float ls2 = (LIGHT(x-1,y-1,z+1,3) + ls*30 + LIGHT(x-1,y-1,z,3) + LIGHT(x,y-1,z+1,3)) / 5.0f / 15.0f;
					float ls3 = (LIGHT(x+1,y-1,z-1,3) + ls*30 + LIGHT(x+1,y-1,z,3) + LIGHT(x,y-1,z-1,3)) / 5.0f / 15.0f;

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

					float lr = LIGHT(x+1,y,z, 0) / 15.0f;
					float lg = LIGHT(x+1,y,z, 1) / 15.0f;
					float lb = LIGHT(x+1,y,z, 2) / 15.0f;
					float ls = LIGHT(x+1,y,z, 3) / 15.0f;

					float lr0 = (LIGHT(x+1,y-1,z-1,0) + lr*30 + LIGHT(x+1,y,z-1,0) + LIGHT(x+1,y-1,z,0)) / 5.0f / 15.0f;
					float lr1 = (LIGHT(x+1,y+1,z-1,0) + lr*30 + LIGHT(x+1,y,z-1,0) + LIGHT(x+1,y+1,z,0)) / 5.0f / 15.0f;
					float lr2 = (LIGHT(x+1,y+1,z+1,0) + lr*30 + LIGHT(x+1,y,z+1,0) + LIGHT(x+1,y+1,z,0)) / 5.0f / 15.0f;
					float lr3 = (LIGHT(x+1,y-1,z+1,0) + lr*30 + LIGHT(x+1,y,z+1,0) + LIGHT(x+1,y-1,z,0)) / 5.0f / 15.0f;

					float lg0 = (LIGHT(x+1,y-1,z-1,1) + lg*30 + LIGHT(x+1,y,z-1,1) + LIGHT(x+1,y-1,z,1)) / 5.0f / 15.0f;
					float lg1 = (LIGHT(x+1,y+1,z-1,1) + lg*30 + LIGHT(x+1,y,z-1,1) + LIGHT(x+1,y+1,z,1)) / 5.0f / 15.0f;
					float lg2 = (LIGHT(x+1,y+1,z+1,1) + lg*30 + LIGHT(x+1,y,z+1,1) + LIGHT(x+1,y+1,z,1)) / 5.0f / 15.0f;
					float lg3 = (LIGHT(x+1,y-1,z+1,1) + lg*30 + LIGHT(x+1,y,z+1,1) + LIGHT(x+1,y-1,z,1)) / 5.0f / 15.0f;

					float lb0 = (LIGHT(x+1,y-1,z-1,2) + lb*30 + LIGHT(x+1,y,z-1,2) + LIGHT(x+1,y-1,z,2)) / 5.0f / 15.0f;
					float lb1 = (LIGHT(x+1,y+1,z-1,2) + lb*30 + LIGHT(x+1,y,z-1,2) + LIGHT(x+1,y+1,z,2)) / 5.0f / 15.0f;
					float lb2 = (LIGHT(x+1,y+1,z+1,2) + lb*30 + LIGHT(x+1,y,z+1,2) + LIGHT(x+1,y+1,z,2)) / 5.0f / 15.0f;
					float lb3 = (LIGHT(x+1,y-1,z+1,2) + lb*30 + LIGHT(x+1,y,z+1,2) + LIGHT(x+1,y-1,z,2)) / 5.0f / 15.0f;

					float ls0 = (LIGHT(x+1,y-1,z-1,3) + ls*30 + LIGHT(x+1,y,z-1,3) + LIGHT(x+1,y-1,z,3)) / 5.0f / 15.0f;
					float ls1 = (LIGHT(x+1,y+1,z-1,3) + ls*30 + LIGHT(x+1,y,z-1,3) + LIGHT(x+1,y+1,z,3)) / 5.0f / 15.0f;
					float ls2 = (LIGHT(x+1,y+1,z+1,3) + ls*30 + LIGHT(x+1,y,z+1,3) + LIGHT(x+1,y+1,z,3)) / 5.0f / 15.0f;
					float ls3 = (LIGHT(x+1,y-1,z+1,3) + ls*30 + LIGHT(x+1,y,z+1,3) + LIGHT(x+1,y-1,z,3)) / 5.0f / 15.0f;

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

					float lr = LIGHT(x-1,y,z, 0) / 15.0f;
					float lg = LIGHT(x-1,y,z, 1) / 15.0f;
					float lb = LIGHT(x-1,y,z, 2) / 15.0f;
					float ls = LIGHT(x-1,y,z, 3) / 15.0f;

					float lr0 = (LIGHT(x-1,y-1,z-1,0) + lr*30 + LIGHT(x-1,y,z-1,0) + LIGHT(x-1,y-1,z,0)) / 5.0f / 15.0f;
					float lr1 = (LIGHT(x-1,y+1,z+1,0) + lr*30 + LIGHT(x-1,y,z+1,0) + LIGHT(x-1,y+1,z,0)) / 5.0f / 15.0f;
					float lr2 = (LIGHT(x-1,y+1,z-1,0) + lr*30 + LIGHT(x-1,y,z-1,0) + LIGHT(x-1,y+1,z,0)) / 5.0f / 15.0f;
					float lr3 = (LIGHT(x-1,y-1,z+1,0) + lr*30 + LIGHT(x-1,y,z+1,0) + LIGHT(x-1,y-1,z,0)) / 5.0f / 15.0f;

					float lg0 = (LIGHT(x-1,y-1,z-1,1) + lg*30 + LIGHT(x-1,y,z-1,1) + LIGHT(x-1,y-1,z,1)) / 5.0f / 15.0f;
					float lg1 = (LIGHT(x-1,y+1,z+1,1) + lg*30 + LIGHT(x-1,y,z+1,1) + LIGHT(x-1,y+1,z,1)) / 5.0f / 15.0f;
					float lg2 = (LIGHT(x-1,y+1,z-1,1) + lg*30 + LIGHT(x-1,y,z-1,1) + LIGHT(x-1,y+1,z,1)) / 5.0f / 15.0f;
					float lg3 = (LIGHT(x-1,y-1,z+1,1) + lg*30 + LIGHT(x-1,y,z+1,1) + LIGHT(x-1,y-1,z,1)) / 5.0f / 15.0f;

					float lb0 = (LIGHT(x-1,y-1,z-1,2) + lb*30 + LIGHT(x-1,y,z-1,2) + LIGHT(x-1,y-1,z,2)) / 5.0f / 15.0f;
					float lb1 = (LIGHT(x-1,y+1,z+1,2) + lb*30 + LIGHT(x-1,y,z+1,2) + LIGHT(x-1,y+1,z,2)) / 5.0f / 15.0f;
					float lb2 = (LIGHT(x-1,y+1,z-1,2) + lb*30 + LIGHT(x-1,y,z-1,2) + LIGHT(x-1,y+1,z,2)) / 5.0f / 15.0f;
					float lb3 = (LIGHT(x-1,y-1,z+1,2) + lb*30 + LIGHT(x-1,y,z+1,2) + LIGHT(x-1,y-1,z,2)) / 5.0f / 15.0f;

					float ls0 = (LIGHT(x-1,y-1,z-1,3) + ls*30 + LIGHT(x-1,y,z-1,3) + LIGHT(x-1,y-1,z,3)) / 5.0f / 15.0f;
					float ls1 = (LIGHT(x-1,y+1,z+1,3) + ls*30 + LIGHT(x-1,y,z+1,3) + LIGHT(x-1,y+1,z,3)) / 5.0f / 15.0f;
					float ls2 = (LIGHT(x-1,y+1,z-1,3) + ls*30 + LIGHT(x-1,y,z-1,3) + LIGHT(x-1,y+1,z,3)) / 5.0f / 15.0f;
					float ls3 = (LIGHT(x-1,y-1,z+1,3) + ls*30 + LIGHT(x-1,y,z+1,3) + LIGHT(x-1,y-1,z,3)) / 5.0f / 15.0f;

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

					float lr = LIGHT(x,y,z+1, 0) / 15.0f;
					float lg = LIGHT(x,y,z+1, 1) / 15.0f;
					float lb = LIGHT(x,y,z+1, 2) / 15.0f;
					float ls = LIGHT(x,y,z+1, 3) / 15.0f;

					float lr0 = l*(LIGHT(x-1,y-1,z+1,0) + lr*30 + LIGHT(x,y-1,z+1,0) + LIGHT(x-1,y,z+1,0)) / 5.0f / 15.0f;
					float lr1 = l*(LIGHT(x+1,y+1,z+1,0) + lr*30 + LIGHT(x,y+1,z+1,0) + LIGHT(x+1,y,z+1,0)) / 5.0f / 15.0f;
					float lr2 = l*(LIGHT(x-1,y+1,z+1,0) + lr*30 + LIGHT(x,y+1,z+1,0) + LIGHT(x-1,y,z+1,0)) / 5.0f / 15.0f;
					float lr3 = l*(LIGHT(x+1,y-1,z+1,0) + lr*30 + LIGHT(x,y-1,z+1,0) + LIGHT(x+1,y,z+1,0)) / 5.0f / 15.0f;

					float lg0 = l*(LIGHT(x-1,y-1,z+1,1) + lg*30 + LIGHT(x,y-1,z+1,1) + LIGHT(x-1,y,z+1,1)) / 5.0f / 15.0f;
					float lg1 = l*(LIGHT(x+1,y+1,z+1,1) + lg*30 + LIGHT(x,y+1,z+1,1) + LIGHT(x+1,y,z+1,1)) / 5.0f / 15.0f;
					float lg2 = l*(LIGHT(x-1,y+1,z+1,1) + lg*30 + LIGHT(x,y+1,z+1,1) + LIGHT(x-1,y,z+1,1)) / 5.0f / 15.0f;
					float lg3 = l*(LIGHT(x+1,y-1,z+1,1) + lg*30 + LIGHT(x,y-1,z+1,1) + LIGHT(x+1,y,z+1,1)) / 5.0f / 15.0f;

					float lb0 = l*(LIGHT(x-1,y-1,z+1,2) + lb*30 + LIGHT(x,y-1,z+1,2) + LIGHT(x-1,y,z+1,2)) / 5.0f / 15.0f;
					float lb1 = l*(LIGHT(x+1,y+1,z+1,2) + lb*30 + LIGHT(x,y+1,z+1,2) + LIGHT(x+1,y,z+1,2)) / 5.0f / 15.0f;
					float lb2 = l*(LIGHT(x-1,y+1,z+1,2) + lb*30 + LIGHT(x,y+1,z+1,2) + LIGHT(x-1,y,z+1,2)) / 5.0f / 15.0f;
					float lb3 = l*(LIGHT(x+1,y-1,z+1,2) + lb*30 + LIGHT(x,y-1,z+1,2) + LIGHT(x+1,y,z+1,2)) / 5.0f / 15.0f;

					float ls0 = l*(LIGHT(x-1,y-1,z+1,3) + ls*30 + LIGHT(x,y-1,z+1,3) + LIGHT(x-1,y,z+1,3)) / 5.0f / 15.0f;
					float ls1 = l*(LIGHT(x+1,y+1,z+1,3) + ls*30 + LIGHT(x,y+1,z+1,3) + LIGHT(x+1,y,z+1,3)) / 5.0f / 15.0f;
					float ls2 = l*(LIGHT(x-1,y+1,z+1,3) + ls*30 + LIGHT(x,y+1,z+1,3) + LIGHT(x-1,y,z+1,3)) / 5.0f / 15.0f;
					float ls3 = l*(LIGHT(x+1,y-1,z+1,3) + ls*30 + LIGHT(x,y-1,z+1,3) + LIGHT(x+1,y,z+1,3)) / 5.0f / 15.0f;

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

					float lr = LIGHT(x,y,z-1, 0) / 15.0f;
					float lg = LIGHT(x,y,z-1, 1) / 15.0f;
					float lb = LIGHT(x,y,z-1, 2) / 15.0f;
					float ls = LIGHT(x,y,z-1, 3) / 15.0f;

					float lr0 = l*(LIGHT(x-1,y-1,z-1,0) + lr*30 + LIGHT(x,y-1,z-1,0) + LIGHT(x-1,y,z-1,0)) / 5.0f / 15.0f;
					float lr1 = l*(LIGHT(x-1,y+1,z-1,0) + lr*30 + LIGHT(x,y+1,z-1,0) + LIGHT(x-1,y,z-1,0)) / 5.0f / 15.0f;
					float lr2 = l*(LIGHT(x+1,y+1,z-1,0) + lr*30 + LIGHT(x,y+1,z-1,0) + LIGHT(x+1,y,z-1,0)) / 5.0f / 15.0f;
					float lr3 = l*(LIGHT(x+1,y-1,z-1,0) + lr*30 + LIGHT(x,y-1,z-1,0) + LIGHT(x+1,y,z-1,0)) / 5.0f / 15.0f;

					float lg0 = l*(LIGHT(x-1,y-1,z-1,1) + lg*30 + LIGHT(x,y-1,z-1,1) + LIGHT(x-1,y,z-1,1)) / 5.0f / 15.0f;
					float lg1 = l*(LIGHT(x-1,y+1,z-1,1) + lg*30 + LIGHT(x,y+1,z-1,1) + LIGHT(x-1,y,z-1,1)) / 5.0f / 15.0f;
					float lg2 = l*(LIGHT(x+1,y+1,z-1,1) + lg*30 + LIGHT(x,y+1,z-1,1) + LIGHT(x+1,y,z-1,1)) / 5.0f / 15.0f;
					float lg3 = l*(LIGHT(x+1,y-1,z-1,1) + lg*30 + LIGHT(x,y-1,z-1,1) + LIGHT(x+1,y,z-1,1)) / 5.0f / 15.0f;

					float lb0 = l*(LIGHT(x-1,y-1,z-1,2) + lb*30 + LIGHT(x,y-1,z-1,2) + LIGHT(x-1,y,z-1,2)) / 5.0f / 15.0f;
					float lb1 = l*(LIGHT(x-1,y+1,z-1,2) + lb*30 + LIGHT(x,y+1,z-1,2) + LIGHT(x-1,y,z-1,2)) / 5.0f / 15.0f;
					float lb2 = l*(LIGHT(x+1,y+1,z-1,2) + lb*30 + LIGHT(x,y+1,z-1,2) + LIGHT(x+1,y,z-1,2)) / 5.0f / 15.0f;
					float lb3 = l*(LIGHT(x+1,y-1,z-1,2) + lb*30 + LIGHT(x,y-1,z-1,2) + LIGHT(x+1,y,z-1,2)) / 5.0f / 15.0f;

					float ls0 = l*(LIGHT(x-1,y-1,z-1,3) + ls*30 + LIGHT(x,y-1,z-1,3) + LIGHT(x-1,y,z-1,3)) / 5.0f / 15.0f;
					float ls1 = l*(LIGHT(x-1,y+1,z-1,3) + ls*30 + LIGHT(x,y+1,z-1,3) + LIGHT(x-1,y,z-1,3)) / 5.0f / 15.0f;
					float ls2 = l*(LIGHT(x+1,y+1,z-1,3) + ls*30 + LIGHT(x,y+1,z-1,3) + LIGHT(x+1,y,z-1,3)) / 5.0f / 15.0f;
					float ls3 = l*(LIGHT(x+1,y-1,z-1,3) + ls*30 + LIGHT(x,y-1,z-1,3) + LIGHT(x+1,y,z-1,3)) / 5.0f / 15.0f;

					VERTEX(index, x-0.5f, y-0.5f, z-0.5f, u2,v1, lr0,lg0,lb0,ls0);
					VERTEX(index, x-0.5f, y+0.5f, z-0.5f, u2,v2, lr1,lg1,lb1,ls1);
					VERTEX(index, x+0.5f, y+0.5f, z-0.5f, u1,v2, lr2,lg2,lb2,ls2);

					VERTEX(index, x-0.5f, y-0.5f, z-0.5f, u2,v1, lr0,lg0,lb0,ls0);
					VERTEX(index, x+0.5f, y+0.5f, z-0.5f, u1,v2, lr2,lg2,lb2,ls2);
					VERTEX(index, x+0.5f, y-0.5f, z-0.5f, u1,v1, lr3,lg3,lb3,ls3);
				}
			}
		}
	}
	return new Mesh(buffer, index / VERTEX_SIZE, chunk_attrs);
}
