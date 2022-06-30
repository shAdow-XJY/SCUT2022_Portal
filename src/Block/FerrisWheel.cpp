#include "FerrisWheel.h"

void FerrisWheel::attachFerrisWheelActor(PxRigidDynamic* ferrisWheel) {
	this->ferrisWheel = ferrisWheel;
}

PxRigidDynamic* FerrisWheel::getFerrisWheelActor() {
	return this->ferrisWheel;
}