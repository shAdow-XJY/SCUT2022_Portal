#pragma once
#ifndef ANIMATION_H
#define ANIMATION_H

#include <Animation/MeshUtils.h>
#include <string>
#include <map>
using namespace std;
#include"../../src/Role/Role.h"

class Role;

class Animation
{
private:
    const aiScene* scene;
    //储存动画数组
    map<string, const aiScene*> animations;
    string current_animation = "walk";
    aiVector3D scene_min;
    aiVector3D scene_max;

    aiVector3D rootPosition;
    std::vector<FBXMesh> initial_state;

    //动画播放毫秒数
    int millisSinceStart = 0.0;
    void updating(aiAnimation* anim, double tick);

    //角色位置绑定
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
    bool update(double playSpeed,bool once = false);

    //模型绘制显示函数
    void display();

    //切换动画状态机
    void setAnimation(string animationName);


    void cleanup();

    //模型绑定位置和旋转移动
    void attachRole(Role&);
    void changeOrientation(const PxQuat& orientation);

    //第一次播放死亡动画
    bool firstOver = true;
};

#endif