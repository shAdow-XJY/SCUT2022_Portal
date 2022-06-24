#include <ctype.h>
#include "PxPhysicsAPI.h"
#include "../Sphere/Pendulum.h"
#include "../Block/Door.h"
#include "../Block/Road.h"
#include "../Block/Seesaw.h"
#include "../Block/RotateRod.h"
#include "../Block/PrismaticRoad.h"
#include<vector>
#include<iostream>
#include <glut.h>
#include <Render/BMPLoader.h>
#include <map>
#include <time.h>

using namespace physx;

extern PxScene* gScene;
extern PxMaterial* gMaterial;
extern PxPhysics* gPhysics;
extern vector<PxVec3> checkpoints;
extern int primaryJumpHeight;
//�趨�������Ծ�߶ȣ��ɵ���
float maxJumpHeight = 6.0;
//������·���ӵİ��
float boxHeight = 0.4 * maxJumpHeight;
//x_distance x�᷽���Ͽ���Ծ�ļ�����ȣ��ɸ�����Ծ��ȵ���
float dx = 8.0;
//z_distance z�᷽���Ͽ���Ծ�ļ�����ȣ��ɸ�����Ծ��ȵ���
float dz = 8.0;

//�Թ����ſɿ�����
vector<vector<int>> frontDoorCanOpen = {
	{0,1,0,1,1,0,1,1,0},
	{0,0,0,1,1,0,1,0,0},
	{1,1,0,1,1,1,1,1,0},
	{0,0,1,0,0,1,1,1,0},
	{0,1,1,0,0,0,1,1,0},
	{0,0,1,0,0,0,0,1,0},
	{0,1,0,0,0,1,0,0,0},
	{0,1,0,1,0,1,0,1,1}
};
//�Թ����ſɿ�����
vector<vector<int>> sideDoorCanOpen = {
	{0,1,0,0,0,1,1,1,0},
	{0,1,1,1,0,0,1,0,0},
	{0,0,1,0,1,0,1,1,0},
	{0,0,0,1,1,1,1,1,0},
	{0,1,0,0,1,1,1,1,0},
	{0,1,0,0,0,1,1,0,0},
	{0,0,1,0,0,0,1,1,0},
	{0,0,0,1,0,1,0,0,0}
};      

//��ѡ���ʣ�["door"]\["wall"]\[]
//extern std::map<string, CBMPLoader*> textureMap;
//extern CBMPLoader* TextureLoader;

const char* typeMapName(OrganType type) {
	switch (type)
	{
	case OrganType::road: {
		return "Road";
		break;
	}
	case OrganType::wall: {
		return "Wall";
		break;
	}
	case OrganType::door: {
		return "Door";
		break;
	}
	case OrganType::seesaw:{
		return "Seesaw";
	}
	case OrganType::seesawbox:{
		return "SeesawBox";
	}
	case OrganType::iceroad: {
		return "Ice";
	}
	default:
		return "Block";
		break;
	}
}

/*����������״�ľ�̬����
tΪ�ø��幹�������ԭ��
vΪ�ø������ĵ������ԭ���λ��
x, y, zΪ�ú��ӵĳ��߿�
pose ����ĳ�ʼ����*/
PxRigidStatic* createStaticBox(const PxTransform& t, const PxVec3& v, PxReal x, PxReal y, PxReal z, PxTransform& pose, OrganType type = OrganType::block) {
	PxTransform local(v);
	PxShape* shape = gPhysics->createShape(PxBoxGeometry(x, y, z), *gMaterial);
	//��ײ���Ĺ�����
	shape->setQueryFilterData(collisionGroup);
	shape->setLocalPose(pose);
	PxRigidStatic* sceneBox = gPhysics->createRigidStatic(t.transform(local));
	sceneBox->attachShape(*shape);
	sceneBox->setName(typeMapName(type));
	gScene->addActor(*sceneBox);
	return sceneBox;
}

/*����������״�Ķ�̬����
kinematic ��̬�����Ƿ�Ϊ�����kinematic����
tΪ�ø��幹�������ԭ��
vΪ�ø������ĵ������ԭ���λ��
x, y, zΪ�ú��ӵĳ��߿�
pose ����ĳ�ʼ����
velocity ����ĳ�ʼ�ٶȣ�Ĭ��Ϊ0*/
PxRigidDynamic* createDynamicBox(bool kinematic, const PxTransform& t, const PxVec3& v, PxReal x, PxReal y, PxReal z, PxTransform& pose, OrganType type = OrganType::block, const PxVec3& velocity = PxVec3(0)) {
	PxTransform local(v);
	PxShape* shape = gPhysics->createShape(PxBoxGeometry(x, y, z), *gMaterial);
	shape->setQueryFilterData(collisionGroup);
	shape->setLocalPose(pose);
	PxRigidDynamic* sceneBox = gPhysics->createRigidDynamic(t.transform(local));
	sceneBox->attachShape(*shape);
	sceneBox->setAngularDamping(10.0f);
	sceneBox->setLinearVelocity(velocity);
	sceneBox->setName(typeMapName(type));
	PxRigidBodyExt::updateMassAndInertia(*sceneBox, 1.0f);
	if (kinematic) {
		sceneBox->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
	}
	gScene->addActor(*sceneBox);
	return sceneBox;
}


//���������࣬����ֻ��Block(OrganType::prop)�������ã����޸�
void createPorp(const PxTransform& t, const PxVec3& v, PxReal x, PxReal y, PxReal z) {
	PxTransform local(v);
	//cout << v.x << " " << v.y << " " << v.z << endl;
	PxShape* shape = gPhysics->createShape(PxBoxGeometry(x, y, z), *gMaterial);
	//��ײ���Ĺ�����
	shape->setQueryFilterData(collisionGroup);

	//setupFiltering(shape, FilterGroup::ePIG, FilterGroup::eBIRD);
	PxRigidDynamic* sceneBox = gPhysics->createRigidDynamic(t.transform(local));
	PxRigidBodyExt::updateMassAndInertia(*sceneBox, 1.0f);
	Block* block = new Block("����", sceneBox->getGlobalPose().p, x, y, z, OrganType::prop);
	sceneBox->attachShape(*shape);
	sceneBox->userData = block;
	sceneBox->setName("Prop");
	gScene->addActor(*sceneBox);
}


void createPlane(const PxVec3& point, const PxVec3& normal) {
	/*PxTransformFromPlaneEquation()��ƽ�淽��ת���ɵȼ۵�transform��
	PxPlaneEquationFromTransform()�ṩ�෴��ת����
	PxPlaneGeometryû��attribute����Ϊ��״��λ����ȫ������ƽ�����ײ�����
	PxPlaneGeometry��shapeֻ��Ϊ��̬actor������*/

	PxPlane p(point, normal);
	PxShape* planeShape = gPhysics->createShape(PxPlaneGeometry(), *gMaterial);	
	PxRigidStatic* plane = gPhysics->createRigidStatic(PxTransformFromPlaneEquation(p));
	plane->attachShape(*planeShape);
	plane->setName("Over"); //over��ʾ��ɫ�Ӵ�������
	gScene->addActor(*plane);

	
}

/*������·
tΪ�õ�·���������ԭ��
vΪ�õ�·���ĵ�����乹��ԭ���λ��
x, y, zΪ�õ�·�ĳ��߿�
pose ��·�ĳ�ʼ����*/
PxRigidStatic* createRoad(const PxTransform& t, const PxVec3& v, PxReal x, PxReal y, PxReal z, PxTransform& pose) {
	PxRigidStatic* roadActor = createStaticBox(t, v, x, y, z, pose, OrganType::road);
	//std::cout << "v::" <<  v.x << " " << v.y << " " << v.z << endl;
	PxVec3 position = roadActor->getGlobalPose().p;
	Road* road = new Road("·��",position, x, y, z,roadActor);
	roadActor->userData = road;
	roadActor->setName("Road");
	return roadActor;
}

/**
* @brief ������·
**/
PxRigidStatic* createIceRoad(const PxTransform& t, const PxVec3& v, PxReal x, PxReal y, PxReal z, PxTransform& pose) {
	PxRigidStatic* roadActor = createStaticBox(t, v, x, y, z, pose, OrganType::iceroad);
	//std::cout << "v::" <<  v.x << " " << v.y << " " << v.z << endl;
	PxVec3 position = roadActor->getGlobalPose().p;
	Road* road = new Road("��·��", position, x, y, z, roadActor,OrganType::iceroad);
	roadActor->userData = road;
	return roadActor;
}

//�������һ�������y����ĺ���
float center_y(float y) {
	return y + 2 * boxHeight;
}

