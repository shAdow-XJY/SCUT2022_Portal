#include "Door.h"
void Door::attachDoorActor(PxRigidDynamic* door) {
	this->door = door;
}

PxRigidDynamic* Door::getDoorActor() {
	return this->door;
}