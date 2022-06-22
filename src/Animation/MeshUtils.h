#pragma once
#ifndef MESH_UTILS_H
#define MESH_UTILS_H

#include "../Role/Role.h"
#include <Animation/assimp_extras.h>
#include <vector>
#include <map>

using namespace std;

class FBXMesh
{
  public:
	std::vector<aiVector3D> vertices;
	std::vector<aiVector3D> normals;
	FBXMesh(std::vector<aiVector3D> vertices, std::vector<aiVector3D> normals) : vertices(vertices), normals(normals) {}
};

// Find the keyframes that the animation is currently between, and produce an interpolated rotation.
aiMatrix4x4 get_interpolated_rotation(double tick, aiNodeAnim* node);
// Find the keyframes that the animation is currently between, and produce an interpolated rotation.
aiMatrix4x4 get_interpolated_position(double tick, aiNodeAnim* node);

void loadGLTextures(const aiScene* scene, std::map<int, int>& texIdMap);

// ------A recursive function to traverse scene graph and render each mesh----------
void render(const aiScene* sc, const aiNode* nd, std::map<int, int> texMap);
// ------A non-recursive function to traverse scene graph and render each mesh----------
void renderDisplay(const aiScene* sc, const aiNode* nd, std::map<int, int> texMap,PxMat44 transform);
void tempDisplay(const aiScene* sc, const aiNode* nd, std::map<int, int> texMap, PxMat44 transform);
#endif