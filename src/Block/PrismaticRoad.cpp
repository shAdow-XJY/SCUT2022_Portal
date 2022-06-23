#include "PrismaticRoad.h"

void PrismaticRoad::attachPrismaticRoadActor(PxRigidDynamic* prismaticRoad) {
	this->prismaticRoad = prismaticRoad;
}

PxRigidDynamic* PrismaticRoad::getPrismaticRoadActor() {
	return this->prismaticRoad;
}

void PrismaticRoad::updateDistance(PxVec3 pos) {
	this->distance = pos - this->prismaticRoad->getGlobalPose().p;
}

PxVec3 PrismaticRoad::getStaticPosition() {
	if (this->distance.isZero()) return this->prismaticRoad->getGlobalPose().p;
	return this->distance + this->prismaticRoad->getGlobalPose().p;
}

void PrismaticRoad::resetDistance() {
	this->distance = PxVec3(0, 0, 0);
}