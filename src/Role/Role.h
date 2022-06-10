#include <ctype.h>
#include <iostream>
#include "PxPhysicsAPI.h"
#include "../Block/Door.h"
#include "../Block/Seesaw.h"
#include <glut.h>
using namespace physx;

extern PxScene* gScene;
extern PxMaterial* gMaterial;
extern PxPhysics* gPhysics;
extern PxControllerManager* cManager;
extern PxVec3 ScenetoWorld(int xCord, int yCord);

const int primaryJumpHeight = 6.0f;

class Role {
private:
	PxRigidBody* role;
	PxController* roleController;

	PxF32 roleRadius = 1.0f;
	PxF32 roleHeight = 2.0f;
	//速度方向
	PxVec3 speed = PxVec3(0, 0, 0);
	//最后一次按下方向键的方向
	PxVec3 lastPressDir = PxVec3(0, 0, 1);
	//摄像机朝向
	PxVec3 dir = PxVec3(0, 0, 1);
	//人物面朝方向
	PxVec3 faceDir = PxVec3(0,0,1);
	PxVec3 nowPostion;	
	PxVec3 lastPostion;	
	bool isMoving = false;
	bool canMove = true;

	bool isJump = false;
	float littleJumpSpeed = 0.06;
	float bigJumpSpeed = 0.08;
	float nowJumpHeight = 0.0;
	float wantJumpHeight = primaryJumpHeight;
	float maxJumpHeight = 12.0;

	bool isFall = false;
	bool isAlive = true;
	//重力加速度
	float midFallSpeed = 0.08;

	bool equiped = false;
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
	PxVec3 getPosition();
	void updatePosition();
	//速度相关
	PxVec3 getSpeed();
	void setSpeed(PxVec3 speed);

	//相机朝向
	PxVec3 getDir();
	//角色朝向
	PxVec3 getFaceDir();
	//角色移动相关
	void roleMoveByMouse(int x, int y);
	void roleMoveByMouse(PxVec3 position);
	void move();
	void move(GLint key, bool status,bool free);
	bool getMovingStatus();
	void stopMoving();

	Block standingBlock;

	//放置物体
	void setEquiped(bool equip = true);
	bool getEquiped();

	//跳跃
	void tryJump(bool release);
	void roleJump();
	void roleFall();
	void fall();

	//下蹲
	void roleCrouch();
	void roleNoCrouch();

	void changeCanMove(bool);
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

/**
* @brief 角色碰撞判定，用于角色撞物体和推物体
**/
class RoleHitBehaviorCallback :public PxControllerBehaviorCallback {
private:
	Role* role = NULL;
public:
	RoleHitBehaviorCallback(Role* role) :role(role) {};
	PxControllerBehaviorFlags getBehaviorFlags(const PxShape& shape, const PxActor& actor){	
		////是否接触到地面
		//if (actor.getName() != "Ground") {
		//	this->role->stopMoving();
		//}
		string name(actor.getName());
		if (name == "Door") {
			Door* door = (Door*)actor.userData;
			if (door->canOpen()) {
				PxRigidBody* doorActor = door->getDoorActor();
				float scale = 3000.0f;
				doorActor->addForce(role->getFaceDir() * scale);
			}
		}
		else if (name == "Seesaw") {
		}
		if (name == "over") {
			this->role->gameOver();
		}
		return PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT;
	}
	PxControllerBehaviorFlags getBehaviorFlags(const PxController& controller) {
		return PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT;
	}
	PxControllerBehaviorFlags getBehaviorFlags(const PxObstacle& obstacle) {
		return PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT;
	}
};