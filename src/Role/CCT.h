#pragma once
#include <ctype.h>
#include "PxPhysicsAPI.h"
using namespace physx;

extern PxScene* gScene;
extern PxMaterial* gMaterial;
extern PxPhysics* gPhysics;

class CCTRole {
private:
	PxRigidBody* role;
	PxControllerManager* manager;
	PxController* controller;
public:
	CCTRole();
	~CCTRole();
	PxRigidBody* getActor();
	void setPosition(PxExtendedVec3 position);
	void roleMove(PxControllerFilters filters);
};