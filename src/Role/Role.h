#include <ctype.h>
#include <iostream>
#include "PxPhysicsAPI.h"
#include "../Block/Door.h"
#include "../LoadModel/Model.h"
#include "../Block/Seesaw.h"
#include <glut.h>
using namespace physx;

extern PxScene* gScene;
extern PxMaterial* gMaterial;
extern PxPhysics* gPhysics;
extern PxControllerManager* cManager;
extern PxVec3 ScenetoWorld(int xCord, int yCord);
extern PxRigidActor* RayCast(PxVec3 origin, PxVec3 unitDir);

const int primaryJumpHeight = 4.0f;

class Role {
private:
	PxRigidBody* role;
	PxController* roleController;
	Model* model;

	/// <summary>
	/// 角色属性
	/// </summary>
	PxF32 roleRadius = 1.0f;
	PxF32 roleHeight = 2.0f;
	//人物速度
	PxVec3 speed = PxVec3(0, 0, 0);
	//最后一次按下方向键的方向
	PxVec3 lastPressDir = PxVec3(0, 0, 1);
	//自由相机人物前进方向
	PxVec3 dir = PxVec3(0, 0, 1);
	//人物面朝方向
	PxVec3 faceDir = PxVec3(0,0,1);
	//人物当前位置
	PxVec3 nowPostion;
	//人物上一次位置
	PxVec3 lastPostion;	
	//角色重力
	float mass = 8000.0f;
	//重力加速度
	float midFallSpeed = 0.5;
	//跳跃相关
	float littleJumpSpeed = 0.4;
	float bigJumpSpeed = 0.5;
	float nowJumpHeight = 0.0;
	float wantJumpHeight = primaryJumpHeight;
	float maxJumpHeight = 10.0;

	/// <summary>
	/// 状态量
	/// </summary>
	bool isAutoMoving = false;
	bool canMove = true;
	bool isJump = false;
	bool isFall = false;
	bool isAlive = true;
	bool equiped = false;
	//本次跳跃是否能够向前
	bool canForward = true;
	bool standingOnBlock = true;
	//冰面滑动
	bool slide = false;
	//边缘滑动
	PxVec3 sliceDir = PxVec3(0, 0, 0);

public:
	Role();
	~Role() {
		this->role->release();
		this->roleController->release();
	};

	bool attachModel(const char*);
	bool getAliveStatus();
	void gameOver();

	//角色位置信息
	void setFootPosition(PxVec3 position);
	PxVec3 getFootPosition();
	PxVec3 getPosition();
	void updatePosition();

	//速度相关
	PxVec3 getSpeed();
	bool isSpeedZero();
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

	//人物站立的方块基类
	Block standingBlock;

	//放置物体
	void setEquiped(bool equip = true);
	bool getEquiped();

	//跳跃
	void tryJump(bool release);
	void roleJump();
	void roleFall();
	void fall();
	void changeForward(bool);

	//下蹲
	void roleCrouch();
	void roleNoCrouch();

	//是否可以移动人物
	void changeCanMove(bool);

	//模拟重力
	void simulationGravity();
	//捡起物体
	void pickUpObj();
	//放置物体
	void layDownObj();

	//角色滑动
	void roleSlide();
	void edgeSliding();

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
				float scale = 9000.0f;
				doorActor->addForce(role->getFaceDir() * scale);
				PxRevoluteJoint* revoluteJoint = door->getJoint();
				PxJointAngularLimitPair limits(-PxPi / 2, PxPi / 2, 0.01f);
				revoluteJoint->setLimit(limits);
			}
		}
		else if (name == "Seesaw") {
		}
		else if (name == "over") {
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