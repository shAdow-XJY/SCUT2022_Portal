#include "../Block/Block.h"
class FerrisWheel :public Block {

private:
	PxRigidDynamic* ferrisWheel = NULL;

public:
	FerrisWheel() {

	}
	FerrisWheel(std::string name, PxVec3 position, PxReal halfX, PxReal halfY, PxReal halfZ)
		:Block(name, position, halfX, halfY, halfZ, OrganType::ferrisWheel) {
	}
	FerrisWheel(std::string name, PxVec3 position, PxReal halfX, PxReal halfY, PxReal halfZ, PxRigidDynamic* ferrisWheel)
		:Block(name, position, halfX, halfY, halfZ, OrganType::ferrisWheel) {
		this->ferrisWheel = ferrisWheel;
	}
	FerrisWheel(std::string name, PxVec3 position, PxReal halfX, PxReal halfY, PxReal halfZ, PxRigidDynamic* ferrisWheel, OrganType type)
		:Block(name, position, halfX, halfY, halfZ, OrganType::ferrisWheel) {
		this->ferrisWheel = ferrisWheel;
		this->type = type;
	}
	void attachFerrisWheelActor(PxRigidDynamic* road);
	PxRigidDynamic* getFerrisWheelActor();
};