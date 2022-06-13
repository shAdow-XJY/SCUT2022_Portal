#include "Role.h"
#include "cmath"
#include <iostream>

#define MAX_NUM_ACTOR_SHAPES 128


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
* @brief ����ɫ��ģ�Ͱ�
* @param path ģ���ļ�·��
**/
bool Role::attachModel(const char* path) {
	PxShape* cap;
	role->getShapes(&cap, 1);
	// ��Ϊfalse����ֻ��Ϊ��ײ�������Ⱦ����
	cap->setFlag(PxShapeFlag::eVISUALIZATION, false);
	this->model = new Model(path);
	this->model->attachMeshes(PxTransform(PxQuat(-PxHalfPi, PxVec3(0.0f, 0.0f, 1.0f))).transform(PxTransform(PxVec3(0.0f,-0.4f,0.0f))), this->role);
	this->role->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
	
	return true;
}


/**
* @brief ��ȡ��ɫ�Ƿ��Զ��ƶ�
**/
bool Role::getMovingStatus() {
	return this->isMoving;
}

/**
* @brief ������ƶ���ȡĿ������
* @param x �����Ļ����x 
* @param y �����Ļ����y
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
* @param position ��ά�ռ�����
**/
void Role::roleMoveByMouse(PxVec3 position) {
	if (this->isMoving || this->isJump || this->isFall || !this->isAlive || !this->canMove) return;
	this->lastPostion = PxVec3(this->nowPostion.x, this->nowPostion.y, this->nowPostion.z);
	this->nowPostion = position;
	this->isMoving = true;
}

/**
* @brief ��ɫ�Զ��ƶ�
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
* @brief ��ɫֹͣ�Զ��ƶ�
**/
void Role::stopMoving() {
	this->isMoving = false;
	this->nowPostion = PxVec3(this->roleController->getFootPosition().x, this->roleController->getFootPosition().y, this->roleController->getFootPosition().z);
}

/**
* @brief ����������ƽ�ɫ�ƶ�
* @desc	 �����ӽ��Խ�ɫ�泯����Ϊǰ�����������ӽ������������Ϊǰ������
* @param key	�������ⰴ�� 
* @param status ����(T)/����(F)
* @param free	����Ƿ������ƶ�
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
	//����
	if (status) {
		PxVec3 dir;
		if (!free) dir = this->faceDir; //�����ɾ�ͷ�����ﳯ��Ϊǰ������
		else dir = this->dir; //���ɾ�ͷ���������ǰ��Ϊǰ������
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
			PxTransform currfront = role->getGlobalPose();

			//PxU32 NbShapes = role->getNbShapes();
			//PxShape* shapes[MAX_NUM_ACTOR_SHAPES];
			//role->getShapes(shapes, NbShapes);
			//for (PxU32 i = 0; i < NbShapes; i++) {
			//	role->detachShape(*shapes[i]);
			//	//shapes[i]->setLocalPose(PxTransform(PxQuat(PxHalfPi, PxVec3(0, 1, 0))));
			//}
			//model->attachMeshes(PxTransform(PxQuat(-PxHalfPi, PxVec3(0, 1, 0))),role);

			role->setGlobalPose(rotate);

			cout << "LEFT!" << endl;
			break;

		}case GLUT_KEY_RIGHT: {
			PxTransform rotate = PxTransform(PxQuat(PxHalfPi, PxVec3(0, 1, 0)));
			dir = rotate.rotate(-dir);

			//PxU32 NbShapes = role->getNbShapes();
			//PxShape* shapes[MAX_NUM_ACTOR_SHAPES];
			//role->getShapes(shapes, NbShapes);
			//for (PxU32 i = 0; i < NbShapes; i++) {
			//	role->detachShape(*shapes[i]);
			//	//shapes[i]->setLocalPose(PxTransform(PxQuat(PxHalfPi, PxVec3(0, 1, 0))));
			//}
			//model->attachMeshes(PxTransform(PxQuat(-PxHalfPi, PxVec3(0, 1, 0))), role);
			role->setGlobalPose(rotate);
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
		std::cout << speed.x << " " << speed.y << ' ' << speed.z << ' ' << std::endl;
		this->updatePosition();
	}
	//����
	else
	{
		if (!this->isJump && !this->isFall) {
			this->faceDir = this->lastPressDir; //��ȡ���һ���ƶ����泯����
			if (!free) {
				this->dir = this->faceDir;//̧���ʱ��Ÿ��½�ɫ����ȷ�������ƶ�
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
* @brief ��ȡ��ɫcontroller�ĵײ�����
* @return PxVec3 
**/
PxVec3 Role::getFootPosition() {
	PxExtendedVec3 pos = this->roleController->getFootPosition();
	return PxVec3(pos.x, pos.y, pos.z);
}

/**
* @brief ��ȡ��ɫcontroller����������
* @return PxVec3
**/
PxVec3 Role::getPosition() {
	PxExtendedVec3 pos = this->roleController->getPosition();
	return PxVec3(pos.x, pos.y, pos.z);
}

/**
* @brief ���ý�ɫcontroller�ĵײ�����
* @param position ��ά�ռ������
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
			wantJumpHeight = primaryJumpHeight;
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
* @brief �����ӽǽ�ɫǰ������
**/
PxVec3 Role::getDir() {
	return this->dir;
}

/**
* @brief ��ȡ��ɫ�泯�ķ���
**/
PxVec3 Role::getFaceDir() {
	PxVec3 dir = this->speed.getNormalized();
	if (!this->speed.x || !this->speed.y || !this->speed.z) return this->faceDir;
	return this->speed.getNormalized();
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

/**
* @brief ��ɫ�Ƿ���
**/
void Role::changeCanMove(bool flag) {
	this->canMove = flag;
}