//PxRevoluteJoint* createSideDoor(const PxTransform& t, PxVec3 v, float scale, PxTransform& pose, bool canOpen = true) {
//	PxTransform pos(t.transform(PxTransform(v)));
//	PxReal x = 10 * scale;
//	PxReal y = 1 * scale;
//	PxReal z = 5 * scale;
//	PxRigidDynamic* actor0 = createDynamicBox(false, pos, PxVec3(0, 0, 11.5 * scale), x, y, z, pose);
//	PxRigidStatic* actor1 = createStaticBox(pos, PxVec3(0, 0, 0), 1 * scale, 10 * scale, 6 * scale, pose);
//	createStaticBox(pos, PxVec3(0, 0, 23 * scale), 1 * scale, 10 * scale, 6 * scale, pose);
//	PxTransform localFrame0(PxVec3(0, 0, -5 * scale));
//	PxTransform localFrame1(PxVec3(0, 0, 6.5 * scale));
//	PxRevoluteJoint* revolute = PxRevoluteJointCreate(*gPhysics, actor0, localFrame0, actor1, localFrame1);
//	Door* door = new Door("����", actor0->getGlobalPose().p, x, y, z, actor0, canOpen, revolute);
//	actor0->setName("Door");
//	actor0->userData = door;
//	//PxJointAngularLimitPair limitPair(-PxPi / 4, PxPi / 4, 0.1f);
//	//limitPair.stiffness = 7.0f;
//	//limitPair.damping = 100.0f;
//	//revolute->setLimit(limitPair);
//	if (canOpen) {
//		PxJointAngularLimitPair limits(-PxPi / 2, PxPi / 2, 0.01f);
//		revolute->setLimit(limits);
//	}
//	else {
//		PxJointAngularLimitPair limits(-PxPi / 130, 0, 0.01f);
//		revolute->setLimit(limits);
//	}
//	revolute->setRevoluteJointFlag(PxRevoluteJointFlag::eLIMIT_ENABLED, true);
//	revolute->setLocalPose(PxJointActorIndex::Enum::eACTOR1, PxTransform(PxVec3(0, 0, 6.5 * scale), PxQuat(PxHalfPi, PxVec3(0, 0, 1))));
//	return revolute;
//}

//PxRevoluteJoint* createFrontDoor(const PxTransform& t, PxVec3 v, float scale, PxTransform& pose, bool canOpen = true) {
//	PxTransform pos(t.transform(PxTransform(v)));//-17.8  -18.56 -18.561815
//	PxReal x = 10 * scale;
//	PxReal y = 5 * scale;
//	PxReal z = 1 * scale;
//	//ʵ��-5  ��ת�Ƕ�С��-17.8  -18.56 -18.561815
//	PxRigidStatic* actor1 = createStaticBox(pos, PxVec3(0, 0, 0), 6 * scale, 10 * scale, 1 * scale, pose);
//	PxRigidDynamic* actor0 = createDynamicBox(false, pos, PxVec3(6.5 * scale, -5 * scale, 0), x, y, z, pose);
//	createStaticBox(pos, PxVec3(23 * scale, 0, 0), 6 * scale, 10 * scale, 1 * scale, pose);
//	PxTransform localFrame0(PxVec3(0, 5 * scale, 0));
//	PxTransform localFrame1(PxVec3(6.5 * scale, 0, 0));
//	PxRevoluteJoint* revolute = PxRevoluteJointCreate(*gPhysics, actor0, localFrame0, actor1, localFrame1);
//	Door* door = new Door("����", actor0->getGlobalPose().p, x, y, z, actor0, canOpen, revolute);
//	actor0->setName("Door");
//	actor0->userData = door;
//	PxJointAngularLimitPair limits(-PxPi / 130, 0, 0.01f);
//	revolute->setLimit(limits);
//	revolute->setRevoluteJointFlag(PxRevoluteJointFlag::eLIMIT_ENABLED, true);
//	revolute->setLocalPose(PxJointActorIndex::Enum::eACTOR1, PxTransform(PxVec3(6.5 * scale, 0, 0), PxQuat(PxHalfPi, PxVec3(0, 0, 1))));
//	return revolute;
//}

/*����
tΪ����������ԭ��
vΪ��������joint���ӵ��ſ�����ĵ���Գ���ԭ���λ�� v(v_x, �������к��ӵĸ�+10*scale��v_z)
scale �ŵ�����ϵ��
pose ���峯��
canOpen �ŵĿɿ�����*/
void createFrontDoor(const PxTransform& t, PxVec3 v, float scale, PxTransform& pose,bool canOpen = true) {
	PxTransform pos(t.transform(PxTransform(v)));//-17.8  -18.56 -18.561815

	PxReal x = 10 * scale;
	PxReal y = 5 * scale;
	PxReal z = 1 * scale;
	//ʵ��-5  ��ת�Ƕ�С��-17.8  -18.56 -18.561815
	PxRigidStatic* actor1 = createStaticBox(pos, PxVec3(0, 0, 0), 6 * scale, 10 * scale, 1 * scale, pose, OrganType::wall);
	createStaticBox(pos, PxVec3(23 * scale, 0, 0), 6 * scale, 10 * scale, 1 * scale, pose, OrganType::wall);
	if (canOpen) {
		PxRigidDynamic* actor0 = createDynamicBox(false, pos, PxVec3(6.5 * scale, -5 * scale, 0), x, y, z, pose, OrganType::door);
		PxTransform localFrame0(PxVec3(0, 5 * scale, 0));
		PxTransform localFrame1(PxVec3(6.5 * scale, 0, 0));
		PxRevoluteJoint* revolute = PxRevoluteJointCreate(*gPhysics, actor0, localFrame0, actor1, localFrame1);
		Door* door = new Door("����", actor0->getGlobalPose().p, x, y, z, actor0, canOpen, revolute);
		actor0->userData = door;
		PxJointAngularLimitPair limits(-PxPi / 130, 0, 0.01f);
		revolute->setLimit(limits);
		revolute->setRevoluteJointFlag(PxRevoluteJointFlag::eLIMIT_ENABLED, true);
		revolute->setLocalPose(PxJointActorIndex::Enum::eACTOR1, PxTransform(PxVec3(6.5 * scale, 0, 0), PxQuat(PxHalfPi, PxVec3(0, 0, 1))));
		//return revolute;
	}
	else {
		PxRigidStatic* actor0 = createStaticBox(pos, PxVec3(11.5 * scale, 0, 0), y, x, z, pose, OrganType::door);
		Door* door = new Door("����", actor0->getGlobalPose().p, x, y, z, false);
		actor0->userData = door;
	}
}

/*����
tΪ����������ԭ��
vΪ�ò�����joint���ӵ��ſ�����ĵ���Գ���ԭ���λ��
scale �ŵ�����ϵ��
pose ���峯��
canOpen �ŵĿɿ�����*/
void createSideDoor(const PxTransform& t, PxVec3 v, float scale, PxTransform& pose, bool canOpen = true) {
	PxTransform pos(t.transform(PxTransform(v)));
	PxReal x = 10 * scale;
	PxReal y = 1 * scale;
	PxReal z = 5 * scale;
	PxRigidStatic* actor1 = createStaticBox(pos, PxVec3(0, 0, 0), 1 * scale, 10 * scale, 6 * scale, pose, OrganType::wall);
	createStaticBox(pos, PxVec3(0, 0, 23 * scale), 1 * scale, 10 * scale, 6 * scale, pose, OrganType::wall);
	if (canOpen) {
		PxRigidDynamic* actor0 = createDynamicBox(false, pos, PxVec3(0, 0, 11.5 * scale), x, y, z, pose, OrganType::door);
		PxTransform localFrame0(PxVec3(0, 0, -5 * scale));
		PxTransform localFrame1(PxVec3(0, 0, 6.5 * scale));
		PxRevoluteJoint* revolute = PxRevoluteJointCreate(*gPhysics, actor0, localFrame0, actor1, localFrame1);
		Door* door = new Door("����", actor0->getGlobalPose().p, x, y, z, actor0, canOpen, revolute);
		actor0->setName("Door");
		actor0->userData = door;
		/*PxJointAngularLimitPair limitPair(-PxPi / 4, PxPi / 4, 0.1f);
		limitPair.stiffness = 7.0f;
		limitPair.damping = 100.0f;
		revolute->setLimit(limitPair);*/
		PxJointAngularLimitPair limits(-PxPi / 2, PxPi / 2, 0.01f);
		revolute->setLimit(limits);
		revolute->setRevoluteJointFlag(PxRevoluteJointFlag::eLIMIT_ENABLED, true);
		revolute->setLocalPose(PxJointActorIndex::Enum::eACTOR1, PxTransform(PxVec3(0, 0, 6.5 * scale), PxQuat(PxHalfPi, PxVec3(0, 0, 1))));
		//return revolute;
	}
	else {
		PxRigidStatic* actor0 =  createStaticBox(pos, PxVec3(0, 0, 11.5 * scale), y, x, z, pose, OrganType::door);
		Door* door = new Door("����", actor0->getGlobalPose().p, x, y, z, false);
		actor0->userData = door;
	}
}

