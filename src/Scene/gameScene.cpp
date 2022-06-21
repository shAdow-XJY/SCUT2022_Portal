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

using namespace physx;

extern PxScene* gScene;
extern PxMaterial* gMaterial;
extern PxPhysics* gPhysics;
extern int primaryJumpHeight;
//�趨�������Ծ�߶ȣ��ɵ���
float maxJumpHeight = 3.0;
//������·���ӵİ��
float boxHeight = 0.4 * maxJumpHeight;
//x_distance x�᷽���Ͽ���Ծ�ļ�����ȣ��ɸ�����Ծ��ȵ���
float dx = 6.0;
//z_distance z�᷽���Ͽ���Ծ�ļ�����ȣ��ɸ�����Ծ��ȵ���
float dz = 6.0;


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
	//�Թ����½�����
	float x = v.x + 4 * 37 * scale;
	float z = v.z - 4 * 37 * scale;

	//�Թ��ŵ�����
	float door_x;
	float door_y = v.y + boxHeight + 0.5 + 10 * scale;
	float door_z;

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
	PxRigidStatic* actor1 = createStaticBox(pos, PxVec3(-(x+y+0.5), 0, 0), y, y, y, pose, OrganType::seesawbox);
	createStaticBox(pos, PxVec3(x+y+0.5, 0, 0), y, y, y, pose, OrganType::seesawbox);
	PxTransform localFrame0(PxVec3(0, 0, 0));
	PxTransform localFrame1(PxVec3(x+y+0.5, 0, 0));
	PxRevoluteJoint* revolute = PxRevoluteJointCreate(*gPhysics, actor0, localFrame0, actor1, localFrame1);
	//revolute->setLocalPose(PxJointActorIndex::Enum::eACTOR0, PxTransform(PxVec3(0, 0, 0), PxQuat(1.75 * PxHalfPi, PxVec3(1, 0, 0))));
	seesaw->attachRevolute(revolute);
	return revolute;
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
	//�� �� �� ��
	/*PxRigidDynamic* rod0 = createDynamicBox(false, pos, PxVec3(0, 0, 13), 1.0, 1.0, 12.0, pose);
	rod0->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	rod0->setAngularDamping(0.f);
	rod0->setLinearVelocity(PxVec3(1, 0, 0) * 300);
	PxTransform localFrame00(PxVec3(0, 0, 1));
	PxTransform localFrame01(PxVec3(0, 0, -12));
	PxSphericalJoint* spherical0 = PxSphericalJointCreate(*gPhysics, sphere, localFrame00, rod0, localFrame01);
	PxRigidDynamic* rod1 = createDynamicBox(false, pos, PxVec3(13, 0, 0), 12.0, 1.0, 1.0, pose);
	rod1->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	rod1->setAngularDamping(0.f);
	rod1->setLinearVelocity(PxVec3(0, 0, 1) * 300);
	PxTransform localFrame10(PxVec3(1, 0, 0));
	PxTransform localFrame11(PxVec3(-12, 0, 0));
	PxSphericalJoint* spherical1 = PxSphericalJointCreate(*gPhysics, sphere, localFrame10, rod1, localFrame11);
	PxRigidDynamic* rod2 = createDynamicBox(false, pos, PxVec3(-13, 0, 0), 12.0, 1.0, 1.0, pose);
	rod2->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	rod2->setAngularDamping(0.f);
	rod2->setLinearVelocity(PxVec3(0, 0, -1) * 300);
	PxTransform localFrame20(PxVec3(-1, 0, 0));
	PxTransform localFrame21(PxVec3(12, 0, 0));
	PxSphericalJoint* spherical2 = PxSphericalJointCreate(*gPhysics, sphere, localFrame20, rod2, localFrame21);
	PxRigidDynamic* rod3 = createDynamicBox(false, pos, PxVec3(0, 0, -13), 1.0, 1.0, 12.0, pose);
	rod3->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	rod3->setAngularDamping(0.f);
	rod3->setLinearVelocity(PxVec3(-1, 0, 0) * 300);
	PxTransform localFrame30(PxVec3(0, 0, -1));
	PxTransform localFrame31(PxVec3(0, 0, 12));
	PxSphericalJoint* spherical3 = PxSphericalJointCreate(*gPhysics, sphere, localFrame30, rod3, localFrame31);*/

	//�� �� ��
	/*PxRigidDynamic* rod0 = createDynamicBox(false, pos, PxVec3(0, 0, 0), 1.0, 1.0, 25.0, pose);
	rod0->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	rod0->setAngularDamping(0.f);
	rod0->setCMassLocalPose(PxTransform(PxVec3(0, 0, 13)));
	rod0->setLinearVelocity(PxVec3(-1, 0, 0) * 300);
	PxTransform localFrame0(PxVec3(0, 0, 0));
	PxSphericalJoint* spherical = PxSphericalJointCreate(*gPhysics, sphere, localFrame0, rod0, localFrame0);
	PxRigidDynamic* rod1 = createDynamicBox(false, pos, PxVec3(13, 0, 0), 12.0, 1.0, 1.0, pose);
	rod1->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	rod1->setAngularDamping(0.f);
	rod1->setLinearVelocity(PxVec3(0, 0, 1) * 300);
	PxTransform localFrame1(PxVec3(1, 0, 0));
	PxTransform localFrame2(PxVec3(-12, 0, 0));
	PxFixedJoint* fixed0 = PxFixedJointCreate(*gPhysics, rod0, localFrame1, rod1, localFrame2);
	PxRigidDynamic* rod2 = createDynamicBox(false, pos, PxVec3(-13, 0, 0), 12.0, 1.0, 1.0, pose);
	rod2->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	rod2->setAngularDamping(0.f);
	rod2->setLinearVelocity(PxVec3(0, 0, -1) * 300);
	PxTransform localFrame3(PxVec3(-1, 0, 0));
	PxTransform localFrame4(PxVec3(12, 0, 0));
	PxFixedJoint* fixed1 = PxFixedJointCreate(*gPhysics, rod0, localFrame3, rod2, localFrame4);
	PxSphericalJoint* spherical0 = PxSphericalJointCreate(*gPhysics, sphere, localFrame0, rod0, localFrame0);
	PxSphericalJoint* spherical1 = PxSphericalJointCreate(*gPhysics, sphere, localFrame1, rod1, localFrame2);
	PxSphericalJoint* spherical2 = PxSphericalJointCreate(*gPhysics, sphere, localFrame3, rod2, localFrame4);*/
}

