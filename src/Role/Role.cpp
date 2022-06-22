#include "Role.h"
#include "cmath"
#include <iostream>

#define MAX_NUM_ACTOR_SHAPES 128


Role::Role() {
	PxCapsuleControllerDesc desc;
	desc.radius = roleRadius;
	desc.height = roleHeight;
	desc.material = gMaterial;
	desc.climbingMode = PxCapsuleClimbingMode::eCONSTRAINED;
	desc.stepOffset = 0.5f;
	desc.contactOffset = 0.1;
	desc.upDirection = PxVec3(0.0, 1.0, 0.0);
	

	RoleHitBehaviorCallback* bCallBack = new RoleHitBehaviorCallback(this);
	desc.behaviorCallback = bCallBack;

	roleController = cManager->createController(desc);
	roleController->setFootPosition(PxExtendedVec3(0, boxHeight + desc.contactOffset +10, 20));
	roleController->resize(desc.height + desc.radius);
	roleController->setContactOffset(0.001f);
	this->role = roleController->getActor();
	this->roleController->setUserData(this);
	this->nowPostion = this->role->getGlobalPose().p;
	this->lastPostion = this->role->getGlobalPose().p;
	this->role->setName("Role");
}

/**
* @brief 将角色与模型绑定
* @param path 模型文件路径
**/
bool Role::attachModel(const char* path) {
	PxShape* cap;
	role->getShapes(&cap, 1);
	// 设为false就能只作为碰撞体而不渲染出来
	cap->setFlag(PxShapeFlag::eVISUALIZATION, false);
	this->model = new Model(path);
	this->model->attachMeshes(PxTransform(PxQuat(-PxHalfPi, PxVec3(0.0f, 0.0f, 1.0f))).transform(PxTransform(PxVec3(0.0f,-0.4f,0.0f))), this->role);
	this->role->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
	
	return true;
}


/**
* @brief 获取角色是否自动移动
**/
bool Role::getMovingStatus() {
	return this->isAutoMoving;
}

/**
* @brief 鼠标点击移动获取目标坐标
* @param x 鼠标屏幕坐标x 
* @param y 鼠标屏幕坐标y
**/
void Role::roleMoveByMouse(int x, int y) {
	if (this->isAutoMoving || this->isJump || this->isFall || !this->isAlive) return;
	PxVec3 nowPosition = ScenetoWorld(x, y);
	this->lastPostion = PxVec3(this->nowPostion.x,this->nowPostion.y,this->nowPostion.z);
	this->nowPostion = nowPosition;
	this->isAutoMoving = true;
}

/**
* @brief 鼠标点击移动获取目标坐标
* @param position 三维空间坐标
**/
void Role::roleMoveByMouse(PxVec3 position) {
	if (this->isAutoMoving || this->isJump || this->isFall || !this->isAlive || !this->canMove) return;
	this->lastPostion = PxVec3(this->nowPostion.x, this->nowPostion.y, this->nowPostion.z);
	this->nowPostion = position;
	this->isAutoMoving = true;
}

/**
* @brief 角色自动移动
**/
void Role::move() {
	if(!this->isAutoMoving || !this->isAlive || !this->canMove) return;
	PxExtendedVec3 position = this->roleController->getFootPosition();
	float offsetX = this->nowPostion.x - position.x;
	float offsetZ = this->nowPostion.z - position.z;
	if (abs(offsetX) <= 0.01f && abs(offsetZ) <= 0.01f) {
		this->roleController->setFootPosition(PxExtendedVec3(this->nowPostion.x, this->nowPostion.y, this->nowPostion.z));
		this->isAutoMoving = false;
		return;
	}

	this->isAutoMoving = true;
	float speed = 0.05f;
	if (abs(offsetX) > 0.05f) {
		speed = offsetX > 0 ? speed : -speed;
		PxVec3 dis(speed, 0, 0);
		this->roleController->move(dis, 0.000001f, 1.0f / 60.0f, PxControllerFilters());
		return;
	}
	else if (abs(offsetZ) > 0.05f)
	{
		speed = offsetZ > 0 ? speed : -speed;
		PxVec3 dis(0, 0, speed);
		this->roleController->move(dis, 0.000001f, 1.0f / 60.0f, PxControllerFilters());
		return;
	}
	else
	{
		this->isAutoMoving = false;
		this->roleController->setFootPosition(PxExtendedVec3(this->nowPostion.x, this->nowPostion.y, this->nowPostion.z));
	}
}

