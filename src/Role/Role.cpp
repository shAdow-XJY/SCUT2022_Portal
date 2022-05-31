#include "Role.h"
#include "cmath"
#include <iostream>

Role::Role() {
	PxCapsuleControllerDesc desc;
	desc.radius = 1.0f;
	desc.height = 2.0f;
	desc.material = gMaterial;
	desc.climbingMode = PxCapsuleClimbingMode::eCONSTRAINED;

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
	if (this->isMoving) return;
	PxVec3 nowPosition = ScenetoWorld(x, y);
	this->lastPostion = PxVec3(this->nowPostion.x,this->nowPostion.y,this->nowPostion.z);
	this->nowPostion = nowPosition;
	
}

/**
* @brief 角色鼠标移动核心函数
**/
void Role::move() {
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
* @brief 停止角色自动移动
**/
void Role::stopMoving() {
	this->isMoving = false;
	this->nowPostion = PxVec3(this->roleController->getFootPosition().x, this->roleController->getFootPosition().y, this->roleController->getFootPosition().z);
}

/**
* @brief 键盘输入控制角色
**/
void Role::move(char key) {
	PxVec3 dir = PxVec3(0,0,0);
	switch (toupper(key)) {
		case 'W':{
			dir = PxVec3(0, 0, 1);
			break;
		}
		case 'S': {
			dir = PxVec3(0, 0, -1);
			break;

		}case 'A': {
			dir = PxVec3(1, 0, 0);
			break;

		}case 'D': {
			dir = PxVec3(-1, 0, 0);
			break;

		}
	}
	float speed = 0.5f;
	this->roleController->move(dir * speed, 0.0001, 1.0f / 60.0f, PxControllerFilters());
	PxExtendedVec3 position = this->roleController->getFootPosition();
	this->lastPostion = PxVec3(this->nowPostion.x, this->nowPostion.y, this->nowPostion.z);
	this->nowPostion = PxVec3(position.x, position.y, position.z);
}


/**
* @brief 获取底部位置
* @Retrun PxVec3 
**/
PxVec3 Role::getFootPosition() {
	PxExtendedVec3 pos = this->roleController->getFootPosition();
	return PxVec3(pos.x, pos.y, pos.z);
}