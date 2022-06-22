#pragma once
#ifndef ANIMATION_H
#define ANIMATION_H

#include <Animation/MeshUtils.h>

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

public:
    void init();

    void update(int millisSinceStart);

    void display();

    void keyboard(unsigned char key);

    void cleanup();
};

#endif