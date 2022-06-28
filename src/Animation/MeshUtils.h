#pragma once
#ifndef MESH_UTILS_H
#define MESH_UTILS_H


#include <Animation/assimp_extras.h>
#include <vector>
#include <map>
#include <LoadModel/Model.h>
using namespace std;

class FBXMesh
{
  public:
	std::vector<aiVector3D> vertices;
	std::vector<aiVector3D> normals;
	FBXMesh(std::vector<aiVector3D> vertices, std::vector<aiVector3D> normals) : vertices(vertices), normals(normals) {}
};
map<int, int> getTextureMap(aiMaterial* mat, const aiScene* scene);
vector<Texture> loadMaterialTextures(aiMaterial* mat, const aiScene* scene);
unsigned int TextureFrom_FBX_EmbeddedTexture(const aiTexture* aiTex);
unsigned int GenerateTex(unsigned char* data, int width, int height, int nrComponents);

// Find the keyframes that the animation is currently between, and produce an interpolated rotation.
aiMatrix4x4 get_interpolated_rotation(double tick, aiNodeAnim* node);
// Find the keyframes that the animation is currently between, and produce an interpolated rotation.
aiMatrix4x4 get_interpolated_position(double tick, aiNodeAnim* node);

// ------A recursive function to traverse scene graph and render each mesh----------
void render(const aiScene* sc, const aiNode* nd, std::map<int, int> texMap);

// ------A non-recursive function to traverse scene graph and render each mesh----------
void renderDisplay(const aiScene* sc, const aiNode* nd, std::map<int, int> texMap,PxMat44 transform);
void tempDisplay(const aiScene* sc, const aiNode* nd, std::map<int, int> texMap, PxMat44 transform);
#endif