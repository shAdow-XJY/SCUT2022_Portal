#include "RotateRod.h"
void RotateRod::attachRotateRodActor(PxRigidDynamic* rotateRod) {
	this->rotateRod = rotateRod;
}

PxRigidDynamic* RotateRod::getRotateRodActor() {
	return this->rotateRod;
}
