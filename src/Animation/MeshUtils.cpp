#include "MeshUtils.h"
#include <IL/il.h>
#include <iostream>
#include <glut.h>

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

void loadGLTextures(const aiScene *scene, std::map<int, int> &texIdMap)
{

	/* initialization of DevIL */
	ilInit();
	if (scene->HasTextures())
	{
		std::cout << "Support for meshes with embedded textures is not implemented" << endl;
		exit(1);
	}

	/* scan scene's materials for textures */
	/* Simplified version: Retrieves only the first texture with index 0 if present*/
	for (unsigned int m = 0; m < scene->mNumMaterials; ++m)
	{
		aiString path; // filename

		if (scene->mMaterials[m]->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
		{
			std::string s(path.C_Str());
			std::string delimiter = "/";
			std::string token = s.substr(s.rfind(delimiter) + 1, s.length());

			std::string s2("models/Model3_X/");
			s2.append(token);

			glEnable(GL_TEXTURE_2D);
			ILuint imageId;
			GLuint texId;
			ilGenImages(1, &imageId);
			glGenTextures(1, &texId);
			texIdMap[m] = texId;  //store tex ID against material id in a hash map
			ilBindImage(imageId); /* Binding of DevIL image name */
			ilEnable(IL_ORIGIN_SET);
			ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
			if (ilLoadImage((ILstring)s2.c_str())) //if success
			{
				/* Convert image to RGBA */
				ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);

				/* Create and load textures to OpenGL */
				glBindTexture(GL_TEXTURE_2D, texId);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ilGetInteger(IL_IMAGE_WIDTH),
							 ilGetInteger(IL_IMAGE_HEIGHT), 0, GL_RGBA, GL_UNSIGNED_BYTE,
							 ilGetData());
				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
				cout << "Texture:" << s2 << " successfully loaded." << endl;
				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
				glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
				glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
				glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
			}
			else
			{
				cout << "Couldn't load Image:\n"
					 << s2 << endl;
			}
		}
	} //loop for material
}

// ------A recursive function to traverse scene graph and render each mesh----------
void render(const aiScene *sc, const aiNode *nd, std::map<int, int> texMap)
{
	aiMatrix4x4 m = nd->mTransformation;
	aiMesh *mesh;
	aiFace *face;
	//GLuint texId;
	int meshIndex; //, materialIndex;

	aiTransposeMatrix4(&m); //Convert to column-major order
	glPushMatrix();
	glMultMatrixf((float *)&m); //Multiply by the transformation matrix for this node

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
			float black[3] = {0, 0, 0};
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
					glColor4fv((GLfloat *)&mesh->mColors[0][vertexIndex]);

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
