#include "../Block/Block.h"

class Door:public Block{
	
private:
	PxRigidDynamic* door = NULL;
	
public:
	Door() {

	}
	Door(string name, PxVec3 position, PxReal halfX, PxReal halfY, PxReal halfZ)
		:Block(name, position, halfX, halfY, halfZ,BlockType::door) {

	}
	Door(std::string name, PxVec3 position, PxReal halfX, PxReal halfY, PxReal halfZ, PxRigidDynamic* door)
		:Block(name, position, halfX, halfY, halfZ, BlockType::door) {
		this->door = door;
	}

	void attachDoorActor(PxRigidDynamic* door);
	PxRigidDynamic* getDoorActor();
};