/**
* @brief 角色停止自动移动
**/
void Role::stopMoving() {
	this->isAutoMoving = false;
	this->nowPostion = PxVec3(this->roleController->getFootPosition().x, this->roleController->getFootPosition().y, this->roleController->getFootPosition().z);
}

/**
* @brief 键盘输入控制角色移动
* @desc	 锁定视角以角色面朝方向为前进方向，自由视角以摄像机朝向为前进方向
* @param key	输入特殊按键 
* @param status 按下(T)/弹起(F)
* @param free	相机是否自由移动
**/
void Role::move(GLint key, bool status, bool free) {
	if (!this->canMove) {
		return;
	}
	if (!this->isAlive) {
		this->setSpeed(PxVec3(0, 0, 0));
		return;
	}
	//按下
	if (status) {
		PxVec3 dir;
		if (!free) dir = this->faceDir; //非自由镜头以人物朝向为前进方向
		else dir = this->dir; //自由镜头以摄像机正前方为前进方向
		//移动方向计算
		switch (key) {
		case GLUT_KEY_UP: {
			//dir = PxVec3(0, 0, 1);
			break;
		}
		case GLUT_KEY_DOWN: {
			dir *= -1;
			break;

		}case GLUT_KEY_LEFT: {
			PxTransform rotate = PxTransform(PxQuat(PxHalfPi, PxVec3(0, 1, 0)));
			dir = rotate.rotate(dir);
			break;

		}case GLUT_KEY_RIGHT: {
			PxTransform rotate = PxTransform(PxQuat(PxHalfPi, PxVec3(0, 1, 0)));
			dir = rotate.rotate(-dir);
			break;

		}
		default: {
			return;
		}
		}
		this->slide = false;
		this->speed = dir * 0.6f;
		if (standingBlock->getType() == OrganType::iceroad) {
			this->speed = dir * 0.86f;
		}
		this->lastPressDir = dir.getNormalized();
		if (this->isJump || this->isFall) return;
		PxVec3 lastPosition = this->getFootPosition();
		this->roleController->move(this->speed, 0.0001, 1.0f / 120.0f, NULL);
		//this->updatePosition();
		//更新距离
		if (standingBlock->getType() == OrganType::prismaticRoad) {
			PrismaticRoad* primaticRoad = (PrismaticRoad*)standingBlock;
			PxVec3 pos = primaticRoad->getStaticPosition()-(lastPosition - this->getFootPosition()) ;
			primaticRoad->updateDistance(pos);
		}
	}
	//弹起
	else
	{
		if (!this->isJump && !this->isFall) {
			this->faceDir = this->lastPressDir; //更新为最后一次移动的面朝方向
			if (!free) {
				this->dir = this->faceDir;//抬起的时候才更新角色朝向，确保持续移动
			}
			if (standingBlock->getType() == OrganType::iceroad) {
				std::cout << "in the ice" << std::endl;
				//this->setSpeed(this->speed);
				this->slide = true;
			}
			else {
				std::cout << "SET ZERO" << std::endl;
				this->speed = PxVec3(0, 0, 0);
			}
		}
		else
		{
			//空中惯性的保持
			this->speed = this->speed * 0.5f;
		}

	}
}

/**
* @brief 获取角色controller的底部坐标
* @return PxVec3 
**/
PxVec3 Role::getFootPosition() {
	PxExtendedVec3 pos = this->roleController->getFootPosition();
	return PxVec3(pos.x, pos.y, pos.z);
}

/**
* @brief 获取角色controller的中心坐标
* @return PxVec3
**/
PxVec3 Role::getPosition() {
	PxExtendedVec3 pos = this->roleController->getPosition();
	return PxVec3(pos.x, pos.y, pos.z);
}

/**
* @brief 设置角色controller的底部坐标
* @param position 三维空间坐标点
**/
void Role::setFootPosition(PxVec3 position) {
	this->roleController->setFootPosition(PxExtendedVec3(position.x,position.y,position.z));
}

/**
* @brief （自动移动）更新同步角色坐标信息
**/
void Role::updatePosition() {
	PxExtendedVec3 position = this->roleController->getFootPosition();
	this->lastPostion = PxVec3(this->nowPostion.x, this->nowPostion.y, this->nowPostion.z);
	this->nowPostion = PxVec3(position.x, position.y, position.z);
}

/**
* @brief 获取角色世界坐标信息
**/
PxVec3 Role::getRoleWorldPosition() {
	return this->role->getGlobalPose().p;
}