/*���ȣ���ת�ˣ�
tΪ�ø��幹�������ԭ��
vΪ�ø������ĵ�����乹��ԭ���λ��
abgularVelocity ������ת���ٶȣ���������ˮƽ��ͨ�����ٶ�PxVec3(0,y,0)��y�Ĵ�С��������ת�ٶ�*/
void createFan(const PxTransform& t, PxVec3 v, PxVec3 angularVelocity) {
	PxTransform pos(t.transform(PxTransform(v)));
	PxRigidDynamic* fan = gPhysics->createRigidDynamic(pos);
	PxShape* shape0 = PxRigidActorExt::createExclusiveShape(*fan, PxBoxGeometry(1.0, 1.0, 25.0), *gMaterial);
	PxShape* shape1 = PxRigidActorExt::createExclusiveShape(*fan, PxBoxGeometry(25.0, 1.0, 1.0), *gMaterial);
	shape0->setQueryFilterData(collisionGroup);
	shape1->setQueryFilterData(collisionGroup);
	fan->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	fan->setAngularVelocity(angularVelocity);
	fan->setAngularDamping(0.f);
	gScene->addActor(*fan);
	fan->setMass(0.f);
	fan->setMassSpaceInertiaTensor(PxVec3(0.f));
	RotateRod* rotateRod = new RotateRod("ת��", fan->getGlobalPose().p, 25.0, 1.0, 1.0, fan);
    fan->setName("RotateRod");
	fan->userData = rotateRod;
	PxRigidStatic* sphere = createStaticSphere(pos, PxVec3(0, 0, 0), 1.8);
	PxTransform localFrame(PxVec3(0, 0, 0));
	PxFixedJoint* fixed = PxFixedJointCreate(*gPhysics, fan, localFrame, sphere, localFrame);
}