/*�����Թ��ؿ�
tΪ����������ԭ��
vΪ���Թ�������Գ���ԭ���λ��
scale �ŵ�����ϵ��
pose ����ĳ���*/
void createMaze(const PxTransform& t, PxVec3 v, float scale, PxTransform& pose) {
	//�Թ�С����߳�
	float roomLength = 37 * scale;
	
	//�Թ����½�����
	float x = v.x + 4 * roomLength;
	float z = v.z - 4 * roomLength;

	//�Թ��ŵ�����
	float door_x;
	float door_y = v.y + boxHeight + 0.5 + 10 * scale;
	float door_z;

	//Կ�״�Ŵ� 21 82 13 86 18
	vector<PxVec3> keyPositions = {
		PxVec3(x - 1.5 * roomLength,door_y,z + 0.5 * roomLength),
		PxVec3(x - 7.5 * roomLength,door_y,z + 1.5 * roomLength),
		PxVec3(x - 0.5 * roomLength,door_y,z + 2.5 * roomLength),
		PxVec3(x - 7.5 * roomLength,door_y,z + 5.5 * roomLength),
		PxVec3(x - 0.5 * roomLength,door_y,z + 7.5 * roomLength)
	};

	//�������Կ��
	srand(time(NULL));
	int index = rand() % keyPositions.size();
	std::cout << "Կ��" << index << endl;
	createPorp(t, keyPositions[index], 1.0, 1.0, 1.0);

	//�����߼�
	//������
	for (int i = 0; i < 8; i++) {
		door_x = x - i * 37 * scale - 30 * scale;
		//���µ���
		for (int j = 0; j < 9; j++) {
			door_z = z + j * 37 * scale;
			if (frontDoorCanOpen[i][j]) {
				createFrontDoor(t, PxVec3(door_x, door_y, door_z), scale, pose, true);
			}
			else {
				createFrontDoor(t, PxVec3(door_x, door_y, door_z), scale, pose, false);
			}
		}
	}

	//�����߼�
	//���µ���
	for (int i = 0; i < 8; i++) {
		door_z = z + i * 37 * scale + 7 * scale;
		//������
		for (int j = 0; j < 9; j++) {
			door_x = x - j * 37 * scale;
			if (sideDoorCanOpen[i][j]) {
				createSideDoor(t, PxVec3(door_x, door_y, door_z), scale, pose, true);
			}
			else {
				createSideDoor(t, PxVec3(door_x, door_y, door_z), scale, pose, false);
			}
		}
	}
}

/*�ΰ�
tΪ����������ԭ��
vΪ���ΰ�������Գ���ԭ���λ�� v�ĵڶ�������Ӧ��Ϊ �������к��ӵĸ� + y
x��y��zΪ����ĳ��߿�
pose ����ĳ�ʼ����*/
PxRevoluteJoint* createSeesaw(const PxTransform& t,PxVec3 v,float x, float y, float z, PxTransform& pose) {
	PxTransform pos(t.transform(PxTransform(v)));
	PxRigidDynamic* actor0 = createDynamicBox(false, pos, PxVec3(0, 0, 0), x, y, z, pose, OrganType::seesaw);
	PxVec3 position = PxVec3(actor0->getGlobalPose().p.x, actor0->getGlobalPose().p.y, actor0->getGlobalPose().p.z);
	Seesaw* seesaw = new Seesaw("�̰�",position,x,y,z,actor0);
	actor0->userData = seesaw;
	PxRigidStatic* actor1 = createStaticBox(pos, PxVec3(-(x + y + 0.5), 0, 0), y, y, y, pose, OrganType::seesawbox);
	createStaticBox(pos, PxVec3(x+y+0.5, 0, 0), y, y, y, pose, OrganType::seesawbox);
	PxTransform localFrame0(PxVec3(0, 0, 0));
	PxTransform localFrame1(PxVec3(x+y+0.5, 0, 0));
	PxRevoluteJoint* revolute = PxRevoluteJointCreate(*gPhysics, actor0, localFrame0, actor1, localFrame1);
	//revolute->setLocalPose(PxJointActorIndex::Enum::eACTOR0, PxTransform(PxVec3(0, 0, 0), PxQuat(1.75 * PxHalfPi, PxVec3(1, 0, 0))));
	seesaw->attachRevolute(revolute);
	return revolute;
}

void createSideSeesaw(const PxTransform& t, PxVec3 v, float x, float y, float z, PxTransform& pose) {
	PxTransform pos(t.transform(v));
	PxRigidStatic* actor0 = createStaticBox(pos, PxVec3(0, 0, 0), y, y, y, pose, OrganType::seesawbox);
	PxRigidDynamic* actor1 = createDynamicBox(false, pos, PxVec3(x, 0, -y - 0.5), x, y, z, pose, OrganType::seesaw);
	PxTransform localFrame0(PxVec3(0, 0, -y - 0.5));
	PxTransform localFrame1(PxVec3(-x, 0, 0));
	PxRevoluteJoint* revolute = PxRevoluteJointCreate(*gPhysics, actor0, localFrame0, actor1, localFrame1);
	revolute->setLocalPose(PxJointActorIndex::eACTOR0, PxTransform(PxVec3(0, 0, -y - 0.5), PxQuat(PxHalfPi, PxVec3(0, 1, 0))));
	createStaticBox(pos, PxVec3(0, 0, -2 * y - 2 * x - 1.0), y, y, y, pose, OrganType::seesawbox);
	PxVec3 position = actor1->getGlobalPose().p;
	Seesaw* seesaw = new Seesaw("�ΰ�", position, x, y, z, actor1,1);
	actor1->userData = seesaw;
	seesaw->attachRevolute(revolute);
	revolute->setLimit(PxJointAngularLimitPair(-PxPi / 130, 0, 0.01f));
	revolute->setRevoluteJointFlag(PxRevoluteJointFlag::eLIMIT_ENABLED, true);
}

/*�������ΰ�ؿ�
vΪ�ΰ�0�����ĵ�
sx��sy��szΪ�ΰ�İ볤
pose ������õĳ���*/
float createSeesawLevel(const PxTransform& t, PxVec3 v,float sx, float sy, float sz, PxTransform& pose) {
	//��һ���ΰ�0
	float x0 = v.x;
	float y0 = v.y;
	float z0 = v.z;
	PxTransform pose0(PxQuat(PxHalfPi / 6, PxVec3(1, 0, 0)));
	//createSeesaw(t, v, sx, sy, sz, pose0);
	//�ΰ�ˮƽλ��
	createSeesaw(t, v, sx, sy, sz, pose);

	//�ڶ����ΰ�1
	float x1 = x0 + 2*sx + dx;
	float z1 = z0 + sz + dz / 2;
	PxTransform pose1(PxQuat(PxHalfPi / 6, PxVec3(1, 0, 0)));
	//createSeesaw(t, PxVec3(x1, y0, z1), sx, sy, sz, pose1);
	//�ΰ�ˮƽλ��
	createSeesaw(t, PxVec3(x1, y0, z1), sx, sy, sz, pose);
	
	float x2 = x1;
	float z2 = z0 - sz - dz / 2;
	PxTransform pose2(PxQuat(-PxHalfPi / 6, PxVec3(1, 0, 0)));
	//createSeesaw(t, PxVec3(x2, y0, z2), sx, sy, sz, pose2);
	//�ΰ�ˮƽλ��
	createSeesaw(t, PxVec3(x2, y0, z2), sx, sy, sz, pose);

	float x3 = x0 + 4 * sx + 2 * dx;
	float z3 = z0;
	createSeesaw(t, PxVec3(x3, y0, z3), sx, sy, sz, pose);

	float x4 = x3;
	float z4 = z0 + 2 * sz + dz;
	PxTransform pose4(PxQuat(PxHalfPi / 6, PxVec3(1, 0, 0)));
	//createSeesaw(t, PxVec3(x4, y0, z4), sx, sy, sz, pose4);
	//�ΰ�ˮƽλ��
	createSeesaw(t, PxVec3(x4, y0, z4), sx, sy, sz, pose);

	float x5 = x3;
	float z5 = z0 - 2 * sz - dz;
	PxTransform pose5(PxQuat(PxHalfPi / 6, PxVec3(1, 0, 0)));
	//createSeesaw(t, PxVec3(x5, y0, z5), sx, sy, sz, pose5);
	//�ΰ�ˮƽλ��
	createSeesaw(t, PxVec3(x5, y0, z5), sx, sy, sz, pose);

	float x6 = x1 + 4 * sx + 2 * dx;
	float z6 = z1;
	PxTransform pose6(PxQuat(PxHalfPi / 6, PxVec3(1, 0, 0)));
	//createSeesaw(t, PxVec3(x6, y0, z6), sx, sy, sz, pose6);
	//�ΰ�ˮƽλ��
	createSeesaw(t, PxVec3(x6, y0, z6), sx, sy, sz, pose);

	float x7 = x6;
	float z7 = z2;
	PxTransform pose7(PxQuat(-PxHalfPi / 6, PxVec3(1, 0, 0)));
	//createSeesaw(t, PxVec3(x7, y0, z7), sx, sy, sz, pose7);
	//�ΰ�ˮƽλ��
	createSeesaw(t, PxVec3(x7, y0, z7), sx, sy, sz, pose);
	
	float x8 = x3 + 4 * sx + 2 * dx;
	float z8 = z0;
	PxTransform pose8(PxQuat(-PxHalfPi / 6, PxVec3(1, 0, 0)));
	//createSeesaw(t, PxVec3(x8, y0, z8), sx, sy, sz, pose8);
	//�ΰ�ˮƽλ��
	createSeesaw(t, PxVec3(x8, y0, z8), sx, sy, sz, pose);

	return x8;
 }