/**
* @brief 角色跳跃条件判断
**/
bool Role::tryJump(bool release) {
	if (!this->isAlive) return false;
	if (!isJump && !isFall) {
		if (!release) {
			//std::cout << "wantJumpHeight" << wantJumpHeight << std::endl;
			wantJumpHeight = wantJumpHeight <= maxJumpHeight ? (wantJumpHeight + bigJumpSpeed*5) : maxJumpHeight;
		}
		else
		{
			isJump = true;
		}
		return true;
	}
	return false;
}
extern clock_t deltaClock;
/**
* @brief 角色跳跃
**/
void Role::roleJump() {
	if (isJump) {
		float speed = 0.0;
		if (nowJumpHeight <= wantJumpHeight / 2) {
			speed = bigJumpSpeed * 0.5 ;
		}
		else
		{
			speed = littleJumpSpeed * 0.5;
		}
		PxVec3 jumpSpeed = PxVec3(0.0, speed, 0.0);
		if (canForward && canMove) {
			jumpSpeed += this->speed * 0.3;
		}
		
		PxControllerCollisionFlags flag = roleController->move(jumpSpeed, PxF32(0.001), deltaClock, NULL);
		nowJumpHeight += speed;
		//std::cout << "wantJumpHeight" << wantJumpHeight << std::endl;
		//std::cout << "nowJumpHeight" << nowJumpHeight << std::endl;
		if (nowJumpHeight >= wantJumpHeight)
		{
			/*std::cout << "max height" << std::endl;
			std::cout << "wantJumpHeight" << wantJumpHeight << std::endl;
			std::cout << "nowJumpHeight" << nowJumpHeight << std::endl;*/

			nowJumpHeight = 0.0;
			wantJumpHeight = primaryJumpHeight;
			isJump = false;
			isFall = true;
		}
		//this->updatePosition();
	}
}

/**
* @brief 角色掉落
**/
void Role::roleFall() {
	if (isFall) {
		PxVec3 fallSpeed = PxVec3(0.0, -midFallSpeed, 0.0);
		if (canForward && canMove) {
			fallSpeed += this->speed * 0.3;
		}
		PxControllerCollisionFlags flag = roleController->move(fallSpeed, PxF32(0.00001), deltaClock, NULL);
		if (flag == PxControllerCollisionFlag::eCOLLISION_SIDES) {
			this->setSpeed(PxVec3(0, 0, 0));
		}
		else if (flag == PxControllerCollisionFlag::eCOLLISION_DOWN) {
			this->setSpeed(PxVec3(0, 0, 0));
			isFall = false;
			if (!this->isAutoMoving) {
				//this->updatePosition();
			};
		}
		
	}
}

/**
* @brief 角色冰面滑动函数
* @return void
**/
void Role::roleSlide() {
	if (this->speed.isZero()) {
		this->slide = false;
	}
	else if (slide) {
		//std::cout << this->speed.x << "  " <<  this->speed.y << "  " << this->speed.z << std::endl;
		this->setSpeed(this->speed * 0.96f);
		std::cout << "  " << this->speed.abs().x << "  " << this->speed.abs().y << "  " << this->speed.abs().z << std::endl;
		//std::cout << "  " << this->speed.abs().minElement() << "  " << this->speed.abs().minElement() << "  " << this->speed.abs().minElement() << std::endl;
		if (this->speed.abs().x > 0.001f || this->speed.abs().y > 0.001f || this->speed.abs().z > 0.001f) {
			PxControllerCollisionFlags flag = this->roleController->move(this->speed, 0.0001, 1.0f / 120.0f, NULL);
			if (flag == PxControllerCollisionFlag::eCOLLISION_SIDES) {
				this->setSpeed(PxVec3(0, 0, 0));
			}
		}
		else
		{
			this->speed = PxVec3(0, 0, 0);
		}

	}
}

/**
* @brief 角色下蹲
**/
void Role::roleCrouch() {
	if (!isJump && !isFall) {
		this->roleController->resize(roleHeight/2.5);
	}
}

/**
* @brief 角色重力模拟
**/
void Role::fall() {
	if (!isJump) {
		this->isFall = true;
	}
	
}

/**
* @brief 角色下蹲恢复阶段
**/
void Role::roleNoCrouch() {
	this->roleController->resize(roleHeight+roleRadius);
}

/**
* @brief 角色获得道具
**/
void Role::setEquiped(bool equip) {
	this->equiped = equip;
}

/**
* @brief 获得角色道具状态
**/
bool Role::getEquiped() {
	return this->equiped;
}

/**
* @brief 角色设置速度
**/
PxVec3 Role::getSpeed() {
	return this->speed;
}

/**
* @brief 自由视角角色前进方向
**/
PxVec3 Role::getDir() {
	return this->dir;
}

