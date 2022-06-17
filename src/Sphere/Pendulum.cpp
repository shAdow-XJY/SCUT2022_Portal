#include "Pendulum.h"
void Pendulum::attachPendulumActor(PxRigidDynamic* pendulum) {
	this->pendulum = pendulum;
}

PxRigidDynamic* Pendulum::getPendulumActor() {
	return this->pendulum;
}
