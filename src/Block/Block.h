#pragma once
#include <ctype.h>
#include <string>
#include "PxPhysicsAPI.h"
using namespace physx;
using namespace std;

//���ڹ��˵�����
const PxFilterData collisionGroupIgnore(0, 0, 0, 1);  //������ײ��
const PxFilterData collisionGroup(1, 0, 0, 0); // �����ײ��

//���ӵĸ߶�
extern float boxHeight;

enum BlockType
{
	error = -1,
	block = 0, //��ͨ����
	road = 1, //��·
	prop = 2,
	door = 3, //��
	seesaw = 4,
	rotateRod = 5, //��ת��
	prismaticRoad = 6 //ƽ��·��
};


//������࣬��������չʵ��
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
