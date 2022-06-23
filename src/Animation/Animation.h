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

    // �ı�ģ�ͳ���ľ���
    PxMat44 yRotate;
public:
    //��ʼ��FBXģ�ͺ�Idle����
    void init();
    //����Ԥ�趯��
    void initAssetAnimaion();

    //��ȡ��ǰ��������
    string getCurrentAnimation();

    //�������º���
    bool update(int millisSinceStart,bool autoDisplay = false);
    void updating(aiAnimation* anim, double tick);

    //ģ�ͻ�����ʾ����
    void display();
    //�л�����״̬��
    void setAnimation(string animationName);


    void cleanup();

    void attachRole(Role&);
    void changeOrientation(const PxQuat& orientation);
};

#endif