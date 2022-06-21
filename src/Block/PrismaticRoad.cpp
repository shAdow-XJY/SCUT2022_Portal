#include "PrismaticRoad.h"

void PrismaticRoad::attachPrismaticRoadActor(PxRigidDynamic* prismaticRoad) {
	this->prismaticRoad = prismaticRoad;
}

PxRigidDynamic* PrismaticRoad::getPrismaticRoadActor() {
	return this->prismaticRoad;
}