/*����������״�Ķ�̬����
tΪ�ø��幹�������ԭ��
vΪ�ø������ĵ�����乹��ԭ���λ��
halfExtend����뾶
velocity ����ĳ�ʼ�ٶȣ�Ĭ��Ϊ0*/
PxRigidDynamic* createDynamicSphere(const PxTransform& t, const PxVec3& v, PxReal halfExtend, const PxVec3& velocity = PxVec3(0)) {
	PxTransform local(v);
	PxShape* shape = gPhysics->createShape(PxSphereGeometry(halfExtend), *gMaterial);
	shape->setQueryFilterData(collisionGroup);
	PxRigidDynamic* sceneBox = gPhysics->createRigidDynamic(t.transform(local));
	sceneBox->attachShape(*shape);
	sceneBox->setAngularDamping(1.0f);
	sceneBox->setLinearVelocity(velocity);
	sceneBox->setName("");
	PxRigidBodyExt::updateMassAndInertia(*sceneBox, 10.0f);
	gScene->addActor(*sceneBox);
	return sceneBox;
}

/*����������״�ľ�̬����
tΪ�ø��幹�������ԭ��
vΪ�ø������ĵ�����乹��ԭ���λ��
halfExtend����뾶*/
PxRigidStatic* createStaticSphere(const PxTransform& t, const PxVec3& v, PxReal halfExtend) {
	PxTransform local(v);
	PxShape* shape = gPhysics->createShape(PxSphereGeometry(halfExtend), *gMaterial);
	shape->setQueryFilterData(collisionGroup);
	PxRigidStatic* sceneBox = gPhysics->createRigidStatic(t.transform(local));
	sceneBox->attachShape(*shape);
	sceneBox->setName("");
	gScene->addActor(*sceneBox);
	return sceneBox;
}

/*�ڴ�
halfExtend �ڴ��ײ�����İ뾶
rod_x,rod_y,rod_z ���Ӹ˵ĳ��߿�
pose ���壨���Ӹˣ��ĳ�ʼ����
velocity �ڴ��ĳ�ʼ�ٶ�*/
void createPendulum(const PxTransform& t, PxVec3 v, float halfExtend, float rod_x, float rod_y, float rod_z, PxTransform& pose,PxVec3 velocity) {
	PxTransform pos(t.transform(PxTransform(v)));
	PxRigidDynamic* actor0 = createDynamicSphere(pos, PxVec3(0, 0, 0), halfExtend);
	actor0->setMass(30.0f);
	actor0->setAngularDamping(0.f);
	//actor0->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	Pendulum* pendulum = new Pendulum("�ڴ�", actor0->getGlobalPose().p, halfExtend, actor0);
	actor0->setName("Pendulum");
	actor0->userData = pendulum;
	PxRigidDynamic* actor1 = createDynamicBox(false, pos, PxVec3(0, halfExtend + rod_y - 2.0, 0), rod_x, rod_y, rod_z, pose);
	actor1->setMass(1.0f);
	actor1->setLinearVelocity(velocity * 2400);
	actor1->setAngularDamping(0.f);
	//actor1->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	PxTransform localFrame0(PxVec3(0, halfExtend, 0));
	PxTransform localFrame1(PxVec3(0, -rod_y + 2, 0)); 
	PxFixedJoint* fixed = PxFixedJointCreate(*gPhysics, actor0, localFrame0, actor1, localFrame1);
	PxRigidStatic* actor2 = createStaticSphere(pos, PxVec3(0, halfExtend + rod_y * 2 + rod_x - 3, 0), rod_x * 2);
	actor2->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	PxTransform localFrame2(PxVec3(0, rod_y + rod_x - 1.0, 0));
	PxTransform localFrame3(PxVec3(0, 0, 0));
	PxSphericalJoint* spherical = PxSphericalJointCreate(*gPhysics, actor1, localFrame2, actor2, localFrame3);
	spherical->setLimitCone(PxJointLimitCone(PxPi / 4, PxPi / 4, 0.05f));
	spherical->setSphericalJointFlag(PxSphericalJointFlag::eLIMIT_ENABLED, true);
}

//����ƽ��·�� ���ڶ�̬����(������road��̬����)
void createPrismaticRoad(const PxTransform& t, PxVec3 v0, PxReal x0, PxReal y0, PxReal z0, PxTransform& pose0, PxVec3 v1, PxReal x1, PxReal y1, PxReal z1, PxTransform& pose1, PxJointLinearLimitPair& limits, const PxVec3& velocity = PxVec3(0)) {
	PxTransform pos(t.transform(PxTransform(v0)));
	PxRigidStatic* actor0 = createRoad(pos, PxVec3(0, 0, 0), x0, y0, z0, pose0);
	PxRigidDynamic* actor1 = createDynamicBox(false, pos, v1, x1, y1, z1, pose1, OrganType::prismaticRoad, velocity);
	//PxRigidDynamic* actor1 = createDynamicBox(false, pos, v1, x1, y1, z1, pose1, OrganType::prismaticRoad);
	PxVec3 position = actor1->getGlobalPose().p;
	PrismaticRoad* prismaticRoad = new PrismaticRoad("ƽ��·��", position, x1, y1, z1, actor1);
	actor1->userData = prismaticRoad;
	actor1->setName("PrismaticRoad");
	PxTransform localFrame0(PxVec3(0, 0, 0));
	PxTransform localFrame1(PxVec3(-v1.x, 0, -v1.z));
	PxPrismaticJoint* prismatic = PxPrismaticJointCreate(*gPhysics, actor0, localFrame0, actor1, localFrame1);
	//PxSpring spring(40.0f, 0.f);  //stiffness��dampling
	prismatic->setLimit(PxJointLinearLimitPair(limits));
	prismatic->setPrismaticJointFlag(PxPrismaticJointFlag::eLIMIT_ENABLED, true);
}

void createRotateRod(const PxTransform& t, PxVec3 v, PxReal halfExtend, PxTransform& pose, PxVec3 velocity0, PxVec3 velocity1) {
	PxTransform pos(t.transform(PxTransform(v)));
	PxRigidStatic* sphere0 = createStaticSphere(pos, PxVec3(0, 0, 0), halfExtend);
	PxReal rod0_x = 1.0;
	PxReal rod0_y = 1.0;
	PxReal rod0_z = 25.0;
	PxRigidDynamic* rod0 = createDynamicBox(false, pos, PxVec3(0, 0, 0), rod0_x, rod0_y, rod0_z, pose);
	RotateRod* rotateRod0 = new RotateRod("ת��", rod0->getGlobalPose().p, rod0_x, rod0_y, rod0_z, rod0);
	rod0->setName("RotateRod");
	rod0->userData = rotateRod0;
	rod0->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	//rod0->setMass(0.f);
	//rod0->setMassSpaceInertiaTensor(PxVec3(0.f));
	rod0->setAngularDamping(0.f);
	rod0->setCMassLocalPose(PxTransform(PxVec3(0, 0, 13)));
	//rod0->setLinearVelocity(velocity0);
	rod0->setAngularVelocity(PxVec3(0., 5., 0.));
	PxTransform localFrame0(PxVec3(0, 0, 0));
	PxSphericalJoint* spherical0 = PxSphericalJointCreate(*gPhysics, sphere0, localFrame0, rod0, localFrame0);

	PxRigidStatic* sphere1 = createStaticSphere(pos, PxVec3(0, 2*halfExtend, 0), halfExtend);
	PxReal rod1_x = 25.0;
	PxReal rod1_y = 1.0;
	PxReal rod1_z = 1.0;
	PxRigidDynamic* rod1 = createDynamicBox(false, pos, PxVec3(0, 2 * halfExtend, 0), rod1_x, rod1_y, rod1_z, pose);
	RotateRod* rotateRod1 = new RotateRod("ת��", rod1->getGlobalPose().p, rod1_x, rod1_y, rod1_z, rod1);
	rod1->setName("RotateRod");
	rod1->userData = rotateRod1;
	rod1->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	//rod1->setMass(0.f);
	//rod1->setMassSpaceInertiaTensor(PxVec3(0.f));
	rod1->setAngularDamping(0.f);
	rod1->setCMassLocalPose(PxTransform(PxVec3(13, 0, 0)));
	//rod1->setLinearVelocity(velocity1);
	rod1->setAngularVelocity(PxVec3(0., 5., 0.));
	PxSphericalJoint* spherical1 = PxSphericalJointCreate(*gPhysics, sphere1, localFrame0, rod1, localFrame0);
}

