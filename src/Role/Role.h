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

	PxF32 roleRadius = 1.0f;
	PxF32 roleHeight = 2.0f;

	PxVec3 speed = PxVec3(0, 0, 0);
	PxVec3 nowPostion;	//��ɫ��Ҫ�ﵽ��λ��
	PxVec3 lastPostion;	//��ɫ֮ǰ���ڵ�λ��
	bool isMoving = false; //�Ƿ�Ϊ�Զ��ƶ�

	bool isJump = false; //��Ծ״̬
	float littleJumpSpeed = 0.06; //��Ծ�ٶ�
	float bigJumpSpeed = 0.08;
	float nowJumpHeight = 0.0; //���ڵ���Ծ�߶�
	float wantJumpHeight = 0.0; //Ŀ����Ծ�߶�
	float maxJumpHeight = 15.0; //�����Ծ�߶�

	bool isFall = false; //����״̬
	float midFallSpeed = 0.08; //�����ٶ�

public:
	Role();
	~Role() {
		this->role->release();
		this->roleController->release();
	};


	PxVec3 getFootPosition();
	void updatePosition();

	//�ٶ����
	PxVec3 getSpeed();
	void setSpeed(PxVec3 speed);

	//�ƶ����
	void roleMoveByMouse(int x, int y);
	void move();
	void move(GLint key, bool status);
	bool getMovingStatus();
	void stopMoving();

	//��Ծ����
	void tryJump(bool release);
	void roleJump();
	void roleFall();

	//�¶�
	void roleCrouch();
	void roleNoCrouch();
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