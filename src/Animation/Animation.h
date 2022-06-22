#pragma once
#ifndef ANIMATION_H
#define ANIMATION_H

#include <Animation/MeshUtils.h>
#include"../../src/Role/Role.h"

class Animation
{
private:
    const aiScene* scene;
    std::vector<const aiScene*> animations;
    int current_animation = 0;
    aiVector3D scene_min;
    aiVector3D scene_max;
    aiVector3D rootPosition;
    std::vector<FBXMesh> initial_state;
    Role* attachedRole;

    // 改变模型朝向的矩阵
    PxMat44 yRotate;
public:
    void init();

    void update(int millisSinceStart);

    void display();

    void keyboard(unsigned char key);

    void cleanup();

    void attachRole(Role&);
    void changeOrientation(const PxQuat& orientation);
};

#endif