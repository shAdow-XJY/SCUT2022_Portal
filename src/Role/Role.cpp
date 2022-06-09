#include "Role.h"
#include "cmath"
#include <iostream>


Role::Role() {
	PxCapsuleControllerDesc desc;
	desc.radius = roleRadius;
	desc.height = roleHeight;
	desc.material = gMaterial;
	//desc.climbingMode = PxCapsuleClimbingMode::eCONSTRAINED;
	desc.stepOffset = 0.0f;
	desc.contactOffset = 0.001;
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
}

/**
* @brief 获取角色是否自动移动
**/
bool Role::getMovingStatus() {
	return this->isMoving;
}

/**
* @brief 鼠标点击移动获取目标坐标
* @Param x 鼠标屏幕坐标x y鼠标屏幕坐标y
**/
void Role::roleMoveByMouse(int x, int y) {
	if (this->isMoving || this->isJump || this->isFall || !this->isAlive) return;
	PxVec3 nowPosition = ScenetoWorld(x, y);
	this->lastPostion = PxVec3(this->nowPostion.x,this->nowPostion.y,this->nowPostion.z);
	this->nowPostion = nowPosition;
	this->isMoving = true;
}

/**
* @brief 鼠标点击移动获取目标坐标
* @Param position 三维空间坐标
**/
void Role::roleMoveByMouse(PxVec3 position) {
	if (this->isMoving || this->isJump || this->isFall || !this->isAlive || !this->canMove) return;
	this->lastPostion = PxVec3(this->nowPostion.x, this->nowPostion.y, this->nowPostion.z);
	this->nowPostion = position;
	this->isMoving = true;
}

/**
* @brief 角色自动移动
**/
void Role::move() {
	if(!this->isMoving || !this->isAlive || !this->canMove) return;
	PxExtendedVec3 position = this->roleController->getFootPosition();
	float offsetX = this->nowPostion.x - position.x;
	float offsetZ = this->nowPostion.z - position.z;
	if (abs(offsetX) <= 0.01f && abs(offsetZ) <= 0.01f) {
		this->roleController->setFootPosition(PxExtendedVec3(this->nowPostion.x, this->nowPostion.y, this->nowPostion.z));
		this->isMoving = false;
		return;
	}

	this->isMoving = true;
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
		this->isMoving = false;
		this->roleController->setFootPosition(PxExtendedVec3(this->nowPostion.x, this->nowPostion.y, this->nowPostion.z));
	}
}

/**
* @brief 角色停止自动移动
**/
void Role::stopMoving() {
	this->isMoving = false;
	this->nowPostion = PxVec3(this->roleController->getFootPosition().x, this->roleController->getFootPosition().y, this->roleController->getFootPosition().z);
}

/**
* @brief 键盘输入控制角色移动
* @desc	 锁定视角以角色面朝方向为前进方向，自由视角以摄像机朝向为前进方向
* @Param key输入特殊按键 status按下(T)/弹起(F) free相机是否自由移动
**/
void Role::move(GLint key,bool status,bool free) {
	if (!this->canMove) {
		return;
	}
	if (!this->isAlive) {
		this->setSpeed(PxVec3(0, 0, 0));
		return;
	}
	this->isMoving = false;
	//按下
	if (status) {
		PxVec3 dir;
		if (!free) dir = this->faceDir; //非自由镜头以人物朝向为前进方向
		else dir = this->dir; //自由镜头以摄像机正前方为前进方向
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
		this->speed = dir * 0.12f;
		this->lastPressDir = dir.getNormalized();
		if (this->isJump || this->isFall) return;
		this->roleController->move(this->speed * 4, 0.0001, 1.0f / 120.0f, NULL);
		this->updatePosition();
	}
	//弹起
	else
	{
		if (!this->isJump && !this->isFall) {
			this->faceDir = this->lastPressDir; //获取最后一次移动的面朝方向
			if (!free) {
				this->dir = this->faceDir;//抬起的时候才更新角色朝向，确保持续移动
			}
			this->speed = PxVec3(0, 0, 0);

		}
		else
		{
			this->speed = this->speed * 0.5f;
		}
		
		
	}
}


/**
* @brief 获取角色controller的底部坐标
* @Return PxVec3 
**/
PxVec3 Role::getFootPosition() {
	PxExtendedVec3 pos = this->roleController->getFootPosition();
	return PxVec3(pos.x, pos.y, pos.z);
}

/**
* @brief 获取角色controller的中心坐标
* @Return PxVec3
**/
PxVec3 Role::getPosition() {
	PxExtendedVec3 pos = this->roleController->getPosition();
	return PxVec3(pos.x, pos.y, pos.z);
}

/**
* @brief 设置角色controller的底部坐标
* @Param position 三维空间坐标点
**/
void Role::setFootPosition(PxVec3 position) {
	this->roleController->setFootPosition(PxExtendedVec3(position.x,position.y,position.z));
}

/**
* @brief 更新同步角色坐标信息
**/
void Role::updatePosition() {
	PxExtendedVec3 position = this->roleController->getFootPosition();
	this->lastPostion = PxVec3(this->nowPostion.x, this->nowPostion.y, this->nowPostion.z);
	this->nowPostion = PxVec3(position.x, position.y, position.z);
}

/**
* @brief 角色跳跃条件判断
**/
void Role::tryJump(bool release) {
	if (!this->isAlive) return;
	if (!isJump && !isFall) {
		if (!release) {
			std::cout << "wantJumpHeight" << wantJumpHeight << std::endl;
			wantJumpHeight = wantJumpHeight <= maxJumpHeight ? (wantJumpHeight + bigJumpSpeed*5) : maxJumpHeight;
		}
		else
		{
			isJump = true;
		}
	}
}

/**
* @brief 角色跳跃
**/
void Role::roleJump() {
	if (isJump) {
		float speed = 0.0;
		if (nowJumpHeight <= wantJumpHeight / 2) {
			speed = bigJumpSpeed ;
		}
		else
		{
			speed = littleJumpSpeed;
		}
		PxControllerCollisionFlags flag = roleController->move(PxVec3(0.0, speed, 0.0) + this->speed * 0.3, PxF32(0.001), 1.0f / 60.0f, NULL);
		nowJumpHeight += speed;
		std::cout << "wantJumpHeight" << wantJumpHeight << std::endl;
		std::cout << "nowJumpHeight" << nowJumpHeight << std::endl;
		if (nowJumpHeight >= wantJumpHeight)
		{
			std::cout << "max height" << std::endl;
			std::cout << "wantJumpHeight" << wantJumpHeight << std::endl;
			std::cout << "nowJumpHeight" << nowJumpHeight << std::endl;

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
		PxControllerCollisionFlags flag = roleController->move(PxVec3(0.0, -midFallSpeed, 0.0) + this->speed * 0.3, PxF32(0.00001), 1.0f / 60.0f, NULL);		
		if (flag == PxControllerCollisionFlag::eCOLLISION_DOWN) {
			isFall = false;
			if (!this->isMoving) {
				this->updatePosition();
			};
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
	if (!this->speed.x || !this->speed.y || !this->speed.z) return this->faceDir;
	return this->speed.getNormalized();
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
bool Role::getRoleStatus() {
	return this->isAlive;
}
/**
* @brief 角色死亡
**/
void Role::gameOver() {
	this->isAlive = false;
}

/**
* @brief 角色是否存活
**/
void Role::changeCanMove(bool flag) {
	this->canMove = flag;
}