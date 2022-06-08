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

const int primaryJumpHeight = 6.0f;

class Role {
private:
	PxRigidBody* role;
	PxController* roleController;

	PxF32 roleRadius = 1.0f;
	PxF32 roleHeight = 2.0f;
	//�ٶȷ���
	PxVec3 speed = PxVec3(0, 0, 0);
	//���һ�ΰ��·�����ķ���
	PxVec3 lastPressDir = PxVec3(0, 0, 1);
	//���������
	PxVec3 dir = PxVec3(0, 0, 1);
	//�����泯����
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
	//�������ٶ�
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
	PxVec3 getPosition();
	void updatePosition();
	//�ٶ����
	PxVec3 getSpeed();
	void setSpeed(PxVec3 speed);

	//�������
	PxVec3 getDir();
	//��ɫ����
	PxVec3 getFaceDir();
	//��ɫ�ƶ����
	void roleMoveByMouse(int x, int y);
	void roleMoveByMouse(PxVec3 position);
	void move();
	void move(GLint key, bool status,bool free);
	bool getMovingStatus();
	void stopMoving();

	Block standingBlock;
	Block propBlock;
	void tryJump(bool release);
	void roleJump();
	void roleFall();
	void fall();

	//�¶�
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

class RoleHitBehaviorCallback :public PxControllerBehaviorCallback {
private:
	Role* role = NULL;
public:
	RoleHitBehaviorCallback(Role* role) :role(role) {};
	PxControllerBehaviorFlags getBehaviorFlags(const PxShape& shape, const PxActor& actor){	
		//�Ƿ�Ӵ�������
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