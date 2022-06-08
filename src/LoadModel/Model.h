#include<iostream>
#include<vector>
#include<queue>
#include<string>
#include<assert.h>

#include<assimp/Importer.hpp>
#include<assimp/scene.h>
#include<assimp/postprocess.h>

#include "PxPhysicsAPI.h"

using namespace physx;
using std::vector;
using std::string;

extern PxScene* gScene;
extern PxCooking*	gCooking;
extern PxPhysics*	gPhysics;
extern PxMaterial* gMaterial;

class Model {
public:
	Model(const char* path);
	void createMeshActor(const PxTransform& vec);
private:
	vector<PxTriangleMesh*> m_triangleMesh;
	

	void loadModel(const char* path);
	void processNode(aiNode* node, const aiScene* scene);
	void processSingleMesh(aiMesh* mesh, const aiScene* scene);
	
};
