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
extern PxRigidActor* RayCast(PxVec3 origin, PxVec3 unitDir);

const int primaryJumpHeight = 6.0f;

class Role {
private:
	PxRigidBody* role;
	PxController* roleController;
	/// <summary>
	/// ��ɫ����
	/// </summary>
	PxF32 roleRadius = 1.0f;
	PxF32 roleHeight = 2.0f;
	//�����ٶ�
	PxVec3 speed = PxVec3(0, 0, 0);
	//���һ�ΰ��·�����ķ���
	PxVec3 lastPressDir = PxVec3(0, 0, 1);
	//�����������ǰ������
	PxVec3 dir = PxVec3(0, 0, 1);
	//�����泯����
	PxVec3 faceDir = PxVec3(0,0,1);
	//���ﵱǰλ��
	PxVec3 nowPostion;
	//������һ��λ��
	PxVec3 lastPostion;	
	//��ɫ����
	float mass = 6000.0f;
	//�������ٶ�
	float midFallSpeed = 0.2;
	//��Ծ���
	float littleJumpSpeed = 0.15;
	float bigJumpSpeed = 0.2;
	float nowJumpHeight = 0.0;
	float wantJumpHeight = primaryJumpHeight;
	float maxJumpHeight = 12.0;

	/// <summary>
	/// ״̬��
	/// </summary>
	bool isAutoMoving = false;
	bool canMove = true;
	bool isJump = false;
	bool isFall = false;
	bool isAlive = true;
	bool equiped = false;

public:
	Role();
	~Role() {
		this->role->release();
		this->roleController->release();
	};

	bool getAliveStatus();
	void gameOver();

	//��ɫλ����Ϣ
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

	//����վ���ķ������
	Block standingBlock;

	//��������
	void setEquiped(bool equip = true);
	bool getEquiped();

	//��Ծ
	void tryJump(bool release);
	void roleJump();
	void roleFall();
	void fall();

	//�¶�
	void roleCrouch();
	void roleNoCrouch();

	//�Ƿ�����ƶ�����
	void changeCanMove(bool);

	//ģ������
	void simulationGravity();
	//��������
	void pickUpObj();
	//��������
	void layDownObj();
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
* @brief ��ɫ��ײ�ж������ڽ�ɫײ�����������
**/
class RoleHitBehaviorCallback :public PxControllerBehaviorCallback {
private:
	Role* role = NULL;
public:
	RoleHitBehaviorCallback(Role* role) :role(role) {};
	PxControllerBehaviorFlags getBehaviorFlags(const PxShape& shape, const PxActor& actor){	
		////�Ƿ�Ӵ�������
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