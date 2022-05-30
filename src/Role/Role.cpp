#include "Role.h"
void Role::setRolePosition(PxVec3 positon) {
	this->role->setGlobalPose(PxTransform(positon));
}