/*���ȣ���ת�ˣ�
tΪ�ø��幹�������ԭ��
vΪ�ø������ĵ�����乹��ԭ���λ��
abgularVelocity ������ת���ٶȣ���������ˮƽ��ͨ�����ٶ�PxVec3(0,y,0)��y�Ĵ�С��������ת�ٶ�*/
void createFan(const PxTransform& t, PxVec3 v, float x, float y, float z, PxVec3 angularVelocity) {
	PxTransform pos(t.transform(PxTransform(v)));
	PxRigidDynamic* fan = gPhysics->createRigidDynamic(pos);
	PxShape* shape0 = PxRigidActorExt::createExclusiveShape(*fan, PxBoxGeometry(x,y,z), *gMaterial);
	PxShape* shape1 = PxRigidActorExt::createExclusiveShape(*fan, PxBoxGeometry(z,y,x), *gMaterial);
	shape0->setQueryFilterData(collisionGroup);
	shape0->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);
	shape1->setQueryFilterData(collisionGroup);
	shape1->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);
	fan->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	fan->setAngularVelocity(angularVelocity);
	fan->setAngularDamping(0.f);
	gScene->addActor(*fan);
	fan->setMass(0.f);
	fan->setMassSpaceInertiaTensor(PxVec3(0.f));
	RotateRod* rotateRod = new RotateRod("ת��", fan->getGlobalPose().p, x,y,z, fan);
    fan->setName("RotateRod");
	fan->userData = rotateRod;
	PxRigidStatic* sphere = createStaticSphere(pos, PxVec3(0, 0, 0), 2*y);
	PxTransform localFrame(PxVec3(0, 0, 0));
	PxFixedJoint* fixed = PxFixedJointCreate(*gPhysics, fan, localFrame, sphere, localFrame);
}

/*���ȣ���ת�ˣ��ؿ�
tΪ�ø��幹�������ԭ��
vΪ�ø������ĵ�����乹��ԭ���λ��
rod_length, rod_height, rod_widthΪ�˵ĳ����ߡ���
abgularVelocity ������ת���ٶȣ���������ˮƽ��ͨ�����ٶ�PxVec3(0,y,0)��y�Ĵ�С��������ת�ٶ�*/
void createRoTateRodLevel(const PxTransform& t, PxVec3 v,float rod_length,float rod_height,float rod_width, float boxHeight,PxTransform& pose) {
	createIceRoad(t, v, rod_length, boxHeight, rod_length, pose);
	//createRotateRod(t, PxVec3(v.x, v.y + boxHeight + 2.0, v.z), 1.5, pose, PxVec3(1, 0, 0) * 70, PxVec3(0, 0, -1) * 70);
	createFan(t, PxVec3(v.x, v.y + 2 * boxHeight + 1.7f, v.z), rod_length, rod_height, rod_width, PxVec3(0, 4, 0));

	//��ת��1���ĵ�
	float rr1_x = v.x - 2 * rod_length;
	float rr1_y = v.y;
	float rr1_z = v.z + rod_length + 0.5;
	//�м�·��
	createIceRoad(t, PxVec3(rr1_x, rr1_y, v.z), rod_length, boxHeight, 2 * rod_length + 1.0, pose);
	//createRotateRod(t, PxVec3(rr1_x, rr1_y + 2 * boxHeight, rr1_z), 1.5, pose, PxVec3(1, 0, 0) * 70, PxVec3(0, 0, -1) * 70);
	createFan(t, PxVec3(rr1_x, rr1_y + 2 * boxHeight + 1.7f, rr1_z), rod_length, rod_height, rod_width, PxVec3(0, 4, 0));

	//��ת��2���ĵ�
	float rr2_x = rr1_x;
	float rr2_y = rr1_y;
	float rr2_z = v.z - rod_length - 0.5;
	//createRotateRod(t, PxVec3(rr2_x, rr2_y + 2 * boxHeight, rr2_z), 1.5, pose, PxVec3(-1, 0, 0) * 70, PxVec3(0, 0, 1) * 70);
	createFan(t, PxVec3(rr2_x, rr2_y + 2 * boxHeight + 1.7f, rr2_z), rod_length, rod_height, rod_width, PxVec3(0, -4, 0));

	//��ת��3���ĵ�
	float rr3_x = v.x - 4 * rod_length;
	float rr3_y = v.y;
	float rr3_z = v.z;
	createIceRoad(t, PxVec3(rr3_x, rr3_y, rr3_z), rod_length, boxHeight, rod_length, pose);
	//createRotateRod(t, PxVec3(rr3_x, rr3_y + 2 * boxHeight, rr3_z), 1.5, pose, PxVec3(-1, 0, 0) * 70, PxVec3(0, 0, 1) * 70);
	createFan(t, PxVec3(rr3_x, rr3_y + 2 * boxHeight + 1.7f, rr3_z), rod_length, rod_height, rod_width, PxVec3(0, -4, 0));
}

/*����ˮ��
tΪ��ˮ�ع��������ԭ��
bottomΪ��ˮ�صײ����ĵ�����乹��ԭ���λ��
poolLength, poolHeight, poolWidthΪ�˵ĳ����ߡ���
pose�����ʼ��̬*/
void createPool(const PxTransform& t, PxVec3 bottom, float poolLength, float poolHeight,float poolWidth, PxTransform& pose) {
	PxTransform pos(t.transform(PxTransform(bottom)));
	//�ײ�
	createStaticBox(pos, PxVec3(0, 0, 0), poolLength + 2.0, 1.0, poolWidth + 2.0, pose);
	//���
	createStaticBox(pos, PxVec3(1.0 + poolLength, 1.0 + poolHeight, 0), 1.0, poolHeight, poolWidth + 2.0, pose);
	//�Ҳ�
	createStaticBox(pos, PxVec3(- 1.0 - poolLength,  1.0 + poolHeight, 0), 1.0, poolHeight, poolWidth + 2.0, pose);
	//ǰ��
	createStaticBox(pos, PxVec3(0,  1.0 + poolHeight, - 1.0 - poolWidth), poolLength, poolHeight, 1.0, pose);
	//���
	createStaticBox(pos, PxVec3(0,  1.0 + poolHeight, 1.0 + poolWidth), poolLength, poolHeight, 1.0, pose);
}

/*��ת�˹ؿ���ˮ�����Ӵ��ĳ���
tΪ�ø��幹�������ԭ��
vΪ�ø������ĵ�����乹��ԭ���λ��
x,y,zΪ��ɳ��ֺ��ӵĳ����ߡ���
abgularVelocity ������ת���ٶȣ�����������ֱ��ͨ�����ٶ�PxVec3(0,0,z)��z�Ĵ�С��������ת�ٶ�*/
void createGear(const PxTransform& t, PxVec3 v, float x, float y, float z, PxVec3 angularVelocity) {
	PxTransform pos(t.transform(v));
	PxRigidDynamic* gear = gPhysics->createRigidDynamic(pos);
	PxShape* shape0 = PxRigidActorExt::createExclusiveShape(*gear, PxBoxGeometry(x, y, z), *gMaterial);
	PxShape* shape1 = PxRigidActorExt::createExclusiveShape(*gear, PxBoxGeometry(x, y, z), *gMaterial);
	//PxShape* shape2 = PxRigidActorExt::createExclusiveShape(*ferrisWheel, PxBoxGeometry(x, y, z), *gMaterial);
	shape1->setLocalPose(PxTransform(PxQuat(PxPi / 2, PxVec3(0, 0, 1))));
	//shape2->setLocalPose(PxTransform(PxQuat(2 * PxPi / 3, PxVec3(0, 0, 1))));
	shape0->setQueryFilterData(collisionGroup);
	shape1->setQueryFilterData(collisionGroup);
	//shape2->setQueryFilterData(collisionGroup);
	gear->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	gear->setAngularVelocity(angularVelocity);
	gear->setAngularDamping(0.f);
	gScene->addActor(*gear);
	gear->setMass(0.f);
	gear->setMassSpaceInertiaTensor(PxVec3(0.f));
	/*RotateRod* rotateRod = new RotateRod("ת��", fan->getGlobalPose().p, 25.0, 1.0, 1.0, fan);
	fan->setName("RotateRod");
	fan->userData = rotateRod;*/
}

