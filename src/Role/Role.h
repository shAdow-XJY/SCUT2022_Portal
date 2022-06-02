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
	PxVec3 speed = PxVec3(0, 0, 0);
	PxVec3 nowPostion;	//��ɫ��Ҫ�ﵽ��λ��
	PxVec3 lastPostion;	//��ɫ֮ǰ���ڵ�λ��
	bool isMoving = false; //�Ƿ�Ϊ�Զ��ƶ�

	bool isJump = false;
	float littleJumpSpeed = 0.06;
	float bigJumpSpeed = 0.08;
	float nowJumpHeight = 0.0;
	float maxJumpHeight = 15.0;

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
	void updatePosition();

	//�ٶ����
	PxVec3 getSpeed();
	void setSpeed(PxVec3 speed);

	//�ƶ����
	void roleMoveByMouse(int x, int y);
	void roleMoveByMouse(PxVec3 position);
	void move();
	void move(GLint key, bool status);
	bool getMovingStatus();
	void stopMoving();

	Block standingBlock;

	void tryJump();
	void roleJump();
	void roleFall();
	void fall();
};

class RoleHitCallback :public PxUserControllerHitReport {
public:
	void onShapeHit(const PxControllerShapeHit& hit) {
		std::cout << "��ײ����" << hit.actor->getName() << std::endl;
	}
	void onControllerHit(const PxControllersHit& hit) {
		std::cout << "��ײ����" << std::endl;
	}
	void  onObstacleHit(const PxControllerObstacleHit& hit) {
		std::cout << "��ײ����" << std::endl;
	}
};

class RoleHitBehaviorCallback :public PxControllerBehaviorCallback {
private:
	Role* role = NULL;
public:
	RoleHitBehaviorCallback(Role* role) :role(role) {};
	PxControllerBehaviorFlags getBehaviorFlags(const PxShape& shape, const PxActor& actor){	
		//�ж���ײ����������ΪGround����ֹͣ�ƶ�
		if (actor.getName() != "Ground") {
			std::cout << "ײ���ˣ�ֹͣ�ƶ�" << std::endl;
			this->role->stopMoving();
		}
		return PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT;
	}
	PxControllerBehaviorFlags getBehaviorFlags(const PxController& controller) {
		std::cout << "��ײ������controller" <<std::endl;
		return PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT;
	}
	PxControllerBehaviorFlags  getBehaviorFlags(const PxObstacle& obstacle) {
		std::cout << "��ײ���û�������ϰ���" << std::endl;
		return PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT;
	}
};