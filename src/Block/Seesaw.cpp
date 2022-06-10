#include "Seesaw.h"

void Seesaw::attachSeesawActor(PxRigidDynamic* seesaw) {
	this->seesaw = seesaw;
}

PxRigidDynamic* Seesaw::getSeesawActor() {
	return this->seesaw;
}