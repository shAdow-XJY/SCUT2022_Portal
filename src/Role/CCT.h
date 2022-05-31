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

	float directX = 0.0;
	float directZ = 0.0;

	bool isJump = false;
	float littleJumpSpeed = 0.8;
	float bigJumpSpeed = 0.9;
	float nowJumpHeight = 0.0;
	float maxJumpHeight = 20.0;

	bool isFall = false;
	float midFallSpeed = 0.85;

public:
	CCTRole();
	~CCTRole();
	PxRigidBody* getActor();

	void setPosition(PxExtendedVec3 position);
	PxVec3 getFootPosition();

	void setDirect(PxVec3 cameraDirect);
	void resetDirect();

	void roleMove();

	void tryJump();
	void roleJump();
	void roleFall();
};