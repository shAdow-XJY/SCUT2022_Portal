#include "MeshUtils.h"
#include <IL/il.h>
#include <iostream>
#include <glut.h>
#include <LoadModel/stb_image.h>

extern map<string, unsigned int> textureMap;

//load FBX
map<int, int> getTextureMap(aiMaterial* mat, const aiScene* scene)
{
	map<int, int> texMap;
	vector<Texture> resultVector = loadMaterialTextures(mat, scene);
	int a = 0;
	for (auto text : resultVector) {
		texMap.insert(pair<int, int>(a,text.m_texID));
		a++;
	}
	return texMap;
}

vector<Texture> loadMaterialTextures(aiMaterial* mat,const aiScene* scene)
{
	vector<Texture> textures;

	vector<Texture> textures_loaded;
	for (unsigned int i = 0; i < mat->GetTextureCount(aiTextureType_DIFFUSE); i++)
	{
		aiString imagePath;
		mat->GetTexture(aiTextureType_DIFFUSE, i, &imagePath);
		// check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
		bool skip = false;
		for (unsigned int j = 0; j < textures_loaded.size(); j++)
		{
			if (std::strcmp(textures_loaded[j].filename.c_str(), imagePath.C_Str()) == 0)
			{
				textures.push_back(textures_loaded[j]);
				skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
				break;
			}
		}
		if (!skip)
		{
			// if texture hasn't been loaded already, load it
			Texture texture;

			//auto filePath = directory + str.C_Str() ;

			//利用此方法判断是否是FBX模型内嵌贴图
			auto tex = scene->GetEmbeddedTexture(imagePath.C_Str());

			if (tex != nullptr)
			{
				//有内嵌贴图
				texture.m_texID = TextureFrom_FBX_EmbeddedTexture(tex);
			}

			//texture.type = typeName;
			texture.filename = imagePath.C_Str();
			textures.push_back(texture);
			textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
		}
	}
	return textures;
};

unsigned int TextureFrom_FBX_EmbeddedTexture(const aiTexture* aiTex)
{
	int width, height, channels;
	//unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
	unsigned char* data = nullptr;

	//FBX模型用stbi_load_form_memory加载
	if (aiTex->mHeight == 0)
	{
		data = stbi_load_from_memory(reinterpret_cast<unsigned char*>(aiTex->pcData), aiTex->mWidth, &width, &height, &channels, 0);
	}
	else
	{
		data = stbi_load_from_memory(reinterpret_cast<unsigned char*>(aiTex->pcData), aiTex->mWidth * aiTex->mHeight, &width, &height, &channels, 0);
	}

	return GenerateTex(data, width, height, channels);
}

unsigned int GenerateTex(unsigned char* data, int width, int height, int nrComponents)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);

		//伽马矫正需要设置内部格式（第三个参数）为GL_SRGB或者GL_SRGB_ALPHA，这里未设置，可参考伽马矫正那一章节
		//https://learnopengl-cn.github.io/05%20Advanced%20Lighting/02%20Gamma%20Correction/
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		//glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}



// Find the keyframes that the animation is currently between, and produce an interpolated rotation.
aiMatrix4x4 get_interpolated_rotation(double tick, aiNodeAnim *node)
{
	aiQuatKey prevKey;
	aiQuatKey nextKey;
	for (unsigned int frame = 0; frame < node->mNumRotationKeys; frame++)
	{
		if (tick >= node->mRotationKeys[frame].mTime)
		{
			prevKey = node->mRotationKeys[frame];
			continue;
		}

		nextKey = node->mRotationKeys[frame];

		double timeDifference = nextKey.mTime - prevKey.mTime;
		double currentTime = tick - prevKey.mTime;
		double timeLerp = currentTime / timeDifference;

		aiQuaternion outRotation;
		aiQuaternion::Interpolate(outRotation, prevKey.mValue, nextKey.mValue, timeLerp);
		return aiMatrix4x4(outRotation.GetMatrix());
	}
	// if there is only 1 keyframe, return its value
	return aiMatrix4x4(prevKey.mValue.GetMatrix());
}

// Find the keyframes that the animation is currently between, and produce an interpolated rotation.
aiMatrix4x4 get_interpolated_position(double tick, aiNodeAnim *node)
{
	aiVectorKey prevKey;
	aiVectorKey nextKey;
	for (unsigned int frame = 0; frame < node->mNumPositionKeys; frame++)
	{
		if (tick >= node->mPositionKeys[frame].mTime)
		{
			prevKey = node->mPositionKeys[frame];
			continue;
		}

		nextKey = node->mPositionKeys[frame];

		double timeDifference = nextKey.mTime - prevKey.mTime;
		double currentTime = tick - prevKey.mTime;
		double timeLerp = currentTime / timeDifference;

		aiVector3D outPosition;
		outPosition = prevKey.mValue + (nextKey.mValue - prevKey.mValue) * float(timeLerp);

		aiMatrix4x4 positionMatrix;
		aiMatrix4x4::Translation(outPosition, positionMatrix);
		return positionMatrix;
	}
	// if there is only 1 keyframe, return its value
	aiMatrix4x4 positionMatrix;
	aiMatrix4x4::Translation(prevKey.mValue, positionMatrix);
	return positionMatrix;
}

