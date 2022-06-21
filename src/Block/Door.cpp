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
	if (this->canOpen()) {
		door->addForce(force);
		PxJointAngularLimitPair limits(-PxPi / 2, PxPi / 2, 0.01f);
		joint->setLimit(limits);
	}
}

//可以开的门的当前状态
void Door::setDoorStatus(bool doorstatus)
{
	this->doorStatus = doorstatus;
}
bool Door::getDoorStauts()
{
	if (this->open) {
		return this->doorStatus;
	}
	return false;
}