/*����Ħ����
tΪ�ø��幹�������ԭ��
vΪ�ø������ĵ�����乹��ԭ���λ��
x,y,zΪ�˵ĳ����ߡ���
seatLength,seatHeight,seatWidthΪ��ת̤��ĳ��߿�
distanceΪ�Ǽ���̤����϶��С
abgularVelocity Ħ������ת���ٶȣ�����������ֱ��ͨ�����ٶ�PxVec3(0,0,z)��z�Ĵ�С��������ת�ٶ�*/
void createFerrisWheel(const PxTransform& t, PxVec3 v, float x, float y, float z, float seatLength, float seatHeight, float seatWidth, float distance, PxVec3 angularVelocity) {
	PxTransform pos(t.transform(v));
	PxRigidDynamic* ferrisWheel = gPhysics->createRigidDynamic(pos);
	PxShape* shape0 = PxRigidActorExt::createExclusiveShape(*ferrisWheel, PxBoxGeometry(x, y, z), *gMaterial);
	PxShape* shape1 = PxRigidActorExt::createExclusiveShape(*ferrisWheel, PxBoxGeometry(x, y, z), *gMaterial);
	PxShape* shape2 = PxRigidActorExt::createExclusiveShape(*ferrisWheel, PxBoxGeometry(x, y, z), *gMaterial);
	shape1->setLocalPose(PxTransform(PxQuat(PxPi / 3, PxVec3(0, 0, 1))));
	shape2->setLocalPose(PxTransform(PxQuat(2 * PxPi / 3, PxVec3(0, 0, 1))));
	shape0->setQueryFilterData(collisionGroup);
	shape1->setQueryFilterData(collisionGroup);
	shape2->setQueryFilterData(collisionGroup);
	ferrisWheel->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	ferrisWheel->setAngularVelocity(angularVelocity);
	ferrisWheel->setAngularDamping(0.f);
	gScene->addActor(*ferrisWheel);
	ferrisWheel->setMass(0.f);
	ferrisWheel->setMassSpaceInertiaTensor(PxVec3(0.f));
	PxTransform pose(PxVec3(0));
	//seat0��
	PxRigidDynamic* seat0 = createDynamicBox(false, pos, PxVec3(0.9 * x, -2.0, -z - seatWidth - distance), seatLength, seatHeight, seatWidth, pose, OrganType::prismaticRoad);
	seat0->setMass(1.0);
	seat0->setMassSpaceInertiaTensor(PxVec3(0.f));
	PxTransform fw0(PxVec3(0.9 * x, 0, -z - seatWidth - distance));
	PxTransform s0(PxVec3(0, 2.0, 0));
	PxDistanceJoint* d0 = PxDistanceJointCreate(*gPhysics, ferrisWheel, fw0, seat0, s0);
	/*d0->setMaxDistance(2.0f);
	d0->setDistanceJointFlag(PxDistanceJointFlag::eMAX_DISTANCE_ENABLED, true);*/
	//seat1��
	PxRigidDynamic* seat1 = createDynamicBox(false, pos, PxVec3(-0.9 * x, -2.0, -z - seatWidth - distance), seatLength, seatHeight, seatWidth, pose, OrganType::prismaticRoad);
	seat1->setMass(1.0);
	seat1->setMassSpaceInertiaTensor(PxVec3(0.f));
	PxTransform fw1(PxVec3(-0.9 * x, 0, -z - seatWidth - distance));
	PxTransform s1(PxVec3(0, 2.0, 0));
	PxDistanceJoint* d1 = PxDistanceJointCreate(*gPhysics, ferrisWheel, fw1, seat1, s1);
	/*d1->setMaxDistance(2.0f);
	d1->setDistanceJointFlag(PxDistanceJointFlag::eMAX_DISTANCE_ENABLED, true);*/
	//seat2����
	PxRigidDynamic* seat2 = createDynamicBox(false, pos, PxVec3(0.9 * x * cos(PxPi / 3), 0.9 * x * sin(PxPi / 3) - 2.0, -z - seatWidth - distance),seatLength, seatHeight, seatWidth, pose, OrganType::prismaticRoad);
	seat2->setMass(1.0);
	seat2->setMassSpaceInertiaTensor(PxVec3(0.f));
	PxTransform fw2(PxVec3(0.9 * x * cos(PxPi / 3), 0.9 * x * sin(PxPi / 3), -z - seatWidth - distance));
	PxTransform s2(PxVec3(0, 2.0, 0));
	PxDistanceJoint* d2 = PxDistanceJointCreate(*gPhysics, ferrisWheel, fw2, seat2, s2);
	/*d2->setMaxDistance(2.0f);
	d2->setDistanceJointFlag(PxDistanceJointFlag::eMAX_DISTANCE_ENABLED, true);*/
	//seat3����
	PxRigidDynamic* seat3 = createDynamicBox(false, pos, PxVec3(-0.9 * x * cos(PxPi / 3), 0.9 * x * sin(PxPi / 3) - 2.0, -z - seatWidth - distance), seatLength, seatHeight, seatWidth, pose, OrganType::prismaticRoad);
	seat3->setMass(1.0);
	seat3->setMassSpaceInertiaTensor(PxVec3(0.f));
	PxTransform fw3(PxVec3(-0.9 * x * cos(PxPi / 3), 0.9 * x * sin(PxPi / 3), -z - seatWidth - distance));
	PxTransform s3(PxVec3(0, 2.0, 0));
	PxDistanceJoint* d3 = PxDistanceJointCreate(*gPhysics, ferrisWheel, fw3, seat3, s3);
	/*d3->setMaxDistance(2.0f);
	d3->setDistanceJointFlag(PxDistanceJointFlag::eMAX_DISTANCE_ENABLED, true);*/
	//seat4����
	PxRigidDynamic* seat4 = createDynamicBox(false, pos, PxVec3(0.9 * x * cos(PxPi / 3), -0.9 * x * sin(PxPi / 3) - 2.0, -z - seatWidth - distance), seatLength, seatHeight, seatWidth, pose, OrganType::prismaticRoad);
	seat4->setMass(1.0);
	seat4->setMassSpaceInertiaTensor(PxVec3(0.f));
	PxTransform fw4(PxVec3(0.9 * x * cos(PxPi / 3), -0.9 * x * sin(PxPi / 3), -z - seatWidth - distance));
	PxTransform s4(PxVec3(0, 2.0, 0));
	PxDistanceJoint* d4 = PxDistanceJointCreate(*gPhysics, ferrisWheel, fw4, seat4, s4);
	/*d4->setMaxDistance(2.0f);
	d4->setDistanceJointFlag(PxDistanceJointFlag::eMAX_DISTANCE_ENABLED, true);*/
	//seat5����
	PxRigidDynamic* seat5 = createDynamicBox(false, pos, PxVec3(-0.9 * x * cos(PxPi / 3), -0.9 * x * sin(PxPi / 3) - 2.0, -z - seatWidth - distance), seatLength, seatHeight, seatWidth, pose, OrganType::prismaticRoad);
	seat5->setMass(1.0);
	seat5->setMassSpaceInertiaTensor(PxVec3(0.f));
	PxTransform fw5(PxVec3(-0.9 * x * cos(PxPi / 3), -0.9 * x * sin(PxPi / 3), -z - seatWidth - distance));
	PxTransform s5(PxVec3(0, 2.0, 0));
	PxDistanceJoint* d5 = PxDistanceJointCreate(*gPhysics, ferrisWheel, fw5, seat5, s5);
	/*d5->setMaxDistance(2.0f);
	d5->setDistanceJointFlag(PxDistanceJointFlag::eMAX_DISTANCE_ENABLED, true);*/
	//��������
	PxRigidDynamic* sphere = createDynamicSphere(pos, PxVec3(0, 0, 0), 2 * y);
	PxTransform center(PxVec3(0));
	PxFixedJointCreate(*gPhysics, ferrisWheel, center, sphere, center);
	//�ԳƹǼ�
	PxTransform pos1(pos.transform(PxTransform(PxVec3(0, 0, -2 * z - 2 * distance - 2 * seatWidth))));
	PxRigidDynamic* ferrisWheel1 = gPhysics->createRigidDynamic(pos1);
	PxShape* shape3 = PxRigidActorExt::createExclusiveShape(*ferrisWheel1, PxBoxGeometry(x, y, z), *gMaterial);
	PxShape* shape4 = PxRigidActorExt::createExclusiveShape(*ferrisWheel1, PxBoxGeometry(x, y, z), *gMaterial);
	PxShape* shape5 = PxRigidActorExt::createExclusiveShape(*ferrisWheel1, PxBoxGeometry(x, y, z), *gMaterial);
	shape4->setLocalPose(PxTransform(PxQuat(PxPi / 3, PxVec3(0, 0, 1))));
	shape5->setLocalPose(PxTransform(PxQuat(2 * PxPi / 3, PxVec3(0, 0, 1))));
	shape3->setQueryFilterData(collisionGroup);
	shape4->setQueryFilterData(collisionGroup);
	shape5->setQueryFilterData(collisionGroup);
	ferrisWheel1->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	ferrisWheel1->setAngularVelocity(angularVelocity);
	ferrisWheel1->setAngularDamping(0.f);
	gScene->addActor(*ferrisWheel1);
	ferrisWheel1->setMass(0.f);
	ferrisWheel1->setMassSpaceInertiaTensor(PxVec3(0.f));
	//��һ��������
	PxRigidDynamic* sphere1 = createDynamicSphere(pos1, PxVec3(0, 0, 0), 2 * y);
	PxFixedJointCreate(*gPhysics, ferrisWheel1, center, sphere1, center);
}

