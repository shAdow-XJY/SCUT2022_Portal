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
    //���涯������
    map<string, const aiScene*> animations;
    string current_animation = "walk";
    aiVector3D scene_min;
    aiVector3D scene_max;

    aiVector3D rootPosition;
    std::vector<FBXMesh> initial_state;

    //�������ź�����
    int millisSinceStart = 0.0;
    void updating(aiAnimation* anim, double tick);

    //��ɫλ�ð�
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
    bool update(double playSpeed,bool once = false);

    //ģ�ͻ�����ʾ����
    void display();

    //�л�����״̬��
    void setAnimation(string animationName);


    void cleanup();

    //ģ�Ͱ�λ�ú���ת�ƶ�
    void attachRole(Role&);
    void changeOrientation(const PxQuat& orientation);

    //��һ�β�����������
    bool firstOver = true;
};

#endif