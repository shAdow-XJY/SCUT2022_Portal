#pragma once
#ifndef ANIMATION_H
#define ANIMATION_H

#include <Animation/MeshUtils.h>
#include <string>
#include <map>
using namespace std;
#include"../../src/Role/Role.h"

class Animation
{
private:
    const aiScene* scene;
    //std::vector<const aiScene*> animations;
    map<string, const aiScene*> animations;
    string current_animation = "walk";
    aiVector3D scene_min;
    aiVector3D scene_max;
    aiVector3D rootPosition;
    std::vector<FBXMesh> initial_state;
    Role* attachedRole;

    // 改变模型朝向的矩阵
    PxMat44 yRotate;
public:
    //初始化FBX模型和Idle动画
    void init();
    //加载预设动画
    void initAssetAnimaion();

    //获取当前动画名称
    string getCurrentAnimation();

    //动画更新函数
    bool update(int millisSinceStart,bool autoDisplay = false);
    void updating(aiAnimation* anim, double tick);

    //模型绘制显示函数
    void display();
    //切换动画状态机
    void setAnimation(string animationName);


    void cleanup();

    void attachRole(Role&);
    void changeOrientation(const PxQuat& orientation);
};

#endif