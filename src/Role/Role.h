#include <ctype.h>
#include <iostream>
#include "PxPhysicsAPI.h"
using namespace physx;

extern PxScene* gScene;
extern PxMaterial* gMaterial;
extern PxPhysics* gPhysics;
extern PxControllerManager* cManager;
extern PxVec3 ScenetoWorld(int xCord, int yCord);

class Role {
private:
	PxRigidBody* role;
	PxController* roleController;
	PxVec3 nowPostion;	//角色需要达到的位置
	PxVec3 lastPostion;	//角色之前所在的位置
	bool isMoving = false;
public:
	Role();
	~Role() {
		this->role->release();
		this->roleController->release();
	};
	bool getMovingStatus();
	PxVec3 getFootPosition();
	void roleMoveByMouse(int x, int y);
	void move();
	void move(char key);
	void stopMoving();
};

class RoleHitCallback :public PxUserControllerHitReport {
public:
	void onShapeHit(const PxControllerShapeHit& hit) {
		std::cout << "我撞到了" << hit.actor->getName() << std::endl;
	}
	void onControllerHit(const PxControllersHit& hit) {
		std::cout << "我撞到了" << std::endl;
	}
	void  onObstacleHit(const PxControllerObstacleHit& hit) {
		std::cout << "我撞到了" << std::endl;
	}
};

class RoleHitBehaviorCallback :public PxControllerBehaviorCallback {
private:
	Role* role = NULL;
public:
	RoleHitBehaviorCallback(Role* role) :role(role) {};
	PxControllerBehaviorFlags getBehaviorFlags(const PxShape& shape, const PxActor& actor){	
		//判断碰撞的物体若不为Ground，则停止移动
		if (actor.getName() != "Ground") {
			this->role->stopMoving();
		}
		return PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT;
	}
	PxControllerBehaviorFlags getBehaviorFlags(const PxController& controller) {
		std::cout << "碰撞到其他controller" <<std::endl;
		return PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT;
	}
	PxControllerBehaviorFlags  getBehaviorFlags(const PxObstacle& obstacle) {
		std::cout << "碰撞到用户定义的障碍物" << std::endl;
		return PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT;
	}
};





