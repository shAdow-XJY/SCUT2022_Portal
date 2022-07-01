#include "Role.h"
#include "cmath"
#include <iostream>
#include <Role/RoleHitCallback.h>
extern void printPxVecFun(const PxVec3& vec);
extern clock_t deltaClock;

#define MAX_NUM_ACTOR_SHAPES 128

Role::Role() {
	
	desc.radius = roleRadius;
	desc.height = roleHeight;
	desc.material = gMaterial;
	desc.climbingMode = PxCapsuleClimbingMode::eCONSTRAINED;
	desc.stepOffset = 2.0f; //该offset可以兼容爬楼梯不卡住和风车正常
	desc.contactOffset = 0.1;
	desc.upDirection = PxVec3(0.0, 1.0, 0.0);


	RoleHitBehaviorCallback* bCallBack = new RoleHitBehaviorCallback(this);
	desc.behaviorCallback = bCallBack;

	roleController = cManager->createController(desc);
	roleController->setFootPosition(PxExtendedVec3(0, boxHeight + desc.contactOffset + 10, 20));
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
	//this->model->attachMeshes(PxTransform(PxQuat(-PxHalfPi, PxVec3(0.0f, 0.0f, 1.0f))).transform(PxTransform(PxVec3(0.0f,-0.4f,0.0f))), this->role);
	//this->role->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
	return true;
}


/**
* @brief 计算角色的移动方向
**/
PxVec3 Role::roleHandleKey(GLint key, bool free) {

	PxVec3 dir;
	if (!free && this->rotateMoveDir) dir = this->faceDir; //非自由镜头以人物朝向为前进方向
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
		return PxVec3(0, 0, 0);
	}
	}
	return dir;
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
PxVec3 Role::getPosition() const {
	PxExtendedVec3 pos = this->roleController->getPosition();
	return PxVec3(pos.x, pos.y, pos.z);
}

/**
* @brief 设置角色controller的底部坐标
* @param position 三维空间坐标点
**/
void Role::setFootPosition(PxVec3 position) {
	this->roleController->setFootPosition(PxExtendedVec3(position.x, position.y, position.z));
}

/**
* @brief （自动移动）更新同步角色坐标信息
**/
void Role::updatePosition() {
	PxExtendedVec3 position = this->roleController->getFootPosition();
	//this->lastPostion = PxVec3(this->nowPostion.x, this->nowPostion.y, this->nowPostion.z);
	this->nowPostion = PxVec3(position.x, position.y, position.z);
}

/**
* @brief 获取角色世界坐标信息
**/
PxVec3 Role::getRoleWorldPosition() {
	return this->role->getGlobalPose().p;
}


/**
* @brief 角色冰面滑动函数
* @return void
**/
void Role::roleSlide() {
	if (this->inertiaSpeed.isZero()) {
		this->slide = false;
	}
	else if (slide) {
		//std::cout << this->speed.x << "  " <<  this->speed.y << "  " << this->speed.z << std::endl;
		this->inertiaSpeed *= 0.96f;
		printPxVecFun(this->inertiaSpeed);
		if (this->inertiaSpeed.abs().x > 0.001f || this->inertiaSpeed.abs().y > 0.001f || this->inertiaSpeed.abs().z > 0.001f) {
			PxControllerCollisionFlags flag = this->roleController->move(this->speed, 0.0001, 1.0f / 120.0f, NULL);
			if (flag == PxControllerCollisionFlag::eCOLLISION_SIDES) {
				this->inertiaSpeed = PxVec3(0, 0, 0);
			}
		}
		else
		{
			inertiaSpeed = PxVec3(0, 0, 0);
		}

	}
}

/**
* @brief 角色下蹲状态
**/
bool Role::getCrouch(){
	return this->isCrouch;
}

/**
* @brief 角色下蹲
**/
void Role::roleCrouch() {
	if (!isJump && !isFall) {
		this->isCrouch = true;
		this->roleController->resize(roleHeight / 5.0);
	}
}

/**
* @brief 角色下蹲恢复阶段
**/
void Role::roleNoCrouch() {
	this->roleController->resize(roleHeight + roleRadius);
	this->isCrouch = false;
}

