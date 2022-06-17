#include "../Block/Block.h"

class PrismaticRoad :public Block {

private:
	PxRigidDynamic* prismaticRoad = NULL;

public:
	PrismaticRoad() {

	}
	PrismaticRoad(string name, PxVec3 position, PxReal halfX, PxReal halfY, PxReal halfZ)
		:Block(name, position, halfX, halfY, halfZ, BlockType::prismaticRoad) {
	}
	PrismaticRoad(std::string name, PxVec3 position, PxReal halfX, PxReal halfY, PxReal halfZ, PxRigidDynamic* prismaticRoad)
		:Block(name, position, halfX, halfY, halfZ, BlockType::prismaticRoad) {
		this->prismaticRoad = prismaticRoad;
	}

	void attachPrismaticRoadActor(PxRigidDynamic* prismaticRoad);
	PxRigidDynamic* getPrismaticRoadActor();
};
