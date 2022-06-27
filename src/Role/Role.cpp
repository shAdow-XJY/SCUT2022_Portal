#include "Role.h"
#include "cmath"
#include <iostream>
#include <Role/RoleHitCallback.h>

extern void printPxVecFun(const PxVec3& vec);
extern clock_t deltaClock;

#define MAX_NUM_ACTOR_SHAPES 128

Role::Role() {
	PxCapsuleControllerDesc desc;
	desc.radius = roleRadius;
	desc.height = roleHeight;
	desc.material = gMaterial;
	desc.climbingMode = PxCapsuleClimbingMode::eCONSTRAINED;
	desc.stepOffset = 0.5f;
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
* @brief ����ɫ��ģ�Ͱ�
* @param path ģ���ļ�·��
**/
bool Role::attachModel(const char* path) {
	PxShape* cap;
	role->getShapes(&cap, 1);
	// ��Ϊfalse����ֻ��Ϊ��ײ�������Ⱦ����
	cap->setFlag(PxShapeFlag::eVISUALIZATION, false);
	this->model = new Model(path);
	//this->model->attachMeshes(PxTransform(PxQuat(-PxHalfPi, PxVec3(0.0f, 0.0f, 1.0f))).transform(PxTransform(PxVec3(0.0f,-0.4f,0.0f))), this->role);
	//this->role->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
	return true;
}


/**
* @brief �����ɫ���ƶ�����
**/
PxVec3 Role::roleHandleKey(GLint key, bool free) {
	PxVec3 dir;
	if (!free && this->rotateMoveDir) dir = this->faceDir; //�����ɾ�ͷ�����ﳯ��Ϊǰ������
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
		return PxVec3(0, 0, 0);
	}
	}
	return dir;
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
PxVec3 Role::getPosition() const {
	PxExtendedVec3 pos = this->roleController->getPosition();
	return PxVec3(pos.x, pos.y, pos.z);
}

/**
* @brief ���ý�ɫcontroller�ĵײ�����
* @param position ��ά�ռ������
**/
void Role::setFootPosition(PxVec3 position) {
	this->roleController->setFootPosition(PxExtendedVec3(position.x, position.y, position.z));
}

/**
* @brief ���Զ��ƶ�������ͬ����ɫ������Ϣ
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
* @brief ��ɫ���滬������
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
* @brief ��ɫ�¶�״̬
**/
bool Role::getCrouch(){
	return this->isCrouch;
}

/**
* @brief ��ɫ�¶�
**/
void Role::roleCrouch() {
	if (!isJump && !isFall) {
		this->isCrouch = true;
		this->roleController->resize(roleHeight / 2.5);
	}
}

