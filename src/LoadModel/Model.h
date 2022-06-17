#pragma once
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
	void attachMeshes(const PxTransform& trans, PxRigidActor* actor);
	void transformModel(const PxTransform& trans);
	vector<PxVec3> m_vertices;
	vector<PxU32> m_indices;
	vector<PxVec3> m_normals;
	vector<PxVec2>m_texCoords;
private:
	vector<PxTriangleMesh*> m_triangleMesh;
	vector<PxShape*> m_shapes;
	
	void loadModel(const char* path);
	void processNode(aiNode* node, const aiScene* scene);
	void processSingleMesh(aiMesh* mesh, const aiScene* scene);
	
public:
	size_t getNbTriangles() const;
};
