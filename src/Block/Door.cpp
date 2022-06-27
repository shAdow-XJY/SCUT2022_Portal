﻿#include "Door.h"
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


//开关门逻辑
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
//需要钥匙状态
bool Door::getNeedKey() {
	return this->needKey;
}
void Door::setHasKey(bool haskey) {
	this->hasKey = hasKey;
}
bool Door::getHasKey() {
	return this->hasKey;
}