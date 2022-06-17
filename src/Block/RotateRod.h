#include "../Block/Block.h"

class RotateRod :public Block {

private:
	PxRigidDynamic* rotateRod = NULL;

public:
	RotateRod() {

	}
	RotateRod(string name, PxVec3 position, PxReal halfX, PxReal halfY, PxReal halfZ)
		:Block(name, position, halfX, halfY, halfZ, BlockType::rotateRod) {
	}
	RotateRod(std::string name, PxVec3 position, PxReal halfX, PxReal halfY, PxReal halfZ, PxRigidDynamic* rotateRod)
		:Block(name, position, halfX, halfY, halfZ, BlockType::rotateRod) {
		this->rotateRod = rotateRod;
	}

	void attachRotateRodActor(PxRigidDynamic* rotateRod);
	PxRigidDynamic* getRotateRodActor();
};