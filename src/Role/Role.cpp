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
	roleController->setFootPosition(PxExtendedVec3(0, commonBoxHeight + desc.contactOffset +10, 20));
	roleController->resize(desc.height + desc.radius);
	roleController->setContactOffset(0.001f);

	this->role = roleController->getActor();
	this->roleController->setUserData(this);
	this->nowPostion = this->role->getGlobalPose().p;
	this->lastPostion = this->role->getGlobalPose().p;
}

/**
* @brief ��ȡ��ɫ�Ƿ��Զ��ƶ�
**/
bool Role::getMovingStatus() {
	return this->isMoving;
}

/**
* @brief ������ƶ���ȡĿ������
* @Param x �����Ļ����x y�����Ļ����y
**/
void Role::roleMoveByMouse(int x, int y) {
	if (this->isMoving || this->isJump || this->isFall || !this->isAlive) return;
	PxVec3 nowPosition = ScenetoWorld(x, y);
	this->lastPostion = PxVec3(this->nowPostion.x,this->nowPostion.y,this->nowPostion.z);
	this->nowPostion = nowPosition;
	this->isMoving = true;
}

/**
* @brief ������ƶ���ȡĿ������
* @Param position ��ά�ռ�����
**/
void Role::roleMoveByMouse(PxVec3 position) {
	if (this->isMoving || this->isJump || this->isFall || !this->isAlive) return;
	this->lastPostion = PxVec3(this->nowPostion.x, this->nowPostion.y, this->nowPostion.z);
	this->nowPostion = position;
	this->isMoving = true;
}

/**
* @brief ��ɫ�Զ��ƶ�
**/
void Role::move() {
	if(!this->isMoving || !this->isAlive) return;
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
* @brief ��ɫֹͣ�Զ��ƶ�
**/
void Role::stopMoving() {
	this->isMoving = false;
	this->nowPostion = PxVec3(this->roleController->getFootPosition().x, this->roleController->getFootPosition().y, this->roleController->getFootPosition().z);
}

/**
* @brief ����������ƽ�ɫ�ƶ�
* @Param key�������ⰴ�� status����(T)/����(F)
**/
void Role::move(GLint key,bool status) {
	if (!this->isAlive) {
		this->setSpeed(PxVec3(0, 0, 0));
		return;
	}
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
		this->roleController->move(this->speed * 4, 0.0001, 1.0f / 120.0f, NULL);
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
* @brief ��ȡ��ɫcontroller�ĵײ�����
* @Return PxVec3 
**/
PxVec3 Role::getFootPosition() {
	PxExtendedVec3 pos = this->roleController->getFootPosition();
	return PxVec3(pos.x, pos.y, pos.z);
}

/**
* @brief ���ý�ɫcontroller�ĵײ�����
* @Param position ��ά�ռ������
**/
void Role::setFootPosition(PxVec3 position) {
	this->roleController->setFootPosition(PxExtendedVec3(position.x,position.y,position.z));
}

/**
* @brief ����ͬ����ɫ������Ϣ
**/
void Role::updatePosition() {
	PxExtendedVec3 position = this->roleController->getFootPosition();
	this->lastPostion = PxVec3(this->nowPostion.x, this->nowPostion.y, this->nowPostion.z);
	this->nowPostion = PxVec3(position.x, position.y, position.z);
}

/**
* @brief ��ɫ��Ծ�����ж�
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
* @brief ��ɫ��Ծ
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
			wantJumpHeight = 6.0;
			isJump = false;
			isFall = true;
		}
		//this->updatePosition();
	}
}

/**
* @brief ��ɫ����
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
* @brief ��ɫ�¶�
**/
void Role::roleCrouch() {
	if (!isJump && !isFall) {
		this->roleController->resize(roleHeight/2.5);
	}
}

/**
* @brief ��ɫ����ģ��
**/
void Role::fall() {
	if (!isJump) {
		this->isFall = true;
	}
	
}


/**
* @brief ��ɫ�¶׻ָ��׶�
**/
void Role::roleNoCrouch() {
	this->roleController->resize(roleHeight+roleRadius);
}

/**
* @brief ��ɫ�����ٶ�
**/
PxVec3 Role::getSpeed() {
	return this->speed;
}

/**
* @brief ��ɫ��ȡ�ٶ�
**/
void Role::setSpeed(PxVec3 speed) {
	 this->speed = speed;
}

/**
* @brief ��ɫ�Ƿ���
**/
bool Role::getRoleStatus() {
	return this->isAlive;
}
/**
* @brief ��ɫ����
**/
void Role::gameOver() {
	this->isAlive = false;
}