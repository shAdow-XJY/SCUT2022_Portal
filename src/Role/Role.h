#include <ctype.h>
#include <iostream>
#include "PxPhysicsAPI.h"
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
	PxVec3 nowPostion;	//��ɫ��Ҫ�ﵽ��λ��
	PxVec3 lastPostion;	//��ɫ֮ǰ���ڵ�λ��
	bool isMoving = false;

	bool isJump = false;
	float littleJumpSpeed = 0.8;
	float bigJumpSpeed = 0.9;
	float nowJumpHeight = 0.0;
	float maxJumpHeight = 20.0;

	bool isFall = false;
	float midFallSpeed = 0.85;

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
	void move(GLint key);
	void stopMoving();

	void tryJump();
	void roleJump();
	void roleFall();
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