// ------A recursive function to traverse scene graph and render each mesh----------
void render(const aiScene* sc, const aiNode* nd, std::map<int, int> texMap)
{
	aiMatrix4x4 m = nd->mTransformation;
	aiMesh* mesh;
	aiFace* face;
	//GLuint texId;
	int meshIndex; //, materialIndex;

	aiTransposeMatrix4(&m); //Convert to column-major order
	glPushMatrix();
	glMultMatrixf((float*)&m); //Multiply by the transformation matrix for this node

	// Draw all meshes assigned to this node
	for (unsigned int n = 0; n < nd->mNumMeshes; n++)
	{
		meshIndex = nd->mMeshes[n];	//Get the mesh indices from the current node
		mesh = sc->mMeshes[meshIndex]; //Using mesh index, get the mesh object

		apply_material(sc->mMaterials[mesh->mMaterialIndex]); //Change opengl state to that material's properties

		if (mesh->HasNormals())
			glEnable(GL_LIGHTING);
		else
			glDisable(GL_LIGHTING);

		if (mesh->HasTextureCoords(0))
		{
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, texMap[meshIndex]);
		}
		else
		{
			float black[3] = { 0, 0, 0 };
			glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, black);
			glDisable(GL_TEXTURE_2D);
		}

		if (mesh->HasVertexColors(0))
		{
			glEnable(GL_COLOR_MATERIAL);
			glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
		}

		//Get the polygons from each mesh and draw them
		for (unsigned int k = 0; k < mesh->mNumFaces; k++)
		{
			face = &mesh->mFaces[k];
			GLenum face_mode;

			switch (face->mNumIndices)
			{
			case 1:
				face_mode = GL_POINTS;
				break;
			case 2:
				face_mode = GL_LINES;
				break;
			case 3:
				face_mode = GL_TRIANGLES;
				break;
			default:
				face_mode = GL_POLYGON;
				break;
			}

			glBegin(face_mode);

			for (unsigned int i = 0; i < face->mNumIndices; i++)
			{
				int vertexIndex = face->mIndices[i];

				if (mesh->HasVertexColors(0))
					glColor4fv((GLfloat*)&mesh->mColors[0][vertexIndex]);

				if (mesh->HasNormals())
					glNormal3fv(&mesh->mNormals[vertexIndex].x);

				if (mesh->HasTextureCoords(0))
				{
					glTexCoord2f(mesh->mTextureCoords[0][vertexIndex].x, mesh->mTextureCoords[0][vertexIndex].y);
				}

				glVertex3fv(&mesh->mVertices[vertexIndex].x);
			}

			glEnd();
		}
	}

	// Draw all children
	for (unsigned int i = 0; i < nd->mNumChildren; i++)
		render(sc, nd->mChildren[i], texMap);

	glPopMatrix();
}
// ------A non-recursive function to traverse scene graph and render each mesh----------
void renderDisplay(const aiScene* sc, const aiNode* nd, std::map<int, int> texMap, PxMat44 transform)
{
	// Draw all children
	glColor4f(1, 1, 1, 1);
	glEnable(GL_COLOR_MATERIAL);
	//glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
	for (unsigned int i = 0; i < nd->mNumChildren; i++)
		tempDisplay(sc, nd->mChildren[i], texMap,transform);
	
}

void tempDisplay(const aiScene* sc, const aiNode* nd, std::map<int, int> texMap,PxMat44 transform)
{
	aiMatrix4x4 m = nd->mTransformation;
	aiMesh* mesh;
	aiFace* face;
	//GLuint texId;
	int meshIndex;// , materialIndex;

	aiTransposeMatrix4(&m); //Convert to column-major order
	glPushMatrix();
	glMultMatrixf(reinterpret_cast<const float*>(&transform));
	glMultMatrixf((float*)&m); //Multiply by the transformation matrix for this node
	glScalef(0.015f, 0.015f, 0.015f);
	// Draw all meshes assigned to this node
	glBegin(GL_TRIANGLES);
	for (unsigned int n = 0; n < nd->mNumMeshes; n++)
	{
		meshIndex = nd->mMeshes[n];	//Get the mesh indices from the current node
		mesh = sc->mMeshes[meshIndex]; //Using mesh index, get the mesh object

		for (unsigned int k = 0; k < mesh->mNumFaces; k++)
		{
			face = &mesh->mFaces[k];
			for (unsigned int i = 0; i < face->mNumIndices; i++)
			{
				int vertexIndex = face->mIndices[i];

				if (mesh->HasNormals())
				{
					//cout << "if (mesh->HasNormals())" << endl;
					glNormal3fv(&mesh->mNormals[vertexIndex].x);
				}
				
				if (mesh->HasTextureCoords(0))
				{
					//cout << "if (mesh->HasTextureCoords(0))" << endl;
					glTexCoord2f(mesh->mTextureCoords[0][vertexIndex].x, mesh->mTextureCoords[0][vertexIndex].y);
				}
				glVertex3fv(&mesh->mVertices[vertexIndex].x);
			}
		}

	}
	glEnd();
	glPopMatrix();
}