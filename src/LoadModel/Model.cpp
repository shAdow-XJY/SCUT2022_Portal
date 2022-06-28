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
	// �ֱ���ȡ·�����ļ���
	m_path = path;
	string::size_type found = m_path.find_last_of("/\\");
	m_modelName = m_path.substr(found + 1);
	m_path = m_path.substr(0, found);
	
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
	Mesh currMesh;
	// ���������ꡢ�������Լ���ͼ����
	for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
		PxVec3 currVertex;
		currVertex.x = mesh->mVertices[i].x;
		currVertex.y = mesh->mVertices[i].y;
		currVertex.z = mesh->mVertices[i].z;
		currMesh.m_vertices.push_back(currVertex);

		PxVec3 currNormal;
		currNormal.x = mesh->mNormals[i].x;
		currNormal.y = mesh->mNormals[i].y;
		currNormal.z = mesh->mNormals[i].z;
		currMesh.m_normals.push_back(currNormal);

		PxVec2 currTexCoord;
		if (mesh->mTextureCoords[0]) {
			currTexCoord.x = mesh->mTextureCoords[0]->x;
			currTexCoord.y = mesh->mTextureCoords[0]->y;
		}
		else {
			currTexCoord.x = currTexCoord.y = 0.0;
		}
		currMesh.m_texCoords.push_back(currTexCoord);
	}

	// ��������
	for (unsigned i = 0; i < mesh->mNumFaces; ++i) {
		aiFace face = mesh->mFaces[i];
		for (unsigned j = 0; j < face.mNumIndices; ++j) {
			currMesh.m_indices.push_back(face.mIndices[j]);
			//m_indices[i * 3 + j] = face.mIndices[j];
		}
	}

	// ��������
	if (mesh->mMaterialIndex >= 0) {
		auto material = scene->mMaterials[mesh->mMaterialIndex];
		for (unsigned i = 0; i < material->GetTextureCount(aiTextureType_DIFFUSE); ++i) {
			Texture tex;
			aiString as;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &as);
			CBMPLoader loader;
			tex.filename = as.C_Str();
			string::size_type pos = tex.filename.find_first_of('\\');
			if (pos != -1) {
				tex.filename[pos] = '/';
			}
			tex.filename = m_path + '/' + tex.filename;
			bool isDuplicate = false;
			// ��������Ƿ��ظ�ʹ��
			for (const auto& t : m_textures) {
				if (t.filename == tex.filename) {
					isDuplicate = true;
					tex.m_texID = t.m_texID;
					break;
				}
			}
			if (!isDuplicate) {
				tex.m_texID = loader.generateModelID(tex.filename.c_str());
			}
			isDuplicate = false;
			currMesh.m_texture = tex;
			m_textures.push_back(tex);
		}
	}

	m_meshes.push_back(currMesh);
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
	size_t ret = 0;
	for (auto i : m_meshes) {
		ret += i.m_vertices.size();
	}
	return ret / 3;
}