/**
* @brief ��ɫ�¶׻ָ��׶�
**/
void Role::roleNoCrouch() {
	this->roleController->resize(roleHeight + roleRadius);
	this->isCrouch = false;
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
	PxVec3 dir = this->lastPressDir.getNormalized();
	if (this->speed.isZero()) return this->faceDir;
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
bool Role::roleOver() {
	this->resetStatus();
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
* @brief ��ɫ�Ƿ�����ƶ�
**/
void Role::changeCanMove(bool flag) {
	this->canMove = flag;
}


/**
* @brief ��ɫ��Ե����
**/
void Role::edgeSliding() {
	if (this->standingBlock->getType() == OrganType::seesaw) {
		PxVec3 spliceSpeed = this->speed.isZero() ? this->sliceDir : this->getFaceDir();
		this->setFootPosition(this->getFootPosition() + spliceSpeed * 1.0f);
	}
	else
	{
		this->setFootPosition(this->getFootPosition() + this->getFaceDir() * 3.0f); //��Ե����
	}
}


/**
* @brief ��ɫ����ʰȡ
**/
bool Role::pickUpObj() {
	//cout << this->faceDir.x<<" " << this->faceDir.y <<" "<< this->faceDir.z<<" " << endl;
	PxVec3 origin = this->getFootPosition() + PxVec3(0, 0.5f, 0);
	//ȷ��role��ǰ������
	PxVec3 forwardDir = this->getFaceDir()*5;
	PxRigidActor* actor = NULL;
	if (actor = RayCast(origin, forwardDir)) {
		GameSceneBasic* basic = (GameSceneBasic*)actor->userData;
		if (basic->getType() == OrganType::prop) {
			actor->release();
			this->equiped = true;
			std::cout << "ʰȡ���߳ɹ�" << std::endl;
			return true;
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
	return false;
}

/**
* @brief ��ɫ���߷���
**/
bool Role::layDownObj() {
	PxVec3 origin = this->getPosition() - PxVec3(0, 0.2f, 0);
	//ȷ��role��ǰ������
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
			std::cout << "���õ��߳ɹ�" << std::endl;
			return true;
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
	return false;
}

/**
* @brief ��ɫ����ʹ��
**/
bool Role::useKeyObj() {
	PxVec3 origin = this->getPosition();
	//ȷ��role��ǰ������
	PxRigidActor* actor = NULL;
	if (actor = RayCast(origin, this->getFaceDir()*5.0f)) {
		GameSceneBasic* basic = (GameSceneBasic*)actor->userData;
		if (basic->getType() == OrganType::keyDoor) {
			Door* door = (Door*)actor->userData;;
			if (this->equiped) {
				if (door->getNeedKey()) {
					if (door->getHasKey()) {
						std::cout << "�����ѱ�ʹ�õ���" << std::endl;
					}
					else {
						door->setHasKey(true);
						this->equiped = false;
						std::cout << "ʹ�õ��߳ɹ�" << std::endl;
						this->keyDoorActor = actor;
						//door->setPosition();
						return true;
					}
				}
				else {
					cout << "���ǿ�ʹ�õ��ߵ���" << endl;
				}
			}
			else
			{
				std::cout << "��ɫû��װ������" << std::endl;
			}
		}
		else
		{
			std::cout << "������" << std::endl;
		}
	}
	else
	{
		std::cout << "����û���ҵ�Ŀ��" << std::endl;
	}
	return false;
}


/**
* @brief ��ɫ�������߼��
**/
void Role::rayAround() {
	PxVec3 origin = this->getPosition() - PxVec3(0, -0.2f, 0);
	PxRigidActor* actor = NULL;
	//�����ܷ�������
	for (int i = -1; i < 2; i++) {
		for (int j = -1; j < 2; j++) {
			if (i == 0 && j == 0) continue;
			PxVec3 dir = PxVec3(i, 0, j).getNormalized() * 2.0f;
			actor = RayCast(origin, dir);
			if (actor) {
				GameSceneBasic* gsb = (GameSceneBasic*)actor->userData;
				if (gsb) {
					//�ڴ�
					if (gsb->getType() == OrganType::pendulum) {
						//cout << "ײ����" << endl;
						Pendulum* pendulem = (Pendulum*)gsb;
						int flag = pendulem->getPendulumActor()->getAngularVelocity().x > 0 ? 1 : -1;
						if (!this->stimulateObj) {
							PxShape* shape = gPhysics->createShape(PxCapsuleGeometry(0.05, 0.5), *gMaterial);
							//ƫ��ֵΪ���Լ����������ֵ��׼ȷֵ�д���ȶ
							PxVec3 pos = this->getPosition() + PxVec3(0, 0, 2) * flag;
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
				}
			}
		}
	}
}

/**
* @brief ��ɫ���и���ģ��
**/
void Role::stimulate() {
	if (this->stimulateObj) {
		const PxVec3 pos = this->stimulateObj->getGlobalPose().p;
		if (pos.y < 1.0f) {
			this->roleOver();
		}
		this->roleController->setPosition(PxExtendedVec3(pos.x, pos.y, pos.z));
	}
}

/**
* @brief ��ɫ����
**/
void Role::protal() {
	if (this->isRebirthing) {
		srand((int)time(0));
		int protalCheckpoint = rand() % this->arrivedCheckpoint;
		this->setFootPosition(checkpoints[protalCheckpoint]);
		this->life--;
		this->isRebirthing = false;
	}
}


/**
* @brief ��ɫ�÷�
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
* @brief ��ɫ�Ƿ�������״̬
**/
bool Role::getRebirthing() {
	return this->isRebirthing;
}

/**
* @brief ��ɫͳһ�ƶ�����
* @desc ÿ֡�ص�
**/
void Role::move() {
	if (!this->isAlive) return;
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
	//this->updatePosition();
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
		//���¾���
		if (standingBlock->getType() == OrganType::prismaticRoad) {
			PrismaticRoad* primaticRoad = (PrismaticRoad*)standingBlock;
			PxVec3 pos = primaticRoad->getStaticPosition() - (lastPosition - this->getFootPosition());
			primaticRoad->updateDistance(pos);
		}
	}
	//����
	else
	{
		if (!this->isJump && !this->isFall) {
			this->faceDir = this->lastPressDir; //����Ϊ���һ���ƶ����泯����
			if (!free && this->rotateMoveDir) {
				this->dir = this->faceDir;//̧���ʱ��Ÿ��½�ɫ����ȷ�������ƶ�
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
* @brief ��ɫ�ŵ�
**/
void Role::touchGround() {
	this->speed.y = 0;
	this->isFall = false;
	this->standingOnBlock = true;
	this->inertiaSpeed = PxVec3(0, 0, 0);
}

/**
* @brief ��ɫ��Ҫ����
**/
void Role::fall() {
	if (!isJump) {
		this->isFall = true;
	}

}

/**
* @brief ��ɫ�ײ���������
* @desc  ��ɫ����ģ�Ⲣ��ײ��������ģ�⽻��
**/
void Role::simulationGravity() {
	PxVec3 origin = this->getFootPosition();
	PxVec3 unitDir = PxVec3(0, -0.8f, 0);
	PxRigidActor* actor = NULL;
	if (actor = RayCast(origin, unitDir)) {
		if (isFall) {
			this->touchGround();
		}
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
				//�ٴμ�������ָ����ӳ�
				if (!RayCast(origin, PxVec3(0, -5.0f, 0))) {
					std::cout << "��Ե����" << endl;
					//this->edgeSliding();
				}
				else
				{
					//��ֹCCT��Ե��סģ�⻬��һ��
					PxVec3 slide = this->getFaceDir().getNormalized() * 0.045;
					roleController->move(slide * deltaClock, PxF32(0.00001), deltaClock, NULL);

				}
			}
			this->standingBlock = errorGameSceneBasic;
			this->standingOnBlock = false;
			this->fall();
		}

	}
}


/**
* @brief ��ɫ��Ծ
**/
bool Role::tryJump(bool release) {
	if (!this->isAlive) return false;
	if (!isJump && !isFall) {
		if (!release) {
			//������
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
* @brief ��ɫ��Ծ����
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
* @brief ��ɫ����
**/
void Role::roleFall() {
	if (isFall) {
		auto speed = this->speed;
		this->speed.y -= gravityAcceleration * deltaClock;
	}
}



/**
* @brief ���ý�ɫ״̬
**/
void Role::resetStatus() {
	this->speed = PxVec3(0, 0, 0);
	this->inertiaSpeed = PxVec3(0, 0, 0);
	this->slide = false;
	this->standingOnBlock = false;
	this->standingBlock = errorGameSceneBasic;

}

/**
* @brief ��ȡ��ɫˮƽ�ٶ�
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