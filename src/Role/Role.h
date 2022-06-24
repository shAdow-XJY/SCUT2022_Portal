#ifndef __ROLE_H__
#define __ROLE_H__

#include <ctype.h>
#include <iostream>
#include "PxPhysicsAPI.h"
#include "../Sphere/Pendulum.h"
#include "../Block/Door.h"
#include "../LoadModel/Model.h"
#include "../Block/Seesaw.h"
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
	PxF32 roleRadius = 3.5f;
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
	bool isCrouch = false;
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

	//视角是否随面朝方向移动
	bool rotateMoveDir = true;

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
	bool roleOver();

	//角色位置信息
	void setFootPosition(PxVec3 position);
	PxVec3 getFootPosition();
	PxVec3 getPosition() const;
	void updatePosition();
	PxVec3 getRoleWorldPosition();

	//速度相关
	PxVec3 getSpeed();
	void setSpeed(PxVec3 speed);

	//相机朝向
	PxVec3 getDir();
	//角色朝向
	PxVec3 getFaceDir();

	//角色移动相关
	void move();
	void move(GLint key, bool status, bool free);

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
	bool getCrouch();
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
	bool getRotateOrNot();
	void setRotateOrNot(bool);

	// 获取游戏信息
	int getCheckpoint() {
		return this->arrivedCheckpoint;
	}
	int getHealth() {
		return this->life;
	}
	int getScore() {
		return this->score;
	}


	//重构部分
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

#endif // !__ROLE_H__
