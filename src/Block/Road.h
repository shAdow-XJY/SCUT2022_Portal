#include "../Block/Block.h"
class Road :public Block {

private:
	PxRigidStatic* road = NULL;

public:
	Road() {

	}
	Road(std::string name, PxVec3 position, PxReal halfX, PxReal halfY, PxReal halfZ)
		:Block(name, position, halfX, halfY, halfZ, BlockType::door) {

	}
	Road(std::string name, PxVec3 position, PxReal halfX, PxReal halfY, PxReal halfZ, PxRigidStatic* road)
		:Block(name, position, halfX, halfY, halfZ, BlockType::door) {
		this->road = road;
	}
	void attachRoadActor(PxRigidStatic* road);
	PxRigidStatic* getRoadActor();
};