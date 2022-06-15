#include "Door.h"
void Door::attachDoorActor(PxRigidDynamic* door) {
	this->door = door;
}

PxRigidDynamic* Door::getDoorActor() {
	return this->door;
}

bool Door::canOpen() {
	return this->open;
}

PxRevoluteJoint* Door::getJoint() {
	return this->joint;
}

void Door::addPForce(PxVec3 force) {
	door->addForce(force);
	PxJointAngularLimitPair limits(-PxPi / 2, PxPi / 2, 0.01f);
	joint->setLimit(limits);
}