void createRoTateRodLevel(const PxTransform& t, PxVec3 v,float rod_length,float boxHeight,PxTransform& pose) {
	createRoad(t, v, rod_length, boxHeight, rod_length, pose);
	//createRotateRod(t, PxVec3(v.x, v.y + boxHeight + 2.0, v.z), 1.5, pose, PxVec3(1, 0, 0) * 70, PxVec3(0, 0, -1) * 70);
	createFan(t, PxVec3(v.x, v.y + 2 * boxHeight, v.z), PxVec3(0, 4, 0));

	//��ת��1���ĵ�
	float rr1_x = v.x - 2 * rod_length;
	float rr1_y = v.y;
	float rr1_z = v.z + rod_length + 0.5;
	//�м�·��
	createRoad(t, PxVec3(rr1_x, rr1_y, v.z), rod_length, boxHeight, 2 * rod_length + 1.0, pose);
	//createRotateRod(t, PxVec3(rr1_x, rr1_y + 2 * boxHeight, rr1_z), 1.5, pose, PxVec3(1, 0, 0) * 70, PxVec3(0, 0, -1) * 70);
	createFan(t, PxVec3(rr1_x, rr1_y + boxHeight + 2.0, rr1_z), PxVec3(0, 4, 0));

	//��ת��2���ĵ�
	float rr2_x = rr1_x;
	float rr2_y = rr1_y;
	float rr2_z = v.z - rod_length - 0.5;
	//createRotateRod(t, PxVec3(rr2_x, rr2_y + 2 * boxHeight, rr2_z), 1.5, pose, PxVec3(-1, 0, 0) * 70, PxVec3(0, 0, 1) * 70);
	createFan(t, PxVec3(rr2_x, rr2_y + boxHeight + 2.0, rr2_z), PxVec3(0, -4, 0));

	//��ת��3���ĵ�
	float rr3_x = v.x - 4 * rod_length;
	float rr3_y = v.y;
	float rr3_z = v.z;
	createRoad(t, PxVec3(rr3_x, rr3_y, rr3_z), rod_length, boxHeight, rod_length, pose);
	//createRotateRod(t, PxVec3(rr3_x, rr3_y + 2 * boxHeight, rr3_z), 1.5, pose, PxVec3(-1, 0, 0) * 70, PxVec3(0, 0, 1) * 70);
	createFan(t, PxVec3(rr3_x, rr3_y + boxHeight + 2.0, rr3_z), PxVec3(0, -4, 0));
}

//����ˮ��
void createPool(const PxTransform& t, PxVec3 bottom, float poolLength, float poolHeight, float poolWidth, PxTransform& pose) {
	PxTransform pos(t.transform(PxTransform(bottom)));
	//�ײ�
	createStaticBox(pos, PxVec3(0, 0, 0), poolLength + 2.0, 1.0, poolWidth + 2.0, pose);
	//���
	createStaticBox(pos, PxVec3(1.0 + poolLength, 1.0 + poolHeight, 0), 1.0, poolHeight, poolWidth + 2.0, pose);
	//�Ҳ�
	createStaticBox(pos, PxVec3(-1.0 - poolLength, 1.0 + poolHeight, 0), 1.0, poolHeight, poolWidth + 2.0, pose);
	//ǰ��
	createStaticBox(pos, PxVec3(0, 1.0 + poolHeight, -1.0 - poolWidth), poolLength, poolHeight, 1.0, pose);
	//���
	createStaticBox(pos, PxVec3(0, 1.0 + poolHeight, 1.0 + poolWidth), poolLength, poolHeight, 1.0, pose);
}

//��������
void createParticles(PxVec3 v)
{
	// set immutable properties.
	PxU32 maxParticles = 2048;
	bool perParticleRestOffset = false;

	// create particle system in PhysX SDK
	PxParticleSystem* ps = gPhysics->createParticleSystem(maxParticles, perParticleRestOffset);

	PxParticleExt::IndexPool* indexPool = PxParticleExt::createIndexPool(maxParticles);


	PxParticleCreationData particleCreationData;
	particleCreationData.numParticles = 1024;
	PxU32 numAllocated = indexPool->allocateIndices(2048, PxStrideIterator<PxU32>(&particleCreationData.numParticles));
	particleCreationData.indexBuffer = PxStrideIterator<const PxU32>(&numAllocated);
	particleCreationData.positionBuffer = PxStrideIterator<const PxVec3>(&v);

	// create particles in *PxParticleSystem* ps
	bool success = ps->createParticles(particleCreationData);

	// lock SDK buffers of *PxParticleSystem* ps for reading
	PxParticleReadData* rd = ps->lockParticleReadData();

	// access particle data from PxParticleReadData
	if (rd)
	{
		PxStrideIterator<const PxParticleFlags> flagsIt(rd->flagsBuffer);
		PxStrideIterator<const PxVec3> positionIt(rd->positionBuffer);

		for (unsigned i = 0; i < rd->validParticleRange; ++i, ++flagsIt, ++positionIt)
		{
			if (*flagsIt & PxParticleFlag::eVALID)
			{
				// access particle position
				const PxVec3& position = *positionIt;
				std::cout << "���ӵ�λ�ã�" << position.x << std::endl;
			}
		}

		// return ownership of the buffers back to the SDK
		rd->unlock();
	}


	// add particle system to scene, in case creation was successful
	if (ps)
		gScene->addActor(*ps);

}


