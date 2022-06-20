#include "../Block/Block.h"

class PrismaticRoad :public Block {

private:
	PxRigidDynamic* prismaticRoad = NULL;

public:
	PrismaticRoad() {

	}
	PrismaticRoad(string name, PxVec3 position, PxReal halfX, PxReal halfY, PxReal halfZ)
		:Block(name, position, halfX, halfY, halfZ, OrganType::prismaticRoad) {
	}
	PrismaticRoad(std::string name, PxVec3 position, PxReal halfX, PxReal halfY, PxReal halfZ, PxRigidDynamic* prismaticRoad)
		:Block(name, position, halfX, halfY, halfZ, OrganType::prismaticRoad) {
		this->prismaticRoad = prismaticRoad;
	}

	void attachPrismaticRoadActor(PxRigidDynamic* prismaticRoad);
	PxRigidDynamic* getPrismaticRoadActor();
};
