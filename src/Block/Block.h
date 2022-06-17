#pragma once
#include <ctype.h>
#include <string>
#include "PxPhysicsAPI.h"
using namespace physx;
using namespace std;

//用于过滤的属性
const PxFilterData collisionGroupIgnore(0, 0, 0, 1);  //忽略碰撞的
const PxFilterData collisionGroup(1, 0, 0, 0); // 检测碰撞的

//盒子的高度
extern float boxHeight;

enum BlockType
{
	error = -1,
	block = 0, //普通方块
	road = 1, //道路
	prop = 2,
	door = 3, //门
	seesaw = 4,
	rotateRod = 5, //旋转杆
	prismaticRoad = 6 //平移路面
};


//方块基类，后续可拓展实现
class Block {
protected:
	PxVec3 position;
	PxReal halfX = 0;
	PxReal halfY = 0;
	PxReal halfZ = 0;
	BlockType type = BlockType::error;
	string name = "";
public:
	Block() {};
	Block(string name, PxVec3 position, PxReal halfX, PxReal halfY, PxReal halfZ, BlockType type = BlockType::block)
	:name(name),position(position), halfX(halfX), halfY(halfY), halfZ(halfZ), type(type) {}


	PxVec3 getPosition() {
		return this->position;
	}

	PxReal getHalfX(){
		return this->halfX;
	}
	PxReal getHalfY() {
		return this->halfY;
	}
	PxReal getHalfZ() {
		return this->halfZ;
	}

	BlockType getType() {
		return this->type;
	}

	string getName() {
		return this->name;
	}

	void setPosition(PxVec3 position) {
		this->position = position;
	}

	void setHalfX(PxReal halfX) {
		this->halfX = halfX;
	}

	void setHalfY(PxReal halfY) {
		this->halfY = halfY;
	}

	void setHalfZ(PxReal halfZ) {
		this->halfZ = halfZ;
	}

	void setType(BlockType type) {
		this->type = type;
	}

	void setName(string name) {
		this->name = name;
	}
};
