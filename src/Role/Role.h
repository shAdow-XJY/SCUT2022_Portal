#include <ctype.h>
#include <iostream>
#include "PxPhysicsAPI.h"
#include "../Block/Block.h"
#include <glut.h>
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

	PxF32 roleRadius = 1.0f;
	PxF32 roleHeight = 2.0f;

	PxVec3 speed = PxVec3(0, 0, 0);
	PxVec3 nowPostion;	
	PxVec3 lastPostion;	
	bool isMoving = false; 

	bool isJump = false; 
	float littleJumpSpeed = 0.06;
	float bigJumpSpeed = 0.08;
	float nowJumpHeight = 0.0;
	float wantJumpHeight = 12.0;
	float maxJumpHeight = 20.0;

	bool isFall = false;
	bool isAlive = true;
	//重力加速度
	float midFallSpeed = 0.08;
public:
	Role();
	~Role() {
		this->role->release();
		this->roleController->release();
	};

	bool getRoleStatus();
	void gameOver();
	void setFootPosition(PxVec3 position);
	PxVec3 getFootPosition();
	void updatePosition();
	//速度相关
	PxVec3 getSpeed();
	void setSpeed(PxVec3 speed);
	//角色移动相关
	void roleMoveByMouse(int x, int y);
	void roleMoveByMouse(PxVec3 position);
	void move();
	void move(GLint key, bool status);
	bool getMovingStatus();
	void stopMoving();

	Block standingBlock;

	void tryJump(bool release);
	void roleJump();
	void roleFall();
	void fall();

	//下蹲
	void roleCrouch();
	void roleNoCrouch();
};

class RoleHitCallback :public PxUserControllerHitReport {
public:
	void onShapeHit(const PxControllerShapeHit& hit) {
	}
	void onControllerHit(const PxControllersHit& hit) {
	}
	void  onObstacleHit(const PxControllerObstacleHit& hit) {
	}
};

class RoleHitBehaviorCallback :public PxControllerBehaviorCallback {
private:
	Role* role = NULL;
public:
	RoleHitBehaviorCallback(Role* role) :role(role) {};
	PxControllerBehaviorFlags getBehaviorFlags(const PxShape& shape, const PxActor& actor){	
		//是否接触到地面
		if (actor.getName() != "Ground") {
			this->role->stopMoving();
		}
		if (actor.getName() == "over") {
			this->role->gameOver();
		}
		return PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT;
	}
	PxControllerBehaviorFlags getBehaviorFlags(const PxController& controller) {
		return PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT;
	}
	PxControllerBehaviorFlags  getBehaviorFlags(const PxObstacle& obstacle) {
		return PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT;
	}
};