/*��ת·�ιؿ�����fan������ɣ�
tΪ�ùؿ����������ԭ��
vΪ��һ����ת�������ĵ�����乹��ԭ���λ��
x,y,zΪ���ȸ˵ĳ����ߡ���
abgularVelocity ������ת���ٶȣ���������ˮƽ��ͨ�����ٶ�PxVec3(0,y,0)��y�Ĵ�С��������ת�ٶ�
poseΪ�����ʼ��̬*/
float createFanRoadLevel(const PxTransform& t, PxVec3 v, float x, float y, float z,PxVec3 angularVelocity, PxTransform& pose) {
	PxTransform pos(t.transform(v));
	float scale = 1.15;
	createFan(pos, PxVec3(0), x, y, z, angularVelocity);
	PxVec3 fanPos1(scale * 2 * x * cos(PxHalfPi / 2), 0, scale * 2 * x * cos(PxHalfPi / 2));
	createFan(pos, fanPos1, x, y, z, -angularVelocity);
	PxVec3 fanPos2(0, 0, scale * 4 * x * cos(PxHalfPi / 2));
	createFan(pos, fanPos2, x, y, z, angularVelocity);
	return v.z + fanPos2.z;
}

void createPrismatic(const PxTransform& t, PxVec3 v,PxTransform& pose) {
	PxRigidDynamic* actor1 = createDynamicBox(false, t, v, 3.0, 1.0, 4.0, pose, OrganType::prismaticRoad,PxVec3(0, 0, 0));
	PxVec3 position = actor1->getGlobalPose().p;
	PrismaticRoad* prismaticRoad = new PrismaticRoad("ƽ��·��", position, 3.0, 1.0, 4.0, actor1);
	actor1->userData = prismaticRoad;
	actor1->setName("PrismaticRoad");
	actor1->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	actor1->setMass(0.f);
	actor1->setMassSpaceInertiaTensor(PxVec3(0.f));
}

