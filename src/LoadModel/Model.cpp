#include"Model.h"
using std::queue;

/*
* @brief ���ز�Cookģ�ͣ�����TriangleMesh
* @param path ģ��·��
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
			processSingleMesh(currentMesh, scene);
		}
		for (unsigned int i = 0; i < currentNode->mNumChildren; ++i) {
			nodeQueue.push(currentNode->mChildren[i]);
		}
		nodeQueue.pop();
	}
}

void Model::processSingleMesh(aiMesh* mesh, const aiScene* scene) {

	// ���������ꡢ�������Լ���ͼ����
	for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
		PxVec3 currVertex;
		currVertex.x = mesh->mVertices[i].x;
		currVertex.y = mesh->mVertices[i].y;
		currVertex.z = mesh->mVertices[i].z;
		m_vertices.push_back(currVertex);

		PxVec3 currNormal;
		currNormal.x = mesh->mNormals[i].x;
		currNormal.y = mesh->mNormals[i].y;
		currNormal.z = mesh->mNormals[i].z;
		m_normals.push_back(currNormal);

		PxVec2 currTexCoord;
		if (mesh->mTextureCoords[0]) {
			currTexCoord.x = mesh->mTextureCoords[0]->x;
			currTexCoord.y = mesh->mTextureCoords[0]->y;
		}
		else {
			currTexCoord.x = currTexCoord.y = 0.0;
		}
		m_texCoords.push_back(currTexCoord);
	}

	// ��������
	for (unsigned i = 0; i < mesh->mNumFaces; ++i) {
		aiFace face = mesh->mFaces[i];
		for (unsigned j = 0; j < face.mNumIndices; ++j) {
			m_indices.push_back(face.mIndices[j]);
			//m_indices[i * 3 + j] = face.mIndices[j];
		}
	}

	// ��������
	if (mesh->mMaterialIndex >= 0) {
		auto material = scene->mMaterials[mesh->mMaterialIndex];
		for (unsigned i = 0; i < material->GetTextureCount(aiTextureType_DIFFUSE); ++i) {
			aiString as;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &as);
			// TODO���������������ļ���ʵ��
			// �Ѿ�include��BMPLoader��ʣ�¾��Ǵ���һ��ʵ��Ȼ����
		}
	}

}



/*
* @brief Ϊrole������ָ����ģ��
* @param trans ģ�͵ľֲ�Pose�任
* @param actor Ҫʹ�ø�ģ�͵�Actor
*/
void Model::attachMeshes(const PxTransform& trans,PxRigidActor* actor) {
	// ��ָ��λ�ô���Actor
	//PxRigidDynamic* TriangleMesh = gPhysics->createRigidDynamic(vec);
	//TriangleMesh->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
	for (size_t i = 0; i < m_triangleMesh.size();++i) {
		
		// ����mesh��������������
		PxTriangleMeshGeometry geom(m_triangleMesh[i],PxMeshScale(PxVec3(2.0f,2.0f,2.0f)));

		// ����Shape
		PxShape* shape = gPhysics->createShape(geom, *gMaterial);
		{
			shape->setContactOffset(0.02f);
			shape->setRestOffset(-0.02f);
			shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
			shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, false);
			shape->setLocalPose(trans);
			m_shapes.push_back(shape);
		}
		
		//TriangleMesh->attachShape(*shape);
 
		actor->attachShape(*shape);
	}
}



size_t Model::getNbTriangles() const
{
	return m_vertices.size() / 3;
}
