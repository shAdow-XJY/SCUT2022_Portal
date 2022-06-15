#include "../Block/Block.h"
class Road :public Block {

private:
	PxRigidStatic* road = NULL;

public:
	Road() {

	}
	Road(std::string name, PxVec3 position, PxReal halfX, PxReal halfY, PxReal halfZ)
		:Block(name, position, halfX, halfY, halfZ, BlockType::road) {
	}
	Road(std::string name, PxVec3 position, PxReal halfX, PxReal halfY, PxReal halfZ, PxRigidStatic* road)
		:Block(name, position, halfX, halfY, halfZ, BlockType::road) {
		this->road = road;
	}
	Road(std::string name, PxVec3 position, PxReal halfX, PxReal halfY, PxReal halfZ, PxRigidStatic* road, BlockType type)
		:Block(name, position, halfX, halfY, halfZ, BlockType::road) {
		this->road = road;
		this->type = type;
	}
	void attachRoadActor(PxRigidStatic* road);
	PxRigidStatic* getRoadActor();
};