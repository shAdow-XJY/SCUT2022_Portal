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
	// 模型成功导入之后，整体数据是一棵树，每个node下面可能有数个单独的mesh
	processNode(scene->mRootNode, scene);
}


void Model::processNode(aiNode* node, const aiScene* scene) {
	// 遍历所有node（虽然大概率只有一个）
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
	// 对每个node下面的mesh，将其转换到Px的MeshDescription类中
	PxTriangleMeshDesc mesh_desc;
	mesh_desc.points.count = mesh->mNumVertices;
	mesh_desc.points.data = mesh->mVertices;
	mesh_desc.points.stride = sizeof(PxVec3);
	mesh_desc.triangles.count = mesh->mNumFaces;

	vector<PxU32> indices;
	for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; ++j) {
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
	// 根据MeshDescription生成Mesh
	PxTriangleMesh* triMesh;
	triMesh = gCooking->createTriangleMesh(mesh_desc, gPhysics->getPhysicsInsertionCallback());
	m_triangleMesh.push_back(triMesh);

	return mesh_desc;
}


void Model::createMeshActor(const PxVec3& vec) {
	for (auto mesh : m_triangleMesh) {
		// 根据mesh描述创建几何体
		PxTriangleMeshGeometry geom(mesh);
		// 在指定位置（？）创建mesh
		PxRigidStatic* TriangleMesh = gPhysics->createRigidStatic(PxTransform(vec));

		// 创建Shape
		PxShape* shape = gPhysics->createShape(geom, *gMaterial);
		{
			shape->setContactOffset(0.02f);
			shape->setRestOffset(-0.02f);
		}
		TriangleMesh->attachShape(*shape);
		shape->release();

		gScene->addActor(*TriangleMesh);
	}
}
