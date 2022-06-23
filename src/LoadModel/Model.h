#ifndef __MODEL_H__
#define __MODEL_H__

#include<iostream>
#include<vector>
#include<queue>
#include<string>
#include<assert.h>

#include<assimp/Importer.hpp>
#include<assimp/scene.h>
#include<assimp/postprocess.h>

#include "PxPhysicsAPI.h"
#include "../Render/BMPLoader.h"

using namespace physx;
using std::vector;
using std::string;

extern PxScene* gScene;
extern PxCooking* gCooking;
extern PxPhysics* gPhysics;
extern PxMaterial* gMaterial;

struct Texture {
	unsigned int m_texID;
	string filename;
};

struct Mesh {
	vector<PxVec3> m_vertices;
	vector<PxU32> m_indices;
	vector<PxVec3> m_normals;
	vector<PxVec2> m_texCoords;
	Texture m_texture;
};

class Model {
public:
	Model(const char* path);
	void attachMeshes(const PxTransform& trans, PxRigidActor* actor);
	void transformModel(const PxTransform& trans);

	vector<Mesh> m_meshes;
	vector<Texture> m_textures;
private:
	vector<PxTriangleMesh*> m_triangleMesh;
	vector<PxShape*> m_shapes;
	string m_path;
	string m_modelName;

	void loadModel(const char* path);
	void processNode(aiNode* node, const aiScene* scene);
	void processSingleMesh(aiMesh* mesh, const aiScene* scene);

public:
	size_t getNbTriangles() const;
};

#endif

