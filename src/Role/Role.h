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
extern void renderGameOver();
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
	/// 角色属性
	/// </summary>
	PxF32 roleRadius = 1.5f;
	PxF32 roleHeight = 10.0f;
	//人物速度
	PxVec3 speed = PxVec3(0, 0, 0);
	//人物跳跃惯性速度
	PxVec3 inertiaSpeed = PxVec3(0, 0, 0);
	//最后一次按下方向键的方向
	PxVec3 lastPressDir = PxVec3(0, 0, 1);
	//自由相机人物前进方向
	PxVec3 dir = PxVec3(0, 0, 1);
	//人物面朝方向
	PxVec3 faceDir = PxVec3(0, 0, 1);
	//人物当前位置
	PxVec3 nowPostion;
	//人物上一次位置
	PxVec3 lastPostion;
	//角色重力
	float mass = 10000.0f;
	//跳跃相关
	float upSpeed = primaryUpSpeed;
	float maxUpSpeed = 0.15;
	float gravityAcceleration = 0.0004f;

	/// <summary>
	/// 状态量
	/// </summary>
	bool canMove = true;
	bool isJump = false;
	bool isHanging = false;
	bool isFall = false;
	bool isAlive = true;
	bool equiped = false;
	bool standingOnBlock = true;
	bool isRebirthing = false;
	//冰面滑动
	bool slide = false;
	//边缘滑动
	PxVec3 sliceDir = PxVec3(0, 0, 0);

	//碰撞模拟
	PxRigidBody* stimulateObj = NULL;
	PxReal stimulateMassScale = 0.00001f;

	//当前已到达的最远检查点
	int arrivedCheckpoint = 1;
	//生命值
	int life = 5;
	//得分
	int score = 0;


	// 是否已绑定静态模型
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
	bool gameOver();

	//角色位置信息
	void setFootPosition(PxVec3 position);
	PxVec3 getFootPosition();
	PxVec3 getPosition() const;
	void updatePosition();
	PxVec3 getRoleWorldPosition();

	//速度相关
	PxVec3 getSpeed();
	bool isSpeedZero();
	void setSpeed(PxVec3 speed);

	//相机朝向
	PxVec3 getDir();
	//角色朝向
	PxVec3 getFaceDir();

	//角色移动相关
	void move();
	void move(GLint key, bool status, bool free);
	void stopMoving();

	//人物站立的方块基类
	GameSceneBasic* standingBlock = errorGameSceneBasic;

	//放置物体
	void setEquiped(bool equip = true);
	bool getEquiped();

	//跳跃
	bool tryJump(bool release);
	void roleJump();
	void roleFall();
	void fall();


	//下蹲
	void roleCrouch();
	void roleNoCrouch();

	//是否可以移动人物
	void changeCanMove(bool);

	//模拟重力
	void simulationGravity();
	//捡起物体
	bool pickUpObj();
	//放置物体
	bool layDownObj();

	//角色滑动
	void roleSlide();
	void edgeSliding();
	//人物发射四周射线
	void rayAround();
	//人物模拟动态刚体功能
	void stimulate();

	bool isStaticAttached() {
		return staticAttached;
	}
	//传送
	void protal();
	//更新角色得分
	void updateScore();
	bool getRebirthing();


	//重构部分
	PxVec3 roleHandleKey(GLint key, bool free);
	void touchGround();
	PxVec3 getHorizontalVelocity();
	void resetStatus();


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
	PxControllerBehaviorFlags getBehaviorFlags(const PxShape& shape, const PxActor& actor) {
		////是否接触到地面
		//if (actor.getName() != "Ground") {
		//	this->role->stopMoving();
		//}
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
			if (this->role->gameOver()) {
				const char* msg = "游戏结束";
				//渲染游戏结束
				renderGameOver();
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
