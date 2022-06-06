#include<iostream>
#include<vector>
#include<queue>
#include<assert.h>

#include<assimp/Importer.hpp>
#include<assimp/scene.h>
#include<assimp/postprocess.h>

#include "PxPhysicsAPI.h"

using namespace physx;
using std::vector;

extern PxCooking* gCooking;
extern PxPhysics* gPhysics;

class Model {
public:
	Model(const char* path);

private:
	vector<PxTriangleMeshDesc> m_meshDesc;
	vector<PxTriangleMesh*> m_triangleMesh;
	void loadModel(const char* path);
	void processNode(aiNode* node, const aiScene* scene);
	PxTriangleMeshDesc processSingleMesh(aiMesh* mesh, const aiScene* scene);
	PxRigidActor* createMeshActor(PxVec3& vec);
};
