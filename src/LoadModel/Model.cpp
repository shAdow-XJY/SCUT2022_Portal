#include"Model.h"
using std::queue;

Model::Model(const char* path) {
	loadModel(path);
}

void Model::loadModel(const char* path) {
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
	if (scene == nullptr || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
		std::cout << "FAILED to load model: " << path << std::endl;
		return;
	}
	// ģ�ͳɹ�����֮������������һ������ÿ��node�������������������mesh
	processNode(scene->mRootNode, scene);
}


void Model::processNode(aiNode* node, const aiScene* scene) {
	// ��������node����Ȼ�����ֻ��һ����
	queue<aiNode*> nodeQueue;
	nodeQueue.push(node);
	while (!nodeQueue.empty()) {
		aiNode* currentNode = nodeQueue.front();
		for (unsigned int i = 0; i < currentNode->mNumMeshes; ++i) {
			aiMesh* currentMesh = scene->mMeshes[currentNode->mMeshes[i]];
			this->m_meshDesc.push_back(processSingleMesh(currentMesh, scene));
		}
		for (unsigned int i = 0; i < currentNode->mNumChildren; ++i) {
			nodeQueue.push(currentNode->mChildren[i]);
		}
		nodeQueue.pop();
	}
}

PxTriangleMeshDesc Model::processSingleMesh(aiMesh* mesh, const aiScene* scene) {
	// ��ÿ��node�����mesh������ת����Px��MeshDescription����
	PxTriangleMeshDesc mesh_desc;
	mesh_desc.points.count = mesh->mNumVertices;
	mesh_desc.points.data = mesh->mVertices;
	mesh_desc.points.stride = sizeof(PxVec3);
	mesh_desc.triangles.count = mesh->mNumFaces;

	vector<PxU32> indices;
	for (int i = 0; i < mesh->mNumFaces; ++i) {
		aiFace face = mesh->mFaces[i];
		for (int j = 0; j < face.mNumIndices; ++j) {
			indices.push_back(face.mIndices[j]);
		}
	}
	assert(!indices.empty());
	mesh_desc.triangles.data = &indices[0];
	mesh_desc.triangles.stride = 3 * sizeof(PxU32);

#ifdef _DEBUG
	bool res = gCooking->validateTriangleMesh(mesh_desc);
	PX_ASSERT(res);
#endif
	// ����MeshDescription����Mesh
	PxTriangleMesh* triMesh;
	triMesh = gCooking->createTriangleMesh(mesh_desc, gPhysics->getPhysicsInsertionCallback());
	m_triangleMesh.push_back(triMesh);

	return mesh_desc;
}


PxRigidActor* Model::createMeshActor(PxVec3& vec) {
	for (auto mesh : m_triangleMesh) {
		// ����mesh��������������
		PxTriangleMeshGeometry geom(mesh);
		// ��ָ��λ�ã���������mesh
		PxRigidStatic* TriangleMesh = gPhysics->createRigidStatic(PxTransform(vec));

		// ����Shape
		//PxShape* shape = gPhysics->createShape(geom,*)
	}
}
