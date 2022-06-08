#include"Model.h"
using std::queue;

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
	// ģ�ͳɹ�����֮������������һ������ÿ��node�������������������mesh
	processNode(scene->mRootNode, scene);
}


void Model::processNode(aiNode* node, const aiScene* scene) {
	// ��������node
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

	/*PxU32* indices = new PxU32[3 * mesh->mNumVertices];
	for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; ++j) {
			indices[i * 3 + j] = face.mIndices[j];
		}
	}
	mesh_desc.triangles.data = indices;*/

	// vector��ʽ���������ݶԲ���ģ�Ϳ��ܳ��������δ��ҵ����
	vector<PxU32> indices;
	for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; ++j) {
			indices.push_back(face.mIndices[j]);
		}
	}
	mesh_desc.triangles.data = &indices[0];

	mesh_desc.triangles.stride = 3 * sizeof(PxU32);

#ifdef _DEBUG
	bool res = gCooking->validateTriangleMesh(mesh_desc);
	PX_ASSERT(res);
#endif
	// ����MeshDescription����Mesh
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
	for (size_t i = 0; i < m_triangleMesh.size();++i) {
		
		// ��ָ��λ�ô���Actor
		PxRigidStatic* TriangleMesh = gPhysics->createRigidStatic(vec);

		// ����mesh��������������
		PxTriangleMeshGeometry geom(m_triangleMesh[i]/*,PxMeshScale(PxVec3(0.5f,0.5f,0.5f))*/);

		// ����Shape
		PxShape* shape = gPhysics->createShape(geom, *gMaterial);
		{
			shape->setContactOffset(0.02f);
			shape->setRestOffset(-0.02f);
			shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
		}
		
		TriangleMesh->attachShape(*shape);
		shape->release();
		/*TriangleMesh->userData = new int;
		int testid = 8888;
		memcpy(TriangleMesh->userData, &testid, sizeof(int));*/

		/*PxRigidDynamic* TriangleMesh = gPhysics->createRigidDynamic(vec);
		TriangleMesh->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
		PxMaterial* material = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);

		PxShape* shape = PxRigidActorExt::createExclusiveShape(*TriangleMesh,
			PxTriangleMeshGeometry(mesh), *material);*/

		gScene->addActor(*TriangleMesh);
	}
}
