#pragma once
#include <ctype.h>
#include <string>
#include "PxPhysicsAPI.h"
using namespace physx;
using namespace std;

//���ڹ��˵�����
const PxFilterData collisionGroupIgnore(0, 0, 0, 1);  //������ײ��
const PxFilterData collisionGroup(1, 0, 0, 0); // �����ײ��

enum OrganType
{
	error = -1,
	block = 0, //��ͨ����
	wall = 1,
	road = 2, //��·
	prop = 3,
	door = 4, //��
	seesaw = 5,
	seesawbox = 6,
	iceroad = 7,  //����
	rotateRod = 8, //��ת��
	prismaticRoad = 9, //ƽ��·��
	sphere = 10, //��ͨ����
	pendulum = 11, //�ڴ�
	ground = 12 //����
};

static int totalCheckpoint = 1;

//������࣬��������չʵ��
class GameSceneBasic {
protected:
	PxVec3 position;
	OrganType type = OrganType::error;
	string name = "";
	int checkpoint = 0;

public:
	GameSceneBasic() {};
	GameSceneBasic(string name,PxVec3 position, OrganType type):position(position),type(type),name(name){
		this->checkpoint = totalCheckpoint;
	}

	PxVec3 getPosition() {
		return this->position;
	}

	OrganType getType() {
		return this->type;
	}

	string getName() {
		return this->name;
	}

	void setPosition(PxVec3 position) {
		this->position = position;
	}

	void setType(OrganType type) {
		this->type = type;
	}

	void setName(string name) {
		this->name = name;
	}

	int getCheckpoint() {
		return this->checkpoint;
	}
};

