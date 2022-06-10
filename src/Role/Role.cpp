#include "Role.h"
#include "cmath"
#include <iostream>


Role::Role() {
	PxCapsuleControllerDesc desc;
	desc.radius = roleRadius;
	desc.height = roleHeight;
	desc.material = gMaterial;
	desc.climbingMode = PxCapsuleClimbingMode::eLAST;
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
* @brief ��ȡ��ɫ�Ƿ��Զ��ƶ�
**/
bool Role::getMovingStatus() {
	return this->isAutoMoving;
}

/**
* @brief ������ƶ���ȡĿ������
* @Param x �����Ļ����x y�����Ļ����y
**/
void Role::roleMoveByMouse(int x, int y) {
	if (this->isAutoMoving || this->isJump || this->isFall || !this->isAlive) return;
	PxVec3 nowPosition = ScenetoWorld(x, y);
	this->lastPostion = PxVec3(this->nowPostion.x,this->nowPostion.y,this->nowPostion.z);
	this->nowPostion = nowPosition;
	this->isAutoMoving = true;
}

/**
* @brief ������ƶ���ȡĿ������
* @Param position ��ά�ռ�����
**/
void Role::roleMoveByMouse(PxVec3 position) {
	if (this->isAutoMoving || this->isJump || this->isFall || !this->isAlive || !this->canMove) return;
	this->lastPostion = PxVec3(this->nowPostion.x, this->nowPostion.y, this->nowPostion.z);
	this->nowPostion = position;
	this->isAutoMoving = true;
}

/**
* @brief ��ɫ�Զ��ƶ�
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
* @brief ��ɫֹͣ�Զ��ƶ�
**/
void Role::stopMoving() {
	this->isAutoMoving = false;
	this->nowPostion = PxVec3(this->roleController->getFootPosition().x, this->roleController->getFootPosition().y, this->roleController->getFootPosition().z);
}

/**
* @brief ����������ƽ�ɫ�ƶ�
* @desc	 �����ӽ��Խ�ɫ�泯����Ϊǰ�����������ӽ������������Ϊǰ������
* @Param key�������ⰴ�� status����(T)/����(F) free����Ƿ������ƶ�
**/
void Role::move(GLint key,bool status,bool free) {
	if (!this->canMove) {
		return;
	}
	if (!this->isAlive) {
		this->setSpeed(PxVec3(0, 0, 0));
		return;
	}
	this->isAutoMoving = false; //ֹͣ�Զ��ƶ�
	//�ƶ���������
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
	//�ƶ���������
	else
	{
		if (!this->isJump && !this->isFall) {
			this->faceDir = this->lastPressDir; //��ȡ���һ���ƶ����泯����
			if (!free) {
				this->dir = this->faceDir;//̧���ʱ��Ÿ��½�ɫ����ȷ�������ƶ�
			}
			this->speed = PxVec3(0, 0, 0); //�����ٶ�

		}
		else
		{
			//���д����ƶ�����
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
* @brief ��ȡ��ɫcontroller����������
* @Return PxVec3
**/
PxVec3 Role::getPosition() {
	PxExtendedVec3 pos = this->roleController->getPosition();
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
			//std::cout << "wantJumpHeight" << wantJumpHeight << std::endl;
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
* @brief ��ɫ����
**/
void Role::roleFall() {
	if (isFall) {
		PxControllerCollisionFlags flag = roleController->move(PxVec3(0.0, -midFallSpeed, 0.0) + this->speed * 0.3, PxF32(0.00001), 1.0f / 60.0f, NULL);		
		if (flag == PxControllerCollisionFlag::eCOLLISION_SIDES) {
			this->setSpeed(PxVec3(0, 0, 0));
		}
		else if (flag == PxControllerCollisionFlag::eCOLLISION_DOWN) {
			this->setSpeed(PxVec3(0, 0, 0));
			isFall = false;
			if (!this->isAutoMoving) {
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
* @brief ��ɫ��õ���
**/
void Role::setEquiped(bool equip) {
	this->equiped = equip;
}

/**
* @brief ��ý�ɫ����״̬
**/
bool Role::getEquiped() {
	return this->equiped;
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
bool Role::getAliveStatus() {
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


/**
* @brief ��ɫ�ײ���������
* @desc  ���ڽ�ɫģ�������͸�����ʩ������
**/
void Role::simulationGravity() {
	PxVec3 origin = this->getFootPosition();
	PxVec3 unitDir = PxVec3(0, -0.5f, 0);
	PxRigidActor* actor = NULL;
	if (actor = RayCast(origin, unitDir)) {
		//��ײ������
		//std::cout << "��������" << std::endl;
		//cout << role->standingBlock.getName() << endl;
		Block* block = (Block*)actor->userData;
		if (block != NULL) {
			//cout << block->getType() << endl;
			if (block->getType() == BlockType::road) {
				//std::cout << role->standingBlock.getName()<<std::endl;

			}
			else if (block->getType() == BlockType::seesaw) {
				//cout << "ʩ������" << endl;
				Seesaw* seesaw = (Seesaw*)block;
				PxRigidBody* seesawBody = seesaw->getSeesawActor();
				PxVec3 force = PxVec3(0, -1, 0) * this->mass;
				PxRigidBodyExt::addForceAtPos(*seesawBody, force, this->getFootPosition());
				//seesawBody->addForce()
			}
			this->standingBlock = *block;
		}
	}
	else {
		if (this->standingBlock.getType() != BlockType::error) {
			this->setFootPosition(this->getFootPosition() + this->getSpeed() * 5.0f); //��Ե����
		}
		this->standingBlock = Block();
		//std::cout << "δ��������" << std::endl;	
		//role->gameOver();
		this->fall();
	}
}


/**
* @brief ��ɫ����ʰȡ
**/
void Role::pickUpObj() {
	PxVec3 origin = this->getPosition();
	//ȷ��role��ǰ������
	PxVec3 forwardDir = this->getFaceDir() * 2;
	PxRigidActor* actor = NULL;
	if (actor = RayCast(origin, forwardDir)) {
		Block* block = (Block*)actor->userData;
		if (block->getType() == BlockType::prop) {
			actor->release();
			this->equiped = true;
			std::cout << "ʰȡ���߳ɹ�" << std::endl;
		}
		else
		{
			std::cout << "���ǵ���" << std::endl;
		}
	}
	else
	{
		std::cout << "����û���ҵ�Ŀ��" << std::endl;
	}
}

/**
* @brief ��ɫ���߷���
**/
void Role::layDownObj() {
	PxVec3 origin = this->getPosition();
	//ȷ��role��ǰ������
	PxVec3 forwardDir = PxVec3(this->getFaceDir().x, -3, this->getFaceDir().z);
	PxRigidActor* actor = NULL;
	if (actor = RayCast(origin, forwardDir)) {
		Block* block = (Block*)actor->userData;
		cout << block->getType() << endl;
		if (block->getType() == BlockType::road) {
			this->equiped = false;
			extern void createPorp(const PxTransform & t, const PxVec3 & v, PxReal x, PxReal y, PxReal z);
			//cout << role->getPosition().x << " " << role->getPosition().y << " " << role->getPosition().z << endl;
			//cout << role->getFaceDir().x << " " << role->getFaceDir().y << " " << role->getFaceDir().z << endl;
			createPorp(PxTransform(PxVec3(0, 0, 0)), this->getPosition() + this->getFaceDir() * 2.5, boxHeight, boxHeight, boxHeight);
			std::cout << "���õ��߳ɹ�" << std::endl;
		}
		else
		{
			std::cout << "���ǿɷ��õ��ߵĵط�" << std::endl;
		}
	}
	else
	{
		std::cout << "����û���ҵ�Ŀ��" << std::endl;
	}
}
