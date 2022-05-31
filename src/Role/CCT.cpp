#include "CCT.h"
#include <iostream>

CCTRole::CCTRole() {
	manager = PxCreateControllerManager(*gScene);
	//deal with overlap
	manager->setOverlapRecoveryModule(true);
	PxCapsuleControllerDesc desc;
	desc.radius = PxF32(4.0);
	desc.height = PxF32(8.0);
	desc.material = gMaterial;
	desc.upDirection = PxVec3(0.0, 1.0, 0.0);
	//skin width/ 可以理解为碰撞盒子体积
	desc.contactOffset = PxF32(0.000001);
	controller = manager->createController(desc);
	controller->setContactOffset(PxF32(0.000001));
	role = controller->getActor();
}

CCTRole::~CCTRole(){
	role->release();
	controller->release();
	manager->release();
}

PxRigidBody* CCTRole::getActor() {
	return role;
}

void CCTRole::setPosition(PxExtendedVec3 position) {
	controller->setPosition(position);
}

void CCTRole::setDirect(PxVec3 cameraDirect) {
	PxVec3 NormalMove = PxVec3(cameraDirect.x,littleJumpSpeed, cameraDirect.z).getNormalized();
	directX = NormalMove.x;
	directZ = NormalMove.z;
}

void CCTRole::resetDirect(){
	directX = 0.0;
	directZ = 0.0;
}

void CCTRole::tryJump() {
	if (!isJump) {
		isJump = true;
	}
}

void CCTRole::roleJump() {
	if (isJump) {
		float speed = 0.0;
		if (nowJumpHeight <= maxJumpHeight / 2) {
			speed = bigJumpSpeed;
		}
		else
		{
			speed = littleJumpSpeed;
		}
		PxControllerCollisionFlags flag = controller->move(PxVec3(directX, speed, directZ), PxF32(0.00001), PxF32(0.1), NULL);
		nowJumpHeight += speed;
		if (nowJumpHeight >= maxJumpHeight || flag == PxControllerCollisionFlag::eCOLLISION_UP)
		{
			nowJumpHeight = 0.0;
			isJump = false;
			isFall = true;
		}
	}
}

void CCTRole::roleFall() {
	if (!isJump) {
		if (isFall) {
			PxControllerCollisionFlags flag = controller->move(PxVec3(directX, -midFallSpeed, directZ), PxF32(0.00001), PxF32(0.1), NULL);
			if (flag == PxControllerCollisionFlag::eCOLLISION_DOWN) {
				isFall = false;
				resetDirect();
			}
		}
	}
}
