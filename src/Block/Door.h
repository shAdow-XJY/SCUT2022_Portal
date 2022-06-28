#include "../Block/Block.h"

class Door :public Block {

private:
	PxRigidDynamic* door = NULL;
	bool open = true;
	PxRevoluteJoint* joint;
	//可以开的门是开还是关的状态
	bool doorStatus = false;
	//可开门是否需要key
	bool needKey = false;
	bool hasKey = false;
public:
	Door() {

	}
	Door(string name, PxVec3 position, PxReal halfX, PxReal halfY, PxReal halfZ, bool canOpen)
		:Block(name, position, halfX, halfY, halfZ, OrganType::door) {
		this->open = canOpen;
	}
	Door(std::string name, PxVec3 position, PxReal halfX, PxReal halfY, PxReal halfZ, PxRigidDynamic* door, bool canOpen, PxRevoluteJoint* revolute)
		:Block(name, position, halfX, halfY, halfZ, OrganType::door) {
		this->door = door;
		this->open = canOpen;
		this->joint = revolute;
	}
	Door(std::string name, PxVec3 position, PxReal halfX, PxReal halfY, PxReal halfZ, PxRigidDynamic* door, PxRevoluteJoint* revolute, bool needKey = true)
		:Block(name, position, halfX, halfY, halfZ, OrganType::keyDoor) {
		this->door = door;
		this->joint = revolute;
		this->open = true;
		this->needKey = needKey;
	}

	void attachDoorActor(PxRigidDynamic* door);
	PxRigidDynamic* getDoorActor();
	bool canOpen();
	PxRevoluteJoint* getJoint();

	void addPForce(PxVec3 force);

	//可以开的门的当前状态
	void setDoorStatus(bool doorstatus);
	bool getDoorStauts();
	//需要钥匙状态
	bool getNeedKey();
	void setHasKey(bool haskey);
	bool getHasKey();
};