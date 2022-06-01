#include "Role.h"
#include "cmath"
#include <iostream>


Role::Role() {
	PxCapsuleControllerDesc desc;
	desc.radius = 1.0f;
	desc.height = 2.0f;
	desc.material = gMaterial;
	desc.climbingMode = PxCapsuleClimbingMode::eEASY;
	desc.upDirection = PxVec3(0.0, 1.0, 0.0);
	//设置碰撞回调函数
	RoleHitBehaviorCallback* bCallBack = new RoleHitBehaviorCallback(this);
	desc.behaviorCallback = bCallBack;

	roleController = cManager->createController(desc);
	roleController->setFootPosition(PxExtendedVec3(0, desc.contactOffset, 0));
	roleController->resize(desc.height + desc.radius);	

	this->role = roleController->getActor();
	this->roleController->setUserData(this);
	this->nowPostion = this->role->getGlobalPose().p;
	this->lastPostion = this->role->getGlobalPose().p;
}

/**
* @brief 判断角色是否正在移动
* @return bool 是否正在移动的标识
**/
bool Role::getMovingStatus() {
	return this->isMoving;
}

/**
* @brief 鼠标输入移动函数
* @param x 屏幕坐标x值 y 屏幕坐标y值
**/
void Role::roleMoveByMouse(int x, int y) {
	if (this->isMoving || this->isJump || this->isFall) return;
	PxVec3 nowPosition = ScenetoWorld(x, y);
	this->lastPostion = PxVec3(this->nowPostion.x,this->nowPostion.y,this->nowPostion.z);
	this->nowPostion = nowPosition;
	this->isMoving = true;
}

/**
* @brief 角色鼠标移动核心函数
**/
void Role::move() {
	if(!this->isMoving) return;
	PxExtendedVec3 position = this->roleController->getFootPosition();
	float offsetX = this->nowPostion.x - position.x;
	float offsetZ = this->nowPostion.z - position.z;
	std::cout << offsetX << std::endl;
	std::cout << offsetZ << std::endl;
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
* @brief 停止角色自动移动
**/
void Role::stopMoving() {
	this->isMoving = false;
	this->nowPostion = PxVec3(this->roleController->getFootPosition().x, this->roleController->getFootPosition().y, this->roleController->getFootPosition().z);
}

/**
* @brief 键盘输入控制角色
* @param key 按键 status true为按下，false为抬起
**/
void Role::move(GLint key,bool status) {
	this->isMoving = false;
	if (status) {
		PxVec3 dir = this->speed.getNormalized();
		switch (key) {
		case GLUT_KEY_UP: {
			dir = PxVec3(0, 0, 1);
			break;
		}
		case GLUT_KEY_DOWN: {
			dir = PxVec3(0, 0, -1);
			break;

		}case GLUT_KEY_LEFT: {
			dir = PxVec3(1, 0, 0);
			break;

		}case GLUT_KEY_RIGHT: {
			dir = PxVec3(-1, 0, 0);
			break;

		}
		default: {
			return;
		}
		}
		this->speed = dir * 0.12f;
		if (this->isJump || this->isFall) return;
		this->roleController->move(this->speed * 4, 0.0001, 1.0f / 60.0f, NULL);
		this->updatePosition();
	}
	else
	{
		if (!this->isJump && !this->isFall) {
			this->speed = PxVec3(0, 0, 0);
		}
		else
		{
			this->speed = this->speed * 0.5f;
		}
		
	}
}


/**
* @brief 获取底部位置
* @Retrun PxVec3 
**/
PxVec3 Role::getFootPosition() {
	PxExtendedVec3 pos = this->roleController->getFootPosition();
	return PxVec3(pos.x, pos.y, pos.z);
}


/**
* @brief 更新函数位置
**/
void Role::updatePosition() {
	PxExtendedVec3 position = this->roleController->getFootPosition();
	this->lastPostion = PxVec3(this->nowPostion.x, this->nowPostion.y, this->nowPostion.z);
	this->nowPostion = PxVec3(position.x, position.y, position.z);
}

/**
* @brief 判断跳跃条件
**/
void Role::tryJump(bool release) {
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
* @brief 角色跳跃上升阶段
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
		PxControllerCollisionFlags flag = roleController->move(PxVec3(0.0, speed, 0.0) + this->speed * 0.2, PxF32(0.001), 1.0f / 60.0f, NULL);
		nowJumpHeight += speed;
		std::cout << "wantJumpHeight" << wantJumpHeight << std::endl;
		std::cout << "nowJumpHeight" << nowJumpHeight << std::endl;
		if (nowJumpHeight >= wantJumpHeight)
		{
			std::cout << "max height" << std::endl;
			std::cout << "wantJumpHeight" << wantJumpHeight << std::endl;
			std::cout << "nowJumpHeight" << nowJumpHeight << std::endl;

			nowJumpHeight = 0.0;
			wantJumpHeight = 0.0;
			isJump = false;
			isFall = true;
		}
		//this->updatePosition();
	}
}

/**
* @brief 角色下降阶段
**/
void Role::roleFall() {
	if (isFall) {
		PxControllerCollisionFlags flag = roleController->move(PxVec3(0.0, -midFallSpeed, 0.0) + this->speed * 0.2, PxF32(0.00001), 1.0f / 60.0f, NULL);		
		if (flag == PxControllerCollisionFlag::eCOLLISION_DOWN) {
			isFall = false;
			this->speed = PxVec3(0,0,0);
			if (!this->isMoving) {
				this->updatePosition();
			};
		}
		
	}
}


/**
* @brief 获取速度
* @return PxVec3
**/
PxVec3 Role::getSpeed() {
	return this->speed;
}

/**
* @brief 设置速度
* @Param speed
**/
void Role::setSpeed(PxVec3 speed) {
	 this->speed = speed;
}