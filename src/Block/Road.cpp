#include "Road.h"

void Road::attachRoadActor(PxRigidStatic* road) {
	this->road = road;
}

PxRigidStatic* Road::getRoadActor() {
	return this->road;
}