/**
* @brief 角色下蹲时的高度
**/
float Role::getRoleHeight() {
	return this->roleHeight / 2.5 + this->roleRadius * 2;
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
	PxVec3 dir = this->lastPressDir.getNormalized();
	if (this->speed.isZero()) return this->faceDir;
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
bool Role::roleOver() {
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


/**
* @brief 角色跷跷板边缘滑动
**/
void Role::edgeSliding() {
	if (this->standingBlock->getType() == OrganType::seesaw) {
		PxVec3 spliceSpeed = this->speed.isZero() ? this->sliceDir : this->getFaceDir();
		this->setFootPosition(this->getFootPosition() + spliceSpeed * 3.0f);
	}
}


/**
* @brief 角色道具拾取
**/
bool Role::pickUpObj() {
	//cout << this->faceDir.x<<" " << this->faceDir.y <<" "<< this->faceDir.z<<" " << endl;
	PxVec3 origin = this->getFootPosition() + PxVec3(0, 0.5f, 0);
	//确定role的前方方向
	PxVec3 forwardDir = this->getFaceDir()*5;
	PxRigidActor* actor = NULL;
	if (actor = RayCast(origin, forwardDir)) {
		GameSceneBasic* basic = (GameSceneBasic*)actor->userData;
		if (basic->getType() == OrganType::prop) {
			actor->release();
			this->equiped = true;
			std::cout << "拾取道具成功" << std::endl;
			return true;
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
	return false;
}

/**
* @brief 角色道具放置
**/
bool Role::layDownObj() {
	PxVec3 origin = this->getPosition() - PxVec3(0, 0.2f, 0);
	//确定role的前方方向
	PxVec3 forwardDir = PxVec3(this->getFaceDir().x * 1.5f, -20.0f, this->getFaceDir().z * 1.5f);
	PxRigidActor* actor = NULL;
	if (actor = RayCast(origin, forwardDir)) {
		GameSceneBasic* basic = (GameSceneBasic*)actor->userData;
		if (basic->getType() == OrganType::road) {
			this->equiped = false;
			extern void createPorp(const PxTransform & t, const PxVec3 & v, PxReal x, PxReal y, PxReal z);
			//cout << role->getPosition().x << " " << role->getPosition().y << " " << role->getPosition().z << endl;
			//cout << role->getFaceDir().x << " " << role->getFaceDir().y << " " << role->getFaceDir().z << endl;
			createPorp(PxTransform(PxVec3(0, 0, 0)), this->getFootPosition() + PxVec3(this->getFaceDir().x * 3.5f, 1.0f, this->getFaceDir().z * 3.5f), 1.0, 1.0, 1.0);
			std::cout << "放置道具成功" << std::endl;
			return true;
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
	return false;
}

/**
* @brief 角色道具使用
**/
bool Role::useKeyObj() {
	PxVec3 origin = this->getPosition();
	//确定role的前方方向
	PxRigidActor* actor = NULL;
	if (actor = RayCast(origin, this->getFaceDir()*5.0f)) {
		GameSceneBasic* basic = (GameSceneBasic*)actor->userData;
		if (basic->getType() == OrganType::keyDoor) {
			Door* door = (Door*)actor->userData;;
			if (this->equiped) {
				if (door->getNeedKey()) {
					if (door->getHasKey()) {
						std::cout << "该门已被使用道具" << std::endl;
					}
					else {
						door->setHasKey(true);
						this->equiped = false;
						std::cout << "使用道具成功" << std::endl;
						this->keyDoorActor = actor;
						//door->setPosition();
						return true;
					}
				}
				else {
					cout << "不是可使用道具的门" << endl;
				}
			}
			else
			{
				std::cout << "角色没有装备道具" << std::endl;
			}
		}
		else
		{
			std::cout << "不是门" << std::endl;
		}
	}
	else
	{
		std::cout << "射线没有找到目标" << std::endl;
	}
	return false;
}


/**
* @brief 角色四周射线检测
**/
void Role::rayAround() {
	PxVec3 origin = nowPostion + PxVec3(0, 3.5f, 0);
	PxRigidActor* actor = NULL;
	//向四周发送射线
	for (float i = -1; i < 2; i+=0.5) {
		for (float j = -1; j < 2; j+=0.5) {
			if (i == 0 && j == 0) continue;
			//摆锤碰撞若检测不灵则调节此处即可！
			PxVec3 dir = PxVec3(i, 0, j).getNormalized() * 3.2f;
			actor = RayCast(origin, dir);		
			if (actor) {
				GameSceneBasic* gsb = (GameSceneBasic*)actor->userData;
				if (gsb) {
					//摆锤
					if (gsb->getType() == OrganType::pendulum) {
						//cout << "撞到了" << endl;
						Pendulum* pendulem = (Pendulum*)gsb;
						animation.setAnimation("roll");
						int flag = pendulem->getPendulumActor()->getAngularVelocity().x > 0 ? 1 : -1;
						if (!this->stimulateObj) {
							PxShape* shape = gPhysics->createShape(PxCapsuleGeometry(0.01, 0.1), *gMaterial);
							//偏移值为测试计算出来，该值的准确值有待商榷
							PxVec3 pos = this->getPosition() + PxVec3(0, 0, -2) * flag;
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
					else if (gsb->getType() == OrganType::poolWall) {
						this->nowCheckpoint = 8;
						return;
					}
					else if (gsb->getType() == OrganType::finalLine) {
						this->nowCheckpoint = 9;
						animation.setAnimation("dancing");
						return;
					}
				}
			}
		}
	}
}

/**
* @brief 角色进行刚体模拟
**/
void Role::stimulate() {
	if (this->stimulateObj) {
		this->canMove = false;
		const PxVec3 pos = this->stimulateObj->getGlobalPose().p;
		if (pos.y < 2.0f) {
			this->roleOver();
		}
		this->roleController->setPosition(PxExtendedVec3(pos.x, pos.y, pos.z));
		this->updatePosition();
	}
}

/**
* @brief 角色传送
**/
void Role::protal() {
	if (this->isRebirthing) {
		srand((int)time(0));
		int protalCheckpoint = rand() % this->arrivedCheckpoint;
		this->setFootPosition(checkpoints[protalCheckpoint]);
		//this->setFootPosition(checkpoints[6]);
		this->updatePosition();
		this->life--;
		this->resetStatus();
		this->isRebirthing = false;
	}
}


/**
* @brief 角色得分
**/
void Role::updateScore() {
	PxVec3 origin = this->getFootPosition();
	PxVec3 unitDir = PxVec3(0, -99.0f, 0);
	PxRigidActor* actor = NULL;
	if (actor = RayCast(origin, unitDir)) {
		GameSceneBasic* basic = (GameSceneBasic*)actor->userData;
		if (basic) {
			int checkpoint = basic->getCheckpoint();		
			if (this->arrivedCheckpoint < checkpoint) {
				this->arrivedCheckpoint = checkpoint;
				this->score += 100;
			}
			if (checkpoint != nowCheckpoint) {
				lastCheckpoint = nowCheckpoint;
			}
			nowCheckpoint = checkpoint;
		}
	}
}

/**
* @brief 角色是否处于重生状态
**/
bool Role::getRebirthing() {
	return this->isRebirthing;
}

/**
* @brief 角色统一移动函数
* @desc 每帧回调
**/
void Role::move() {
	if (!this->isAlive || !canMove) return;
	PxVec3 moveSpeed = PxVec3(0, 0, 0);
	if (!this->stimulateObj) {
		if (this->getHorizontalVelocity().isZero()) {
			moveSpeed += this->inertiaSpeed;
			moveSpeed += this->speed;
		}
		else
		{
			moveSpeed = this->speed;
		}
		PxControllerCollisionFlags flag = roleController->move(moveSpeed * deltaClock, PxF32(0.00001), deltaClock, NULL);
	}

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
		PxVec3 dir = this->roleHandleKey(key, free).getNormalized();
		this->slide = false;
		float speed_y = this->speed.y;
		this->speed = dir * 0.020f;
		this->speed.y = speed_y;
		if (standingBlock->getType() == OrganType::iceroad) {
			this->speed = dir * 0.03f;
		}
		this->lastPressDir = dir.getNormalized();
		PxVec3 lastPosition = this->getFootPosition();
		//更新距离
		if (standingBlock->getType() == OrganType::prismaticRoad) {
			PrismaticRoad* primaticRoad = (PrismaticRoad*)standingBlock;
			PxVec3 pos = primaticRoad->getStaticPosition() - (lastPosition - this->getFootPosition());
			primaticRoad->updateDistance(pos);
		}
	}
	//弹起
	else
	{
		this->faceDir = this->lastPressDir; //更新为最后一次移动的面朝方向
		if (!this->isJump && !this->isFall) {		
			if (!free && this->rotateMoveDir) {
				this->dir = this->faceDir;//抬起的时候才更新角色朝向，确保持续移动
			}
			if (standingBlock->getType() == OrganType::iceroad) {
				cout << "ice slide" << endl;
				this->slide = true;
				this->inertiaSpeed = PxVec3(this->speed.x, 0, this->speed.z);
			}
		}
		this->speed.x = 0;
		this->speed.z = 0;

	}
}

/**
* @brief 角色着地
**/
void Role::touchGround() {
	this->speed.y = 0;
	this->isFall = false;
	this->standingOnBlock = true;
	this->inertiaSpeed = PxVec3(0, 0, 0);
}

/**
* @brief 角色需要下落
**/
void Role::fall() {
	if (!isJump) {
		this->isFall = true;
	}

}

/**
* @brief 角色底部发送射线
* @desc  角色重力模拟并与底部物体进行模拟交互
**/
void Role::simulationGravity() {
	PxVec3 origin = this->getFootPosition();
	PxVec3 unitDir = PxVec3(0, -0.8f, 0);
	PxRigidActor* actor = NULL;
	if (actor = RayCast(origin, unitDir)) {
		if (isFall) {
			this->touchGround();
		}
		////死亡逻辑 跑不到这段代码
		//if (actor->getName()) {
		//	string name(actor->getName());
		//	if (name == "Over" || name == "Ground0") {
		//		this->canMove = false;
		//		cout << name << endl;
		//		if (this->isAlive) {
		//			animation.setAnimation("dying");
		//		}
		//		return;
		//	}
		//}

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
				prismaticRoad->updateDistance(this->getFootPosition());
				if (!this->isJump && !this->isFall) {
					this->setFootPosition(prismaticRoad->getStaticPosition());
					updatePosition();
				}
				else
				{
					prismaticRoad->resetDistance();
				}
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
* @brief 角色跳跃
**/
bool Role::tryJump(bool release) {
	if (!this->isAlive || !canMove) return false;
	if (!isJump && !isFall) {
		if (!release) {
			//蓄力跳
			float scale = 0.001;
			std::cout << "upSpeed" << upSpeed << std::endl;
			upSpeed = upSpeed <= maxUpSpeed ? (upSpeed + scale) : maxUpSpeed;

		}
		else
		{
			isJump = true;
			slide = false;
			if (!this->speed.isZero()) {
				this->inertiaSpeed = this->getHorizontalVelocity();
			}

		}
		return true;
	}
	return false;
}

/**
* @brief 角色跳跃上升
**/
void Role::roleJump() {
	if (isJump) {
		float speed_y = 0.0;
		if (isHanging == false) {
			speed_y = upSpeed;
			isHanging = true;
			upSpeed = primaryUpSpeed;
		}
		this->speed.y += speed_y;
		this->speed.y -= gravityAcceleration * deltaClock;
		if (this->speed.y <= 0.0) {
			isJump = false;
			isHanging = false;
			isFall = true;
			this->speed.y = 0.0f;
		}
	}
}

/**
* @brief 角色下落
**/
void Role::roleFall() {
	if (isFall) {
		auto speed = this->speed;
		this->speed.y -= gravityAcceleration * deltaClock;
	}
}



/**
* @brief 重置角色状态
**/
void Role::resetStatus() {
	if (this->stimulateObj) {
		this->stimulateObj->release();
		this->stimulateObj = NULL;
	}
	this->speed = PxVec3(0, 0, 0);
	this->inertiaSpeed = PxVec3(0, 0, 0);	
	this->slide = false;
	this->standingOnBlock = false;
	this->canMove = true;
	this->standingBlock = errorGameSceneBasic;

}

/**
* @brief 获取角色水平速度
**/
PxVec3 Role::getHorizontalVelocity() {
	return PxVec3(this->speed.x, 0, this->speed.z);
}


bool Role::getRotateOrNot() {
	return this->rotateMoveDir;
}

void Role::setRotateOrNot(bool flag) {
	this->rotateMoveDir = flag;
}

bool Role::isJumping() {
	return this->isJump || this->isFall;
}

void Role::setDir(PxVec3 dir) {
	this->dir = dir;
}

void Role::setFaceDir(PxVec3 dir){
	this->faceDir = dir;
}


bool Role::isOver()
{
	return !this->isAlive;
}
