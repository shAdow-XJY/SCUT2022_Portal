#include "../Block/Block.h"

class Door:public Block{
	
private:
	PxRigidDynamic* door = NULL;
	bool open = true;

public:
	Door() {

	}
	Door(string name, PxVec3 position, PxReal halfX, PxReal halfY, PxReal halfZ, bool canOpen)
		:Block(name, position, halfX, halfY, halfZ,BlockType::door) {
		this->open = canOpen;
	}
	Door(std::string name, PxVec3 position, PxReal halfX, PxReal halfY, PxReal halfZ, PxRigidDynamic* door, bool canOpen)
		:Block(name, position, halfX, halfY, halfZ, BlockType::door) {
		this->door = door;
		this->open = canOpen;
	}

	void attachDoorActor(PxRigidDynamic* door);
	PxRigidDynamic* getDoorActor();
	bool canOpen();
};