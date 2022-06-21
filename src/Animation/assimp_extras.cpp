#include "assimp_extras.h"
#include <glut.h>
#include <iostream>
using namespace std;


void get_bounding_box_for_node(const aiScene *scene, aiNode *nd, aiVector3D *min, aiVector3D *max,
							   aiMatrix4x4 trafo)
{
	aiMatrix4x4 prev = trafo;
	unsigned int n = 0, t;

	aiMultiplyMatrix4(&trafo, &nd->mTransformation);

	for (; n < nd->mNumMeshes; ++n)
	{
		const aiMesh *mesh = scene->mMeshes[nd->mMeshes[n]];
		for (t = 0; t < mesh->mNumVertices; ++t)
		{

			aiVector3D tmp = mesh->mVertices[t];
			aiTransformVecByMatrix4(&tmp, &trafo);

			min->x = aisgl_min(min->x, tmp.x);
			min->y = aisgl_min(min->y, tmp.y);
			min->z = aisgl_min(min->z, tmp.z);

			max->x = aisgl_max(max->x, tmp.x);
			max->y = aisgl_max(max->y, tmp.y);
			max->z = aisgl_max(max->z, tmp.z);
		}
	}

	for (n = 0; n < nd->mNumChildren; ++n)
	{
		get_bounding_box_for_node(scene, nd->mChildren[n], min, max, trafo);
	}
	trafo = prev;
}

// ----------------------------------------------------------------------------
void get_bounding_box(const aiScene *scene, aiVector3D *min, aiVector3D *max)
{
	aiMatrix4x4 trafo;
	aiIdentityMatrix4(&trafo);

	min->x = min->y = min->z = 1e10f;
	max->x = max->y = max->z = -1e10f;
	get_bounding_box_for_node(scene, scene->mRootNode, min, max, trafo);
}

// ----------------------------------------------------------------------------
void color4_to_float4(aiColor4D *c, float f[4])
{
	f[0] = c->r;
	f[1] = c->g;
	f[2] = c->b;
	f[3] = c->a;
}

// ----------------------------------------------------------------------------
void set_float4(float f[4], float a, float b, float c, float d)
{
	f[0] = a;
	f[1] = b;
	f[2] = c;
	f[3] = d;
}

// ----------------------------------------------------------------------------
void apply_material(aiMaterial *mtl)
{
	float c[4];

	GLenum fill_mode;
	int ret1;
	aiColor4D diffuse, specular, ambient, emission;
	float shininess;
	int two_sided;
	int wireframe;
	unsigned int max;

	set_float4(c, 1.0f, 1.0f, 1.0f, 1.0f);
	if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &diffuse))
		color4_to_float4(&diffuse, c);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, c);

	set_float4(c, 1.0f, 1.0f, 1.0f, 1.0f);
	if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_SPECULAR, &specular))
		color4_to_float4(&specular, c);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, c);

	set_float4(c, 1.0f, 1.0f, 1.0f, 1.0f);
	if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_AMBIENT, &ambient))
		color4_to_float4(&ambient, c);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, c);

	set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
	if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_EMISSIVE, &emission))
		color4_to_float4(&emission, c);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, c);

	max = 1;
	ret1 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS, &shininess, &max);
	if (ret1 == AI_SUCCESS)
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
	else
	{
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0f);
		set_float4(c, 0.0f, 0.0f, 0.0f, 0.0f);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, c);
	}

	max = 1;
	if (AI_SUCCESS == aiGetMaterialIntegerArray(mtl, AI_MATKEY_ENABLE_WIREFRAME, &wireframe, &max))
		fill_mode = wireframe ? GL_LINE : GL_FILL;
	else
		fill_mode = GL_FILL;
	glPolygonMode(GL_FRONT_AND_BACK, fill_mode);

	max = 1;
	if ((AI_SUCCESS == aiGetMaterialIntegerArray(mtl, AI_MATKEY_TWOSIDED, &two_sided, &max)) && two_sided)
		glDisable(GL_CULL_FACE);
	else
		glEnable(GL_CULL_FACE);
}

void printSceneInfo(const aiScene *scene)
{
	if (scene != NULL)
	{
		cout << "Success! " << endl;
		cout << "---------------- Scene Data -------------------" << endl;
		cout << "Number of animations = " << scene->mNumAnimations << endl;
		cout << "Number of cameras = " << scene->mNumCameras << endl;
		cout << "Number of lights = " << scene->mNumLights << endl;
		cout << "Number of materials = " << scene->mNumMaterials << endl;
		cout << "Number of meshes = " << scene->mNumMeshes << endl;
		cout << "Number of textures = " << scene->mNumTextures << endl;
		cout << "--------------------------------------" << endl;
		int nd = scene->mNumMeshes;
		for (unsigned int n = 0; n < scene->mNumMeshes; ++n)
		{
			aiMesh *mesh = scene->mMeshes[n];
			cout << "Mesh " << n << ": nverts = " << mesh->mNumVertices << "  nfaces =  " << mesh->mNumFaces << "  nbones = " << mesh->mNumBones << "  Material index = " << mesh->mMaterialIndex << endl;
		}
	}
	else
		cout << "=== Error:  Empty scene =====" << endl;
}

