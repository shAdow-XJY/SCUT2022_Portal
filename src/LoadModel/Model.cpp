#include"Model.h"
using std::queue;

/*
* @brief 加载并Cook模型，生成TriangleMesh
* @param path 模型路径
*/

Model::Model(const char* path) {
	loadModel(path);
}

void Model::loadModel(const char* path) {
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, /*aiProcess_Triangulate | */aiProcess_FlipUVs);
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
			processSingleMesh(currentMesh, scene);
		}
		for (unsigned int i = 0; i < currentNode->mNumChildren; ++i) {
			nodeQueue.push(currentNode->mChildren[i]);
		}
		nodeQueue.pop();
	}
}

void Model::processSingleMesh(aiMesh* mesh, const aiScene* scene) {
	// 对每个node下面的mesh，将其转换到Px的MeshDescription类中
	PxTriangleMeshDesc mesh_desc;
	mesh_desc.points.count = mesh->mNumVertices;
	PxVec3* vertices = new PxVec3[mesh->mNumVertices];
	for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
		PxVec3 currVertex;
		currVertex.x = mesh->mVertices[i].x;
		currVertex.y = mesh->mVertices[i].y;
		currVertex.z = mesh->mVertices[i].z;
		vertices[i] = currVertex;
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

	//// vector形式的索引数据对部分模型可能出现三角形错乱的情况
	//vector<PxU32> indices;
	//for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
	//	aiFace face = mesh->mFaces[i];
	//	for (unsigned int j = 0; j < face.mNumIndices; ++j) {
	//		indices.push_back(face.mIndices[j]);
	//	}
	//}
	//mesh_desc.triangles.data = &indices[0];

	mesh_desc.triangles.stride = 3 * sizeof(PxU32);

#ifdef _DEBUG
	bool res = gCooking->validateTriangleMesh(mesh_desc);
	PX_ASSERT(res);
#endif
	// 根据MeshDescription生成Mesh
	PxDefaultMemoryOutputStream writeBuffer;
	PxTriangleMeshCookingResult::Enum result;
	bool status = gCooking->cookTriangleMesh(mesh_desc, writeBuffer, &result);
	assert(status);
	PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
	PxTriangleMesh* triMesh;
	triMesh = gPhysics->createTriangleMesh(readBuffer);
	m_triangleMesh.push_back(triMesh);

}

/*
* @brief 为role连接上指定的模型
* @param trans 模型的局部Pose变换
* @param actor 要使用该模型的Actor
*/
void Model::attachMeshes(const PxTransform& trans,PxRigidActor* actor) {
	// 在指定位置创建Actor
	//PxRigidDynamic* TriangleMesh = gPhysics->createRigidDynamic(vec);
	//TriangleMesh->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
	for (size_t i = 0; i < m_triangleMesh.size();++i) {
		
		// 根据mesh描述创建几何体
		PxTriangleMeshGeometry geom(m_triangleMesh[i]/*,PxMeshScale(PxVec3(0.5f,0.5f,0.5f))*/);

		// 创建Shape
		PxShape* shape = gPhysics->createShape(geom, *gMaterial);
		{
			shape->setContactOffset(0.02f);
			shape->setRestOffset(-0.02f);
			shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
			shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, false);
			shape->setLocalPose(trans);
		}
		
		//TriangleMesh->attachShape(*shape);
 
		actor->attachShape(*shape);

		shape->release();
		/*TriangleMesh->userData = new int;
		int testid = 8888;
		memcpy(TriangleMesh->userData, &testid, sizeof(int));*/

		/*PxRigidDynamic* TriangleMesh = gPhysics->createRigidDynamic(vec);
		TriangleMesh->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
		PxMaterial* material = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);

		PxShape* shape = PxRigidActorExt::createExclusiveShape(*TriangleMesh,
			PxTriangleMeshGeometry(mesh), *material);*/
		
	}
	//gScene->addActor(*TriangleMesh);
}
