#pragma once
#include <ctype.h>
#include <string>
#include "PxPhysicsAPI.h"
using namespace physx;
using namespace std;

//用于过滤的属性
const PxFilterData collisionGroupIgnore(0, 0, 0, 1);  //忽略碰撞的
const PxFilterData collisionGroup(1, 0, 0, 0); // 检测碰撞的

enum OrganType
{
	error = -1,
	block = 0, //普通方块
	wall = 1,
	road = 2, //道路
	prop = 3,
	door = 4, //门
	seesaw = 5,
	seesawbox = 6,
	iceroad = 7,  //冰面
	rotateRod = 8, //旋转杆
	prismaticRoad = 9, //平移路面
	sphere = 10, //普通球体
	pendulum = 11, //摆锤
	ground = 12,  //地面
	keyDoor = 13 //需要钥匙的门
};

static int totalCheckpoint = 1;

//方块基类，后续可拓展实现
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