void printTreeInfo(const aiNode *node)
{
	int numMesh = node->mNumMeshes;
	const char *parentName;
	float *mat = new float[16];
	if (node->mParent != NULL)
		parentName = (node->mParent->mName).C_Str();
	else
		parentName = "NO PARENT";
	cout << "============= Node Data ========================" << endl;
	cout << "Node Name: " << (node->mName).C_Str() << "  Parent: " << parentName << "  nchild = " << node->mNumChildren << "  nmesh = " << numMesh << endl;
	if (numMesh > 0)
	{
		cout << "Mesh indices: ";
		for (int n = 0; n < numMesh; n++)
			cout << node->mMeshes[n] << " ";
		cout << endl;
	}
	cout << "Transformation:  ";
	mat = (float *)&(node->mTransformation.a1);
	for (int n = 0; n < 16; ++n)
		cout << mat[n] << " ";
	cout << endl;

	for (unsigned int n = 0; n < node->mNumChildren; n++)
		printTreeInfo(node->mChildren[n]);
}

void printBoneInfo(const aiScene *scene)
{
	float *mat = new float[16];
	cout << "---------------- Bone Data -------------------" << endl;
	int nd = scene->mNumMeshes;
	for (unsigned int n = 0; n < scene->mNumMeshes; ++n)
	{
		aiMesh *mesh = scene->mMeshes[n];
		if (mesh->HasBones())
		{
			for (unsigned int i = 0; i < mesh->mNumBones; i++)
			{
				aiBone *bone = mesh->mBones[i];
				cout << "Bone Name: " << (bone->mName).C_Str() << "   Mesh: " << n << "  nweights = "
					 << bone->mNumWeights << endl;
				mat = &(bone->mOffsetMatrix.a1);
				cout << "     Offset matrix: ";
				for (int k = 0; k < 16; k++)
					cout << mat[k] << " ";
				cout << endl;
				cout << "      Vertex ids: " << (bone->mWeights[0]).mVertexId << "  "
					 << (bone->mWeights[bone->mNumWeights - 1]).mVertexId << endl;
			}
		}
	}
}

void printAnimInfo(const aiScene *scene)
{
	float *pos = new float[3];
	float *quat = new float[4];
	if (scene != NULL)
	{
		cout << "---------------- Animation Data -------------------" << endl;
		cout << "Number of animations = " << scene->mNumAnimations << endl;

		for (unsigned int n = 0; n < scene->mNumAnimations; ++n)
		{
			aiAnimation *anim = scene->mAnimations[n];
			cout << " --- Anim " << n << ":  Name = " << (anim->mName).C_Str() << "  nchanls = " << anim->mNumChannels << " nticks = " << anim->mTicksPerSecond << "  duration (ticks) = " << anim->mDuration << endl;
			for (unsigned int i = 0; i < anim->mNumChannels; i++)
			{
				aiNodeAnim *ndAnim = anim->mChannels[i];
				cout << "     Channel " << i << ": nodeName = " << (ndAnim->mNodeName).C_Str() << " nposkeys = " << ndAnim->mNumPositionKeys << "  nrotKeys = " << ndAnim->mNumRotationKeys << " nsclKeys = " << ndAnim->mNumScalingKeys << endl;
				for (unsigned int k = 0; k < ndAnim->mNumPositionKeys; k++)
				{
					aiVectorKey posKey = ndAnim->mPositionKeys[k]; //Note: Does not return a pointer
					pos = (float *)&posKey.mValue;
					cout << "        posKey " << k << ":  Time = " << posKey.mTime << " Value = " << pos[0] << " " << pos[1] << " " << pos[2] << endl;
				}
				for (unsigned int k = 0; k < ndAnim->mNumRotationKeys; k++)
				{
					aiQuatKey rotnKey = ndAnim->mRotationKeys[k]; //Note: Does not return a pointer
					quat = (float *)&rotnKey.mValue;
					cout << "        rotnKey " << k << ":  Time = " << rotnKey.mTime << " Value = " << quat[0] << " " << quat[1] << " " << quat[2] << " " << quat[3] << endl;
				}
			}
		}
	}
}

