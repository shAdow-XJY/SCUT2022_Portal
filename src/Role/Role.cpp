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
	//������ײ�ص�����
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
* @brief �жϽ�ɫ�Ƿ������ƶ�
* @return bool �Ƿ������ƶ��ı�ʶ
**/
bool Role::getMovingStatus() {
	return this->isMoving;
}

/**
* @brief ��������ƶ�����
* @param x ��Ļ����xֵ y ��Ļ����yֵ
**/
void Role::roleMoveByMouse(int x, int y) {
	if (this->isMoving || this->isJump || this->isFall) return;
	PxVec3 nowPosition = ScenetoWorld(x, y);
	//std::cout << nowPosition.x << " " << nowPosition.y << std::endl;
	this->lastPostion = PxVec3(this->nowPostion.x,this->nowPostion.y,this->nowPostion.z);
	this->nowPostion = nowPosition;
	this->isMoving = true;
}

/**
* @brief ��ɫ����ƶ����ĺ���
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
* @brief ֹͣ��ɫ�Զ��ƶ�
**/
void Role::stopMoving() {
	this->isMoving = false;
	this->nowPostion = PxVec3(this->roleController->getFootPosition().x, this->roleController->getFootPosition().y, this->roleController->getFootPosition().z);
}

/**
* @brief ����������ƽ�ɫ
**/
void Role::move(GLint key) {
	this->isMoving = false;
	PxVec3 dir = PxVec3(0,0,0);
	switch (key) {
		case GLUT_KEY_UP:{
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
	}
	this->speed = dir * 0.3f;
	if (this->isJump || this->isFall) return;
	this->roleController->move(this->speed, 0.0001,1.0f/60.0f, NULL);
	this->updatePosition();
}


/**
* @brief ��ȡ�ײ�λ��
* @Retrun PxVec3 
**/
PxVec3 Role::getFootPosition() {
	PxExtendedVec3 pos = this->roleController->getFootPosition();
	return PxVec3(pos.x, pos.y, pos.z);
}

void Role::updatePosition() {
	PxExtendedVec3 position = this->roleController->getFootPosition();
	this->lastPostion = PxVec3(this->nowPostion.x, this->nowPostion.y, this->nowPostion.z);
	this->nowPostion = PxVec3(position.x, position.y, position.z);
}

/**
* @brief �ж���Ծ����
**/
void Role::tryJump() {
	if (!isJump && !isFall) {
		isJump = true;
	}
}

/**
* @brief ��ɫ��Ծ�����׶�
**/
void Role::roleJump() {
	if (isJump) {
		float speed = 0.0;
		if (nowJumpHeight <= maxJumpHeight / 2) {
			speed = bigJumpSpeed ;
		}
		else
		{
			speed = littleJumpSpeed;
		}

		std::cout << this->speed.x << " " << this->speed.z << std::endl;
		PxControllerCollisionFlags flag = roleController->move(PxVec3(0.0, speed, 0.0) + this->speed * 0.4, PxF32(0.001), 1.0f / 60.0f, NULL);
		nowJumpHeight += speed;
		std::cout << "maxJumpHeight" << maxJumpHeight << std::endl;
		std::cout << "nowJumpHeight" << nowJumpHeight << std::endl;
		if (nowJumpHeight >= maxJumpHeight)
		{
			std::cout << "max height" << std::endl;
			std::cout << "maxJumpHeight" << maxJumpHeight << std::endl;
			std::cout << "nowJumpHeight" << nowJumpHeight << std::endl;

			nowJumpHeight = 0.0;
			isJump = false;
			isFall = true;
		}
		//this->updatePosition();
	}
}

/**
* @brief ��ɫ�½��׶�
**/
void Role::roleFall() {
	if (isFall) {
		PxControllerCollisionFlags flag = roleController->move(PxVec3(0.0, -midFallSpeed, 0.0) + this->speed * 0.4, PxF32(0.00001), 1.0f / 60.0f, NULL);		
		if (flag == PxControllerCollisionFlag::eCOLLISION_DOWN) {
			isFall = false;
			this->speed = PxVec3(0,0,0);
			this->updatePosition();
		}
		
	}
}