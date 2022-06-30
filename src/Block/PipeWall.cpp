#include "PipeWall.h"

void PipeWall::attachPipeWallActor(PxRigidStatic* pipeWall) {
	this->pipeWall = pipeWall;
}

PxRigidStatic* PipeWall::getPipeWallActor() {
	return this->pipeWall;
}