//������Ϸ����
void createGameScene(const PxTransform& t) {
	time_t startTime = time(NULL);
	PxTransform defaultPose(PxQuat(0, PxVec3(0, 1, 0)));  //����Ĭ��pose

	float r_1_l = 4.0;  //road_1_length 4 100
	float r_1_w = 15.0;  //road_1_width 8 100
	float c_1_y = boxHeight;  //the position of the center of road_1
	// create road_1
	createIceRoad(t, PxVec3(0, c_1_y, 0), r_1_l, boxHeight, r_1_w, defaultPose);

	float r_2_w = 4.0;
	float r_2_l = 20.0;
	float c_2_x = r_2_l - r_1_l;
	float c_2_y = center_y(c_1_y);
	float c_2_z = r_1_w + r_2_w;
	createRoad(t, PxVec3(c_2_x, c_2_y, c_2_z), r_2_l , boxHeight, r_2_w, defaultPose);

	//���������ೡ��
	createPorp(t, PxVec3(c_2_x, c_2_y + 2.5, c_2_z), boxHeight, boxHeight, boxHeight);

	//stairs
	float stairsWidth = 4.0;
	float stairsLength = 2.0;
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
	
	//����·��
	float roadblock_length = 3.0;
	float roadblock_width = 4.0;
	float rb_x = center_x - 2 * stairsLength + dx + roadblock_length;
	float rb_y = centerHeight - 2 * boxHeight;
	float rb_z = center_z;
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

	//�����ΰ�ؿ����Թ��ؿ���·��
	float r_3_l = 5.0;
	float r_3_w = 50.0;
	float c_3_x = lastSeesaw_x + sx + dx + r_3_l;
	float c_3_y = seesawpos_y;
	float c_3_z = seesawpos_z + r_3_w - 5.0;
	PxRigidStatic* r_3 = createRoad(t, PxVec3(c_3_x, c_3_y, c_3_z), r_3_l, boxHeight, r_3_w, defaultPose);
	std::cout <<"����·����Գ���ԭ�������:"<< c_3_x << "," << c_3_y << "," << c_3_z << endl;

	//�Թ��߳�����ϵ��
	float scale = 0.8;
	//�Թ�С�����α߳�37*scale
	float sideLength = 37.0 * scale;
	//�Թ����ĵ�����
	float mazePos_x = c_3_x - 1.5 * sideLength;
	float mazePos_y = c_3_y;
	float mazePos_z = c_3_z + r_3_w + 4 * sideLength;
	PxVec3 maze_v(mazePos_x, mazePos_y, mazePos_z);
	//�Թ��ذ�߳�
	float mazeLength = 4 * sideLength+1;
	//�����Թ��ذ�
	createRoad(t, maze_v, mazeLength, boxHeight, mazeLength, defaultPose);
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

	//��ת��0���ĵ�
	float rod_length = 25.0;
	float c_6_x = c_5_x - 45.0;
	float c_6_y = c_5_y;
	float c_6_z = c_5_z - roadblock_width - dz - rod_length;
	createRoTateRodLevel(t, PxVec3(c_6_x, c_6_y, c_6_z), rod_length, boxHeight, defaultPose);
	std::cout << "��ת�˹ؿ�����λ������ڳ���ԭ�������Ϊ" << c_6_x + rod_length << "," << c_6_y << "," << c_6_z + rod_length << endl;

	//���ȹؿ���ˮ������·��
	float r_7_l = 20.0;
	float r_7_w = 5.0;
	float c_7_x = c_6_x - 5 * rod_length - r_7_l;
	float c_7_y = c_6_y;
	float c_7_z = c_6_z;
	createRoad(t, PxVec3(c_7_x, c_7_y, c_7_z), r_7_l, boxHeight, r_7_w, defaultPose);

	//ˮ��
	float poolLength = 50.0;
	float poolHeight = 10.0;
	float poolWidth = 25.0;
	/*float bottom_x = c_6_x - 5 * rod_length - poolLength - 2.0;
	float bottom_y = c_6_y + boxHeight - 2 * poolHeight - 1.0;
	float bottom_z = c_6_z;*/
	float bottom_x = c_7_x - r_7_l - poolLength - 2.0;
	float bottom_y = c_7_y + boxHeight - 2 * poolHeight - 1.0;
	float bottom_z = c_7_z;
	createPool(t, PxVec3(bottom_x, bottom_y, bottom_z), poolLength, poolHeight, poolWidth, defaultPose);
	//ˮ�صײ�������ڳ���ԭ��t��λ�� PxVec3 localPose(bottom_x,bottom_y,bottom_z)
	//ȫ��λ�� t.transform(PxTransform(localPose)).p
	createParticles(t.transform(PxTransform(PxVec3(bottom_x, bottom_y, bottom_z))).p);
	createFan(t, PxVec3(-50, 40, 20), PxVec3(0, 5, 0));
	createPlane(PxVec3(0, 0, 0), PxVec3(0, 1, 0));

}