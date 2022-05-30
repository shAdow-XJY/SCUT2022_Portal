#include "Role.h"
#include "cmath"
#include <iostream>


/**
* @brief ��������ƶ�����
* @param x ��Ļ����xֵ y ��Ļ����yֵ
**/
void Role::roleMoveByMouse(int x, int y) {
	if (this->isMoving) return;
	PxVec3 nowPosition = ScenetoWorld(x, y);
	this->lastPostion = PxVec3(this->nowPostion.x,this->nowPostion.y,this->nowPostion.z);
	this->nowPostion = nowPosition;
	
}

/**
* @brief ��ɫ�����ƶ�����
**/
void Role::move() {
	PxVec3 position = this->role->getGlobalPose().p;
	float offsetX = this->nowPostion.x - position.x;
	float offsetZ = this->nowPostion.z - position.z;
	if (abs(offsetX) <= 0.01f && abs(offsetZ) <= 0.01f) {
		this->roleController->setFootPosition(PxExtendedVec3(this->nowPostion.x, this->nowPostion.y, this->nowPostion.z));
		return;
	}
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
		this->roleController->setFootPosition(PxExtendedVec3(this->nowPostion.x, this->nowPostion.y, this->nowPostion.z));
	}
}