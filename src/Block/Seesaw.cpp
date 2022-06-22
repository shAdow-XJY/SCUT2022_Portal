#include "Seesaw.h"
#include <iostream>

void Seesaw::attachSeesawActor(PxRigidDynamic* seesaw) {
	this->seesaw = seesaw;
}

PxRigidDynamic* Seesaw::getSeesawActor() {
	return this->seesaw;
}

void Seesaw::attachRevolute(PxRevoluteJoint* revorevolute) {
	this->revorevolute = revorevolute;
}

/**
* @brief 跷跷板受重力影响
**/
PxVec3 Seesaw::addGForce(PxVec3 position, PxVec3 force) {
	revorevolute->setRevoluteJointFlag(PxRevoluteJointFlag::eLIMIT_ENABLED, false);
	position -= PxVec3(0, 0.25, 0);
	PxRigidBodyExt::addForceAtPos(*this->seesaw, force, position);
	return this->calTilt();
}


/**
* @brief 根据跷跷板倾斜程度得到下滑速度
**/
PxVec3 Seesaw::calTilt() {
	float angle = -this->revorevolute->getAngle();
	cout << this->revorevolute->getAngle() << endl;
	if (abs(angle) < 0.1f) return PxVec3(0, 0, 0);
	float speed = 0.3 * angle * abs(angle);
	return this->tiltDir ? PxVec3(speed, 0, 0) : PxVec3(0, 0, speed);
}