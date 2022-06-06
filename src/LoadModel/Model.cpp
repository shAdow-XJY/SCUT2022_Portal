#include"Model.h"
using std::queue;

Model::Model(const char* path) {
	loadModel(path);
}

void Model::loadModel(const char* path) {
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_FlipUVs);
	if (scene == nullptr || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
		std::cout << "FAILED to load model: " << path << std::endl;
		return;
	}
	// 模型成功导入之后，整体数据是一棵树，每个node下面可能有数个单独的mesh
	processNode(scene->mRootNode, scene);
}


void Model::processNode(aiNode* node, const aiScene* scene) {
	// 遍历所有node
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
	PxVec3* vertices = new PxVec3[mesh->mNumVertices];
	for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
		vertices[i].x = mesh->mVertices[i].x;
		vertices[i].y = mesh->mVertices[i].y;
		vertices[i].z = mesh->mVertices[i].z;
	}
	mesh_desc.points.data = vertices;
	mesh_desc.points.stride = sizeof(PxVec3);
	mesh_desc.triangles.count = mesh->mNumFaces;

	PxU32* indices = new PxU32[3 * mesh->mNumVertices];
	for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; ++j) {
			indices[i * 3 + j] = face.mIndices[j];
		}
	}
	mesh_desc.triangles.data = indices;

	// 用vector似乎不满足MeshDescription的要求
	/*vector<PxU32> indices;
	for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; ++j) {
			indices.push_back(face.mIndices[j]);
		}
	}
	mesh_desc.triangles.data = &indices[0];*/

	mesh_desc.triangles.stride = 3 * sizeof(PxU32);

#ifdef _DEBUG
	bool res = gCooking->validateTriangleMesh(mesh_desc);
	PX_ASSERT(res);
#endif
	// 根据MeshDescription生成Mesh
	/*PxTriangleMesh* triMesh;
	triMesh = gCooking->createTriangleMesh(mesh_desc, gPhysics->getPhysicsInsertionCallback());
	m_triangleMesh.push_back(triMesh);*/

	PxDefaultMemoryOutputStream writeBuffer;
	PxTriangleMeshCookingResult::Enum result;
	bool status = gCooking->cookTriangleMesh(mesh_desc, writeBuffer, &result);
	assert(status);
	PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
	PxTriangleMesh* triMesh;
	triMesh = gPhysics->createTriangleMesh(readBuffer);
	m_triangleMesh.push_back(triMesh);

	
	return mesh_desc;
}


void Model::createMeshActor(const PxTransform& vec) {
	for (auto mesh : m_triangleMesh) {
		//auto mesh = m_triangleMesh[1];
		// 根据mesh描述创建几何体
		PxTriangleMeshGeometry geom(mesh);
		// 在指定位置（？）创建mesh
		PxRigidStatic* TriangleMesh = gPhysics->createRigidStatic(vec);

		// 创建Shape
		PxShape* shape = gPhysics->createShape(geom, *gMaterial);
		{
			shape->setContactOffset(0.02f);
			shape->setRestOffset(-0.02f);
			shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
		}
		TriangleMesh->attachShape(*shape);
		shape->release();
		TriangleMesh->userData = new int;
		int testid = 8888;
		memcpy(TriangleMesh->userData, &testid, sizeof(int));

		gScene->addActor(*TriangleMesh);
	}
}
