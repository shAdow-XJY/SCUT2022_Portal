// ----------------------------------------------------------------------------
// Helper functions
//-----------------------------------------------------------------------------
#pragma once
#ifndef ASSIMP_EXTRAS_H
#define ASSIMP_EXTRAS_H

#include <assimp/cimport.h>
#include <assimp/types.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define aisgl_min(x, y) (x < y ? x : y)
#define aisgl_max(x, y) (y > x ? y : x)


void get_bounding_box_for_node(const aiScene* scene, aiNode* nd, aiVector3D* min, aiVector3D* max,
	aiMatrix4x4 trafo);

// ----------------------------------------------------------------------------
void get_bounding_box(const aiScene* scene, aiVector3D* min, aiVector3D* max);

// ----------------------------------------------------------------------------
void color4_to_float4(aiColor4D* c, float f[4]);

// ----------------------------------------------------------------------------
void set_float4(float f[4], float a, float b, float c, float d);

// ----------------------------------------------------------------------------
void apply_material(aiMaterial* mtl);

void printSceneInfo(const aiScene* scene);

void printTreeInfo(const aiNode* node);

void printBoneInfo(const aiScene* scene);

void printAnimInfo(const aiScene* scene);

#endif