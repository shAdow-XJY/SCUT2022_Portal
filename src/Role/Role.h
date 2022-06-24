#ifndef __ROLE_H__
#define __ROLE_H__

#include <ctype.h>
#include <iostream>
#include "PxPhysicsAPI.h"
#include "../Sphere/Pendulum.h"
#include "../Block/Door.h"
#include "../LoadModel/Model.h"
#include "../Block/Seesaw.h"
#include "../Sound/SoundTools.h"
#include "../Block/PrismaticRoad.h"
#include "../Block/RotateRod.h"
#include <glut.h>
using namespace physx;

extern PxScene* gScene;
extern PxMaterial* gMaterial;
extern PxPhysics* gPhysics;
extern PxControllerManager* cManager;
extern PxVec3 ScenetoWorld(int xCord, int yCord);
extern PxRigidActor* RayCast(PxVec3 origin, PxVec3 unitDir);
const float primaryUpSpeed = 0.10;

extern SoundTool soundtool;
extern vector<PxVec3> checkpoints;
static GameSceneBasic* errorGameSceneBasic = new GameSceneBasic();


class Role {
private:

	Model* model;
	PxRigidBody* role;
	PxController* roleController;
	/// <summary>
	/// ��ɫ����
	/// </summary>
	PxF32 roleRadius = 1.5f;
	PxF32 roleHeight = 10.0f;
	//�����ٶ�
	PxVec3 speed = PxVec3(0, 0, 0);
	//������Ծ�����ٶ�
	PxVec3 inertiaSpeed = PxVec3(0, 0, 0);
	//���һ�ΰ��·�����ķ���
	PxVec3 lastPressDir = PxVec3(0, 0, 1);
	//�����������ǰ������
	PxVec3 dir = PxVec3(0, 0, 1);
	//�����泯����
	PxVec3 faceDir = PxVec3(0, 0, 1);
	//���ﵱǰλ��
	PxVec3 nowPostion;
	//������һ��λ��
	PxVec3 lastPostion;
	//��ɫ����
	float mass = 10000.0f;
	//��Ծ���
	float upSpeed = primaryUpSpeed;
	float maxUpSpeed = 0.15;
	float gravityAcceleration = 0.0004f;

	/// <summary>
	/// ״̬��
	/// </summary>
	bool canMove = true;
	bool isJump = false;
	bool isHanging = false;
	bool isFall = false;
	bool isAlive = true;
	bool equiped = false;
	bool standingOnBlock = true;
	bool isRebirthing = false;
	bool isCrouch = false;
	//���滬��
	bool slide = false;
	//��Ե����
	PxVec3 sliceDir = PxVec3(0, 0, 0);

	//��ײģ��
	PxRigidBody* stimulateObj = NULL;
	PxReal stimulateMassScale = 0.00001f;

	//��ǰ�ѵ������Զ����
	int arrivedCheckpoint = 1;
	//����ֵ
	int life = 5;
	//�÷�
	int score = 0;

	//�ӽ��Ƿ����泯�����ƶ�
	bool rotateMoveDir = true;

	// �Ƿ��Ѱ󶨾�̬ģ��
	bool staticAttached = false;

public:
	Role();
	~Role() {
		this->role->release();
		this->roleController->release();
	};

	bool attachModel(const char*);

	Model& getModel() const {
		return *this->model;
	}
	PxShape* getShape() const {
		PxShape* shapePtr = nullptr;
		this->roleController->getActor()->getShapes(&shapePtr, 1);
		return shapePtr;
	}
	PxRigidActor* getActor() const {
		return this->role;
	}
	bool getAliveStatus();
	bool roleOver();

	//��ɫλ����Ϣ
	void setFootPosition(PxVec3 position);
	PxVec3 getFootPosition();
	PxVec3 getPosition() const;
	void updatePosition();
	PxVec3 getRoleWorldPosition();

	//�ٶ����
	PxVec3 getSpeed();
	void setSpeed(PxVec3 speed);

	//�������
	PxVec3 getDir();
	//��ɫ����
	PxVec3 getFaceDir();

	//��ɫ�ƶ����
	void move();
	void move(GLint key, bool status, bool free);

	//����վ���ķ������
	GameSceneBasic* standingBlock = errorGameSceneBasic;

	//��������
	void setEquiped(bool equip = true);
	bool getEquiped();

	//��Ծ
	bool tryJump(bool release);
	void roleJump();
	void roleFall();
	void fall();


	//�¶�
	bool getCrouch();
	void roleCrouch();
	void roleNoCrouch();

	//�Ƿ�����ƶ�����
	void changeCanMove(bool);

	//ģ������
	void simulationGravity();
	//��������
	bool pickUpObj();
	//��������
	bool layDownObj();

	//��ɫ����
	void roleSlide();
	void edgeSliding();
	//���﷢����������
	void rayAround();
	//����ģ�⶯̬���幦��
	void stimulate();

	bool isStaticAttached() {
		return staticAttached;
	}
	//����
	void protal();
	//���½�ɫ�÷�
	void updateScore();
	bool getRebirthing();
	bool getRotateOrNot();
	void setRotateOrNot(bool);

	//�ع�����
	PxVec3 roleHandleKey(GLint key, bool free);
	void touchGround();
	PxVec3 getHorizontalVelocity();
	void resetStatus();
	
	int nowCheckpoint = 1;
	int lastCheckpoint = 1;
	bool isJumping();
	void setDir(PxVec3 dir);
	void setFaceDir(PxVec3 dir);
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
	PxControllerBehaviorFlags getBehaviorFlags(const PxShape& shape, const PxActor& actor) {
		////�Ƿ�Ӵ�������
		//if (actor.getName() != "Ground") {
		//	this->role->stopMoving();
		//}
		if (actor.getName()) {
			string name(actor.getName());
			if (name == "Door") {
				Door* door = (Door*)actor.userData;
				float scale = 9000.0f;
				door->addPForce(role->getFaceDir() * scale);

				if (door->canOpen()) {
					if (!door->getDoorStauts()) {
						soundtool.playSound("openDoorSlowly.wav", true);
						door->setDoorStatus(true);
					}
				}
			}
			else if (name == "Over") {
				role->roleOver();
			}
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

#endif // !__ROLE_H__
