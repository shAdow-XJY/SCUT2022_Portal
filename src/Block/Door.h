#include "../Block/Block.h"

class Door:public Block{
	
private:
	PxRigidDynamic* door = NULL;
	bool open = true;
	PxRevoluteJoint* joint;
	//可以开的门是开还是关的状态
	bool doorStatus = false;
public:
	Door() {

	}
	Door(string name, PxVec3 position, PxReal halfX, PxReal halfY, PxReal halfZ, bool canOpen)
		:Block(name, position, halfX, halfY, halfZ,BlockType::door) {
		this->open = canOpen;
	}
	Door(std::string name, PxVec3 position, PxReal halfX, PxReal halfY, PxReal halfZ, PxRigidDynamic* door, bool canOpen, PxRevoluteJoint* revolute)
		:Block(name, position, halfX, halfY, halfZ, BlockType::door) {
		this->door = door;
		this->open = canOpen;
		this->joint = revolute;
	}

	void attachDoorActor(PxRigidDynamic* door);
	PxRigidDynamic* getDoorActor();
	bool canOpen();
	PxRevoluteJoint* getJoint();

	void addPForce(PxVec3 force);

	//可以开的门的当前状态
	void setDoorStatus(bool doorstatus);
	bool getDoorStauts();
};