/**
* @brief 获取角色面朝的方向
**/
PxVec3 Role::getFaceDir() {
	PxVec3 dir = this->speed.getNormalized();
	if (isSpeedZero()) return this->faceDir;
	return dir;
}

/**
* @brief 角色获取速度
**/
void Role::setSpeed(PxVec3 speed) {
	 this->speed = speed;
}


/**
* @brief 角色是否存活
**/
bool Role::getAliveStatus() {
	return this->isAlive;
}
/**
* @brief 角色死亡
**/
bool Role::gameOver() {
	if (this->stimulateObj) {
		this->stimulateObj->release();
	}
	if (this->life > 0) {
		this->isRebirthing = true;
	}
	else {
		this->isAlive = false;
		return true;
	}	
	this->stimulateObj = NULL;
	return false;
	
}

/**
* @brief 角色是否可以移动
**/
void Role::changeCanMove(bool flag) {
	this->canMove = flag;
}

bool Role::isSpeedZero() {
	if (!this->speed.x && !this->speed.y && !this->speed.z) return true;
	return false;
}


void Role::edgeSliding() {
	if (this->standingBlock->getType() == OrganType::seesaw) {
		PxVec3 spliceSpeed = isSpeedZero() ? this->sliceDir : this->getFaceDir();
		this->setFootPosition(this->getFootPosition() + spliceSpeed * 2.0f);
	}
	else
	{
		this->setFootPosition(this->getFootPosition() + this->getFaceDir() * 3.0f); //边缘滑动
	}
}

/**
* @brief 角色底部发送射线
* @desc  用于角色模拟重力和给物体施加重力
**/
void Role::simulationGravity() {
	PxVec3 origin = this->getFootPosition();
	PxVec3 unitDir = PxVec3(0, -0.8f, 0);
	PxRigidActor* actor = NULL;
	if (actor = RayCast(origin, unitDir)) {
		//碰撞到物体
		//std::cout << "碰到地面" << std::endl;
		//cout << role->standingBlock.getName() << endl;
		this->standingOnBlock = true;
		GameSceneBasic* basic = (GameSceneBasic*)actor->userData;		
		this->sliceDir = PxVec3(0, 0, 0);
		if (basic != NULL) {
			//cout << basic->getType() << endl;
			if (basic->getType() == OrganType::road) {
				//std::cout << role->standingBlock.getName()<<std::endl;
			}
			else if (basic->getType() == OrganType::seesaw) {
				Seesaw* seesaw = (Seesaw*)basic;
				PxVec3 force = PxVec3(0, -1, 0) * this->mass;
				PxVec3 speed = seesaw->addGForce(this->getFootPosition(), force);
				this->sliceDir = speed.getNormalized();
				if (!this->isJump && !this->isFall) {
					this->roleController->move(speed + PxVec3(0, -0.3, 0), 0.0001, 1.0f / 120.0f, NULL);
				}
				
			}
			else if (basic->getType() == OrganType::prismaticRoad) {
				PrismaticRoad* prismaticRoad = (PrismaticRoad*)basic;
				if (!this->isJump && !this->isFall) {
					this->setFootPosition(prismaticRoad->getStaticPosition());
				}
				else
				{
					prismaticRoad->resetDistance();
				}
				prismaticRoad->updateDistance(this->getFootPosition());
			
			}
			this->standingBlock = basic;
		}
	}
	else {
		if (!this->isJump && !this->isFall) {
			if (this->standingBlock->getType() != OrganType::error) {
				//再次检测避免出现更新延迟
				if (!RayCast(origin, PxVec3(0, -5.0f, 0))) {
					std::cout << "边缘滑动" << endl;
					this->edgeSliding();
				}
			}
			this->standingBlock = errorGameSceneBasic;
			this->standingOnBlock = false;
			this->fall();
		}	
		
	}
}


/**
* @brief 角色道具拾取
**/
void Role::pickUpObj() {
	//cout << this->faceDir.x<<" " << this->faceDir.y <<" "<< this->faceDir.z<<" " << endl;
	PxVec3 origin = this->getPosition() - PxVec3(0,0.2f,0);
	//确定role的前方方向
	PxVec3 forwardDir = this->getFaceDir() * 2;
	PxRigidActor* actor = NULL;
	if (actor = RayCast(origin, forwardDir)) {
		GameSceneBasic* basic = (GameSceneBasic*)actor->userData;
		if (basic->getType() == OrganType::prop) {
			actor->release();
			this->equiped = true;
			std::cout << "拾取道具成功" << std::endl;
		}
		else
		{
			std::cout << "不是道具" << std::endl;
		}
	}
	else
	{
		std::cout << "射线没有找到目标" << std::endl;
	}
}

