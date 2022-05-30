#include "CCT.h"

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
	desc.contactOffset = PxF32(1.0);
	controller = manager->createController(desc);
	controller->setContactOffset(PxF32(1.0));
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

void CCTRole::roleMove(PxControllerFilters filters) {
	controller->move(PxVec3(0.0, 0.8, 0.0), PxF32(0.5), PxF32(1.0), filters);
}

