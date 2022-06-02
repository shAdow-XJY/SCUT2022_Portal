#include<iostream>
#include<vector>
#include<queue>

#include<assimp/Importer.hpp>
#include<assimp/scene.h>
#include<assimp/postprocess.h>

#include "PxPhysicsAPI.h"

using namespace physx;
using std::vector;


class Model {
public:
	Model(const char* path);

private:
	vector<PxTriangleMeshDesc> meshes;
	void loadModel(const char* path);
	void processNode(aiNode* node, const aiScene* scene);
	PxTriangleMeshDesc processSingleMesh(aiMesh* mesh, const aiScene* scene);
	
};