//������Ϸ����
void createGameScene(const PxTransform& t) {
	PxTransform defaultPose(PxQuat(0, PxVec3(0, 1, 0)));  //����Ĭ��pose
	
	//���� �Ӵ��ж�������
	PxRigidStatic* ground = createStaticBox(t, PxVec3(50, 1.0, 300), 500, 1.0, 500, defaultPose,OrganType::ground);
	ground->setName("Over");

	//#Checkpoint1
	totalCheckpoint = 1;

	float r_1_l = 6.0;  //road_1_length 4
	float r_1_w = 20.0;  //road_1_width 15
	float c_1_y = 5.0 + boxHeight;  //the position of the center of road_1
	// create road_1
	createRoad(t, PxVec3(0, c_1_y, 0), r_1_l, boxHeight, r_1_w, defaultPose);
	//createRoad(t, PxVec3(0, c_1_y, 0), 100, boxHeight, 100, defaultPose);

	//��ʼλ��
	checkpoints.push_back(t.transform(PxVec3(0, c_1_y + 7.0, 0)));

	float r_2_w = 6.0; //4
	float r_2_l = 25.0; //20
	float c_2_x = r_2_l - r_1_l;
	float c_2_y = center_y(c_1_y);
	float c_2_z = r_1_w + r_2_w;
	createRoad(t, PxVec3(c_2_x, c_2_y, c_2_z), r_2_l , boxHeight, r_2_w, defaultPose);

	//���������ೡ��
	createPorp(t, PxVec3(c_2_x, c_2_y + boxHeight + 1.0, c_2_z), 1.0, 1.0, 1.0);

	//stairs
	float stairsWidth = 6.0; //4
	float stairsLength = 4.0; //2
	float center_x = stairsLength + r_2_l + c_2_x;
	float centerHeight = center_y(c_2_y);
	float center_z = c_2_z;
	for (int i = 0; i <= 5; i++) {
		createRoad(t, PxVec3(center_x, centerHeight, center_z), stairsLength, boxHeight, stairsWidth, defaultPose);
		centerHeight = center_y(centerHeight);
		center_x += 2 * stairsLength;
	}
	
	//���һ��̨�׵����ĵ㣨center_x-2*stairsLength,centerHeight-2*boxHeight,center_z��
	std::cout << "�ڴ�ǰλ����Գ���ԭ�������Ϊ:" << center_x - 2 * stairsLength << "," << centerHeight - 2 * boxHeight << "," << center_z << endl;
	
	//#Checkpoint2
	totalCheckpoint++;
	checkpoints.push_back(t.transform(PxVec3(center_x - 2 * stairsLength, centerHeight - 2 * boxHeight + 7.0, center_z)));

	//����·��
	float roadblock_length = 3.0;
	float roadblock_width = 6.0;  //4
	float rb_x = center_x - stairsLength + dx + roadblock_length;
	float rb_y = centerHeight - 2 * boxHeight;
	float rb_z = center_z;
	//createRoad(t, PxVec3(rb_x, rb_y, rb_z), roadblock_length, boxHeight, roadblock_width, defaultPose);
	for (int i = 0; i < 3; i++) {
		createRoad(t, PxVec3(rb_x, rb_y, rb_z), roadblock_length, boxHeight, roadblock_width, defaultPose);
		if (i % 2 == 0) {
			//�ڴ�����yΪ �������к��Ӹ�+halfExtend+0.5�������
			createPendulum(t, PxVec3(rb_x, rb_y + boxHeight + 5.5, rb_z), 5.0, 0.6, 10.0, 0.6, defaultPose, PxVec3(0, 0, -1));
		}
		else {
			createPendulum(t, PxVec3(rb_x, rb_y + boxHeight + 5.5, rb_z), 5.0, 0.6, 10.0, 0.6, defaultPose, PxVec3(0, 0, 1));
		}
		rb_x = rb_x + 2 * roadblock_length + dx;
	}

	//���һ������·�ε����ĵ㣨rb_x-2 * roadblock_length - dx, rb_y, rb_z��
	std::cout << "���һ������·�ε����ĵ���Գ���ԭ�������Ϊ:" << rb_x - 2 * roadblock_length - dx << "," << rb_y << "," << rb_z << endl;
	
	//��һ���ΰ������
	float seesawpos_x = rb_x - roadblock_length + 0.5 * dx;
	float seesawpos_y = rb_y;
	float seesawpos_z = rb_z;
	float sx = 5.0;
	float sy = 1.0;
	float sz = 15.0;
	PxVec3 seesawpos_v(seesawpos_x, seesawpos_y, seesawpos_z);
	float lastSeesaw_x = createSeesawLevel(t, seesawpos_v, sx, sy, sz, defaultPose);
	//���һ���ΰ�����ĵ㣨lastSeesaw_x,seesawpos_y, seesawpos_z��
	std::cout << "���һ���ΰ�����ĵ���Գ���ԭ�������Ϊ:" << lastSeesaw_x << "," << seesawpos_y << "," << seesawpos_z << endl;

	//#Checkpoint3
	totalCheckpoint++;

	//�ΰ���Ħ����֮������·��
	float r_8_l = 15.0;
	float r_8_w = 6.0;
	float c_8_x = lastSeesaw_x + sx + dx + r_8_l;
	float c_8_y = seesawpos_y;
	float c_8_z = seesawpos_z;
	createRoad(t, PxVec3(c_8_x, c_8_y, c_8_z), r_8_l, boxHeight, r_8_w, defaultPose);

	checkpoints.push_back(t.transform(PxVec3(c_8_x, c_8_y + 7.0, c_8_z)));

	//�ΰ�ؿ����Ħ����
	//�˵Ĳ���
	float fw_l = 30.0;
	float fw_h = 0.8;
	float fw_w = 0.8;
	//seat����
	float seatLength = 4.0;
	float seatHeight = 0.4;
	float seatWidth = 3.0;
	float seatDistance = 0.4;
	float fw_x = c_8_x + r_8_l + 0.75 * fw_l;
	float fw_y = c_8_y + fw_l;
	float fw_z = c_8_z + seatWidth; + seatDistance + fw_w;
	createFerrisWheel(t, PxVec3(fw_x, fw_y, fw_z), fw_l, fw_h, fw_w, seatLength, seatHeight, seatWidth, seatDistance, PxVec3(0, 0, -2));

	//#Checkpoint4
	totalCheckpoint++;

	/*ԭ�ΰ����Թ�����·��
	float r_3_l = 5.0;
	float r_3_w = 50.0;
	float c_3_x = lastSeesaw_x + sx + dx + r_3_l;
	float c_3_y = seesawpos_y;
	float c_3_z = seesawpos_z + r_3_w - 5.0;
	PxRigidStatic* r_3 = createRoad(t, PxVec3(c_3_x, c_3_y, c_3_z), r_3_l, boxHeight, r_3_w, defaultPose);
	std::cout << "����·����Գ���ԭ�������:" << c_3_x << "," << c_3_y << "," << c_3_z << endl;
	checkpoints.push_back(t.transform(PxVec3(c_3_x, c_3_y + 7.0, c_3_z)));*/

	//����Ħ��������ת·�ؿ���·��
	float r_3_l = 6.0;
	float r_3_w = 10.0;
	float c_3_x = fw_x + 0.8 * fw_l + r_3_l;
	float c_3_y = fw_y + 0.8 * fw_l;
	float c_3_z = seesawpos_z + r_3_w - 2 * seatWidth;
	PxRigidStatic* r_3 = createRoad(t, PxVec3(c_3_x, c_3_y, c_3_z), r_3_l, boxHeight, r_3_w, defaultPose);
	std::cout <<"����Ħ��������ת·�ؿ���·����Գ���ԭ�������:"<< c_3_x << "," << c_3_y << "," << c_3_z << endl;

	checkpoints.push_back(t.transform(PxVec3(c_3_x, c_3_y + 7.0, c_3_z)));

	//���������������ת·�ιؿ�
	float fan_l = 30.0;
	float fan_h = 1.0;
	float fan_w = 6.0;
	float fan0_x = c_3_x;
	float fan0_y = c_3_y;
	float fan0_z = c_3_z + r_3_w + 0.5 * dz + fan_l;
	float fan2_z = createFanRoadLevel(t, PxVec3(fan0_x, fan0_y, fan0_z), fan_l, fan_h, fan_w, PxVec3(0, 3, 0), defaultPose);
	//���һ����ת·�����ĵ㣨fan0_x,fan0_y,fan2_z��

	//#Checkpoint5
	totalCheckpoint++;

	//������ת·�ؿ����Թ���·��
	float r_7_l = 6.0;
	float r_7_w = 10.0;
	float c_7_x = fan0_x;
	float c_7_y = fan0_y;
	float c_7_z = fan2_z + r_7_w + 0.5 * dz + fan_l;
	createRoad(t, PxVec3(c_7_x, c_7_y, c_7_z), r_7_l, boxHeight, r_7_w, defaultPose);
	std::cout << "������ת·�ؿ����Թ���·����Գ���ԭ�������:" << c_7_x << "," << c_7_y << "," << c_7_z << endl;

	checkpoints.push_back(t.transform(PxVec3(c_7_x, c_7_y + 7.0, c_7_z)));

	//�Թ��߳�����ϵ��
	float scale = 0.8;
	//�Թ�С�����α߳�37*scale
	float sideLength = 37.0 * scale;
	//�Թ����ĵ�����
	/*float mazePos_x = c_3_x - 1.5 * sideLength;
	float mazePos_y = c_3_y;
	float mazePos_z = c_3_z + r_3_w + 4 * sideLength;*/
	float mazePos_x = c_7_x - 1.5 * sideLength;
	float mazePos_y = c_7_y;
	float mazePos_z = c_7_z + r_7_w + 4 * sideLength;
	PxVec3 maze_v(mazePos_x, mazePos_y, mazePos_z);
	//�Թ��ذ�߳�
	//float mazeLength = 4 * sideLength+1;
	float mazeLength =  2 * sideLength;
	//�����Թ��ذ�
	//createRoad(t, maze_v, mazeLength, boxHeight, mazeLength, defaultPose);
	createRoad(t, maze_v+PxVec3(mazeLength,0.0f, mazeLength), mazeLength, boxHeight, mazeLength, defaultPose);
	createRoad(t, maze_v+ PxVec3(mazeLength, 0.0f, -mazeLength), mazeLength, boxHeight, mazeLength, defaultPose);
	createRoad(t, maze_v+ PxVec3(-mazeLength, 0.0f, mazeLength), mazeLength, boxHeight, mazeLength, defaultPose);
	createRoad(t, maze_v+ PxVec3(-mazeLength, 0.0f, -mazeLength), mazeLength, boxHeight, mazeLength, defaultPose);

	createMaze(t, maze_v, scale, defaultPose);
	//�Թ���������
	float mazeOut_x = mazePos_x - 3.5 * sideLength;
	float mazeOut_y = mazePos_y;
	float mazeOut_z = mazePos_z + 4 * sideLength;

	//�Թ�����·��
	float r_4_l = 5.0;
	float r_4_w = 35.0;
	float c_4_x = mazeOut_x;
	float c_4_y = mazeOut_y;
	float c_4_z = mazeOut_z + r_4_w;
	createRoad(t, PxVec3(c_4_x, c_4_y, c_4_z), r_4_l, boxHeight, r_4_w, defaultPose);
	std::cout << "�Թ�����·����Գ���ԭ�������Ϊ:" <<c_4_x << "," << c_4_y << "," << c_4_z << endl;
	
	//#Checkpoint6
	totalCheckpoint++;
	checkpoints.push_back(t.transform(PxVec3(c_4_x, c_4_y + 7.0, c_4_z)));

	//¥��1
	//stairsWidth �� 4.0
	//stairsLength �� 2.0
	float center_x1 = c_4_x - r_4_l - stairsLength;
	float centerHeight1 = center_y(c_4_y);
	float center_z1 = c_4_z + r_4_w - stairsWidth;
	for (int i = 0; i <= 5; i++) {
		createRoad(t, PxVec3(center_x1, centerHeight1, center_z1), stairsLength, boxHeight, stairsWidth, defaultPose);
		centerHeight1 = center_y(centerHeight1);
		center_x1 -= 2 * stairsLength;
	}
    //���һ��̨�׵����ĵ㣨center_x1+2*stairsLength,centerHeight1-2*boxHeight,center_z1��
	
	//ƽ��·��0 prismaticRoad
	//roadblock_length : 3.0
	//roadblock_width : 4.0
	float c_5_x = center_x1 + stairsLength - roadblock_length;
	float c_5_y = centerHeight1;
	float c_5_z = center_z1;
	PxVec3 pr_v0(-2.5 * roadblock_length, 0, 0);
	PxJointLinearLimitPair limits0(-30.0, -2.5 * roadblock_length, PxSpring(20.0, 0));
	PxVec3 velocity0((-1, 0, 0) * 30);
	createPrismaticRoad(t, PxVec3(c_5_x, c_5_y, c_5_z), roadblock_length, boxHeight, roadblock_width, defaultPose, pr_v0, roadblock_length, boxHeight, roadblock_width, defaultPose, limits0, velocity0);
	std::cout << "ƽ��·��ǰ��Գ���ԭ�������Ϊ:" << c_5_x << "," << c_5_y << "," << c_5_z << endl;

	//#Checkpoint7
	totalCheckpoint++;

	//��ת��0���ĵ�
	float rod_length = 25.0;
	float rod_height = 0.5;
	float rod_width = 1.0;
	float c_6_x = c_5_x - 45.0;
	float c_6_y = c_5_y;
	float c_6_z = c_5_z - roadblock_width - dz - rod_length;
	createRoTateRodLevel(t, PxVec3(c_6_x, c_6_y, c_6_z), rod_length, rod_height, rod_width, boxHeight, defaultPose);
	std::cout << "��ת�˹ؿ�����λ������ڳ���ԭ�������Ϊ" << c_6_x + rod_length << "," << c_6_y << "," << c_6_z + rod_length << endl;

	checkpoints.push_back(t.transform(PxVec3(c_6_x + rod_length, c_6_y + 7.0, c_6_z + rod_length)));

	//���ȹؿ���ˮ�����ӳ���
	float gearLength = 8.0;
	float gearHeight = 0.4;
	float gearWidth = 16.0;
	float gear0_x = c_6_x - 5 * rod_length - dx * 0.8 - gearLength;
	float gear0_y = c_6_y;
	float gear0_z = c_6_z;
	createGear(t, PxVec3(gear0_x, gear0_y, gear0_z), gearLength, gearHeight, gearWidth, PxVec3(0, 0, 4));
	float gear1_x = gear0_x - 2 * gearLength - dx * 0.8;
	createGear(t, PxVec3(gear1_x, gear0_y, gear0_z), gearLength, gearHeight, gearWidth, PxVec3(0, 0, 4));

	//ˮ��
	float poolLength = 50.0;
	float poolHeight = 10.0;
	float poolWidth = 25.0;
	float bottom_x = gear1_x - gearLength - dx * 0.8 - poolLength - 2.0;
	float bottom_y = gear0_y + boxHeight - 2 * poolHeight - 1.0;
	float bottom_z = gear0_z;
	createPool(t, PxVec3(bottom_x, bottom_y, bottom_z), poolLength, poolHeight, poolWidth, defaultPose);
	//ˮ�صײ�������ڳ���ԭ��t��λ�� PxVec3 localPose(bottom_x,bottom_y,bottom_z)
	//ȫ��λ�� t.transform(PxTransform(localPose)).p


	//createPrismatic(t, PxVec3(-2, 20, 0), defaultPose);
	//createRoad(t, PxVec3(-4, 20, -6), 4.0, 1.0, 2.0, defaultPose);
	//createSideSeesaw(t, PxVec3(-2, 20, 0), 5.0, 1.0, 15.0, defaultPose);
	//createPlane(PxVec3(0, 0, 0), PxVec3(0, 1, 0));

}