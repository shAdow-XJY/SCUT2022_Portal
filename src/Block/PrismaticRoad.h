#include "../Block/Block.h"

class PrismaticRoad :public Block {

private:
	PxRigidDynamic* prismaticRoad = NULL;
	PxVec3 distance = PxVec3(0, 0, 0);
	PxVec3 startPosition;
	PxVec3 endPosition;
public:
	PrismaticRoad() {

	}
	PrismaticRoad(string name, PxVec3 position, PxReal halfX, PxReal halfY, PxReal halfZ)
		:Block(name, position, halfX, halfY, halfZ, OrganType::prismaticRoad) {
	}
	PrismaticRoad(std::string name, PxVec3 position, PxReal halfX, PxReal halfY, PxReal halfZ, PxVec3 startPos, PxVec3 endPos, PxRigidDynamic* prismaticRoad)
		:Block(name, position, halfX, halfY, halfZ, OrganType::prismaticRoad) {
		this->startPosition = startPos;
		this->endPosition = endPos;
		this->prismaticRoad = prismaticRoad;
	}

	void attachPrismaticRoadActor(PxRigidDynamic* prismaticRoad);
	PxRigidDynamic* getPrismaticRoadActor();

	void updateDistance(PxVec3 pos);
	PxVec3 getStaticPosition();
	void resetDistance();

	PxVec3 getStartPosition();
	PxVec3 getEndPosition();
};
