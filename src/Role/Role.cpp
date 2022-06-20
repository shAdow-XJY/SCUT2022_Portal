#include "Role.h"
#include "cmath"
#include <iostream>

#define MAX_NUM_ACTOR_SHAPES 128


Role::Role() {
	PxCapsuleControllerDesc desc;
	desc.radius = roleRadius;
	desc.height = roleHeight;
	desc.material = gMaterial;
	desc.climbingMode = PxCapsuleClimbingMode::eLAST;
	desc.stepOffset = 0.1f;
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
	return this->isAutoMoving;
}

/**
* @brief ������ƶ���ȡĿ������
* @param x �����Ļ����x 
* @param y �����Ļ����y
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
* @param position ��ά�ռ�����
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
* @param key	�������ⰴ�� 
* @param status ����(T)/����(F)
* @param free	����Ƿ������ƶ�
**/
void Role::move(GLint key, bool status, bool free) {
	if (!this->canMove) {
		return;
	}
	if (!this->isAlive) {
		this->setSpeed(PxVec3(0, 0, 0));
		return;
	}
	//����
	if (status) {
		PxVec3 dir;
		if (!free) dir = this->faceDir; //�����ɾ�ͷ�����ﳯ��Ϊǰ������
		else dir = this->dir; //���ɾ�ͷ���������ǰ��Ϊǰ������
		//�ƶ��������
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
		if (gameSceneBasic.getType() == OrganType::iceroad) {
			this->speed = dir * 0.86f;
		}
		this->lastPressDir = dir.getNormalized();
		if (this->isJump || this->isFall) return;
		this->roleController->move(this->speed, 0.0001, 1.0f / 120.0f, NULL);
		this->updatePosition();
	}
	//����
	else
	{
		if (!this->isJump && !this->isFall) {
			this->faceDir = this->lastPressDir; //����Ϊ���һ���ƶ����泯����
			if (!free) {
				this->dir = this->faceDir;//̧���ʱ��Ÿ��½�ɫ����ȷ�������ƶ�
			}
			if (gameSceneBasic.getType() == OrganType::iceroad) {
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
* @brief ��ȡ��ɫ����������Ϣ
**/
PxVec3 Role::getRoleWorldPosition() {
	return this->role->getGlobalPose().p;
}

/**
* @brief ��ɫ��Ծ�����ж�
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
* @brief ��ɫ��Ծ
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
* @brief ��ɫ����
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
				this->updatePosition();
			};
		}
		
	}
}

/**
* @brief ��ɫ���滬������
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
	if (isSpeedZero()) return this->faceDir;
	return dir;
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
	this->stimulateObj = NULL;
}

/**
* @brief ��ɫ�Ƿ�����ƶ�
**/
void Role::changeCanMove(bool flag) {
	this->canMove = flag;
}

bool Role::isSpeedZero() {
	if (!this->speed.x && !this->speed.y && !this->speed.z) return true;
	return false;
}


void Role::edgeSliding() {
	if (this->gameSceneBasic.getType() == OrganType::seesaw) {
		PxVec3 spliceSpeed = isSpeedZero() ? this->sliceDir : this->getFaceDir();
		this->setFootPosition(this->getFootPosition() + spliceSpeed * 2.0f);
	}
	else
	{
		this->setFootPosition(this->getFootPosition() + this->getFaceDir() * 3.0f); //��Ե����
	}
}

/**
* @brief ��ɫ�ײ���������
* @desc  ���ڽ�ɫģ�������͸�����ʩ������
**/
void Role::simulationGravity() {
	PxVec3 origin = this->getFootPosition();
	PxVec3 unitDir = PxVec3(0, -0.8f, 0);
	PxRigidActor* actor = NULL;
	if (actor = RayCast(origin, unitDir)) {
		//��ײ������
		//std::cout << "��������" << std::endl;
		//cout << role->standingBlock.getName() << endl;
		this->standingOnBlock = true;
		GameSceneBasic* basic = (GameSceneBasic*)actor->userData;		
		this->sliceDir = PxVec3(0, 0, 0);
		if (basic != NULL) {
			//cout << block->getType() << endl;
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
			//std::cout << "yes" << std::endl;
			this->gameSceneBasic = *basic;
		}
	}
	else {
		if (!this->isJump && !this->isFall) {
			if (this->gameSceneBasic.getType() != OrganType::error) {
				//�ٴμ�������ָ����ӳ�
				if (!RayCast(origin, PxVec3(0, -5.0f, 0))) {
					std::cout << "��Ե����" << endl;
					this->edgeSliding();
				}
			}
			this->gameSceneBasic = GameSceneBasic();
			this->standingOnBlock = false;
			this->fall();
		}	
		
	}
}


/**
* @brief ��ɫ����ʰȡ
**/
void Role::pickUpObj() {
	//cout << this->faceDir.x<<" " << this->faceDir.y <<" "<< this->faceDir.z<<" " << endl;
	PxVec3 origin = this->getPosition() - PxVec3(0,0.2f,0);
	//ȷ��role��ǰ������
	PxVec3 forwardDir = this->getFaceDir() * 2;
	PxRigidActor* actor = NULL;
	if (actor = RayCast(origin, forwardDir)) {
		GameSceneBasic* basic = (GameSceneBasic*)actor->userData;
		if (basic->getType() == OrganType::prop) {
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


//�����ܷ�������
void Role::rayAround() {
	PxVec3 origin = this->getPosition();
	PxRigidActor* actor = NULL;
	for (int i = -1; i < 2; i++) {
		for (int j = -1; j < 2; j++) {
			 if (i == 0 && j == 0) continue;
			 PxVec3 dir = PxVec3(i, 0, j).getNormalized() * 2.0f;
			 actor = RayCast(origin, dir);
			 if (actor) {				 
				 GameSceneBasic* gsb = (GameSceneBasic*)actor->userData;
				 //�ڴ�
				 if (gsb && gsb->getType() == OrganType::pendulum) {
					 //cout << "ײ����" << endl;
					 Pendulum* pendulem = (Pendulum*)gsb;
					 //extern void printPxVecFun(const PxVec3 & vec);
					 int flag = pendulem->getPendulumActor()->getAngularVelocity().x > 0 ? 1 : -1;
					 if (!this->stimulateObj) {
						 PxShape* shape = gPhysics->createShape(PxCapsuleGeometry(0.05,0.5), *gMaterial);
						 //ƫ��ֵΪ���Լ����������ֵ��׼ȷֵ�д���ȶ
						 PxVec3 pos = this->getPosition() + PxVec3(0,0,2) * flag;
						 PxRigidDynamic* sceneBox = gPhysics->createRigidDynamic(PxTransform(pos));			 
						 sceneBox->attachShape(*shape);
						 sceneBox->setName("");
						 PxRigidBodyExt::updateMassAndInertia(*sceneBox, 0.00001f);
						 gScene->addActor(*sceneBox);
						 this->stimulateObj = sceneBox;
					 }
					
					 
				 }
			}
		}
	}
}

//ģ��
void Role::stimulate() {
	if (this->stimulateObj) {
		const PxVec3 pos = this->stimulateObj->getGlobalPose().p;
		this->roleController->setPosition(PxExtendedVec3(pos.x,pos.y,pos.z));
	}
}