/**
* @brief 角色道具放置
**/
void Role::layDownObj() {
	PxVec3 origin = this->getPosition() -PxVec3(0, 0.2f, 0);
	//确定role的前方方向
	PxVec3 forwardDir = PxVec3(this->getFaceDir().x * 1.5f, -3, this->getFaceDir().z * 1.5f);
	PxRigidActor* actor = NULL;
	if (actor = RayCast(origin, forwardDir)) {
		GameSceneBasic* basic = (GameSceneBasic*)actor->userData;
		if (basic->getType() == OrganType::road) {
			this->equiped = false;
			extern void createPorp(const PxTransform & t, const PxVec3 & v, PxReal x, PxReal y, PxReal z);
			//cout << role->getPosition().x << " " << role->getPosition().y << " " << role->getPosition().z << endl;
			//cout << role->getFaceDir().x << " " << role->getFaceDir().y << " " << role->getFaceDir().z << endl;
			createPorp(PxTransform(PxVec3(0, 0, 0)), this->getPosition() + this->getFaceDir() * 2.5, boxHeight, boxHeight, boxHeight);
			std::cout << "放置道具成功" << std::endl;
		}
		else
		{
			std::cout << "不是可放置道具的地方" << std::endl;
		}
	}
	else
	{
		std::cout << "射线没有找到目标" << std::endl;
	}
}


//向四周发送射线
void Role::rayAround() {
	PxVec3 origin = this->getPosition() - PxVec3(0,-0.2f,0);
	PxRigidActor* actor = NULL;
	//向四周发送射线
	for (int i = -1; i < 2; i++) {
		for (int j = -1; j < 2; j++) {
			 if (i == 0 && j == 0) continue;
			 PxVec3 dir = PxVec3(i, 0, j).getNormalized() * 2.0f;
			 actor = RayCast(origin, dir);
			 if (actor) {				 
				 GameSceneBasic* gsb = (GameSceneBasic*)actor->userData;
				 if (gsb) {
					 //摆锤
					 if (gsb->getType() == OrganType::pendulum) {
						 //cout << "撞到了" << endl;
						 Pendulum* pendulem = (Pendulum*)gsb;
						 int flag = pendulem->getPendulumActor()->getAngularVelocity().x > 0 ? 1 : -1;
						 if (!this->stimulateObj) {
							 PxShape* shape = gPhysics->createShape(PxCapsuleGeometry(0.05, 0.5), *gMaterial);
							 //偏移值为测试计算出来，该值的准确值有待商榷
							 PxVec3 pos = this->getPosition() + PxVec3(0, 0, 2) * flag;
							 PxRigidDynamic* dynamic = gPhysics->createRigidDynamic(PxTransform(pos));
							 dynamic->attachShape(*shape);
							 dynamic->setName("");
							 PxRigidBodyExt::updateMassAndInertia(*dynamic, this->stimulateMassScale);
							 gScene->addActor(*dynamic);
							 this->stimulateObj = dynamic;
						 }
						 return;

					 }
					 else if (gsb->getType() == OrganType::rotateRod) {
						 RotateRod* rotateRod = (RotateRod*)gsb;
						 this->roleController->move(PxVec3(0, 1.5f, 0), 0.0001, 1.0f / 120.0f, NULL);
						 return;
					 }
				 }
			}
		}
	}
}

//模拟
void Role::stimulate() {
	if (this->stimulateObj) {
		const PxVec3 pos = this->stimulateObj->getGlobalPose().p;
		this->roleController->setPosition(PxExtendedVec3(pos.x,pos.y,pos.z));
	}
}

//
void Role::protal() {
	if (this->isRebirthing) {
		srand((int)time(0));
		int protalCheckpoint = rand() % this->arrivedCheckpoint;
		this->setFootPosition(checkpoints[protalCheckpoint]);
		this->life--;
		this->isRebirthing = false;
	}
}


void Role::updateScore() {
	PxVec3 origin = this->getFootPosition();
	PxVec3 unitDir = PxVec3(0, -99.0f, 0);
	PxRigidActor* actor = NULL;
	if (actor = RayCast(origin, unitDir)) {
		GameSceneBasic* basic = (GameSceneBasic*)actor->userData;
		int checkpoint = basic->getCheckpoint();
		if (this->arrivedCheckpoint < checkpoint) {
			this->arrivedCheckpoint = checkpoint;
			this->score += 100;
		}
	}
}

bool Role::getRebirthing() {
	return this->isRebirthing;
}