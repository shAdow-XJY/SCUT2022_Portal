#include <ctype.h>
#include "PxPhysicsAPI.h"
#include "../Block/Block.h"
#include<vector>
#include<iostream>

using namespace physx;

extern PxScene* gScene;
extern PxMaterial* gMaterial;
extern PxPhysics* gPhysics;
extern int primaryJumpHeight;
float maxJumpHeight = 3.0;
float boxHeight = 0.4 * maxJumpHeight;

static int floorCount = 0;

PxRigidStatic* createStaticBox(const PxTransform& t, const PxVec3& v, PxReal x, PxReal y, PxReal z, PxTransform& pose) {
	//����ʹ�ö�̬�̳и�Block������Զ�������
	Block* floor = new Block(y, "�ذ�"+floorCount);
	PxTransform local(v);
	PxShape* shape = gPhysics->createShape(PxBoxGeometry(x, y, z), *gMaterial);
	//��ײ���Ĺ�����
	shape->setQueryFilterData(collisionGroup);
	shape->setLocalPose(pose);
	PxRigidStatic* sceneBox = gPhysics->createRigidStatic(t.transform(local));
	sceneBox->attachShape(*shape);
	sceneBox->userData = floor;
	sceneBox->setName("Ground");
	gScene->addActor(*sceneBox);
	return sceneBox;
}


//���������࣬����ֻ��Block(BlockType::prop)�������ã����޸�
void createPorp(const PxTransform& t, const PxVec3& v, PxReal x, PxReal y, PxReal z) {
	//����ʹ�ö�̬�̳и�Block������Զ�������
	Block* floor = new Block(y, "�ذ�" + floorCount,BlockType::prop);
	PxTransform local(v);
	PxShape* shape = gPhysics->createShape(PxBoxGeometry(x, y, z), *gMaterial);
	//��ײ���Ĺ�����
	shape->setQueryFilterData(collisionGroup);
	//setupFiltering(shape, FilterGroup::ePIG, FilterGroup::eBIRD);
	PxRigidStatic* sceneBox = gPhysics->createRigidStatic(t.transform(local));
	sceneBox->attachShape(*shape);
	sceneBox->userData = floor;
	sceneBox->setName("Ground");
	gScene->addActor(*sceneBox);
}



PxRigidDynamic* createDynamicBox(bool kinematic, const PxTransform& t, const PxVec3& v, PxReal x, PxReal y, PxReal z, PxTransform& pose, const PxVec3& velocity = PxVec3(0)) {
	Block* floor = new Block(y, "�ذ�" + floorCount);
	PxTransform local(v);
	PxShape* shape = gPhysics->createShape(PxBoxGeometry(x, y, z), *gMaterial);
	shape->setQueryFilterData(collisionGroup);
	shape->setLocalPose(pose);
	PxRigidDynamic* sceneBox = gPhysics->createRigidDynamic(t.transform(local));
	sceneBox->attachShape(*shape);
	sceneBox->setAngularDamping(10.0f);
	sceneBox->setLinearVelocity(velocity);
	PxRigidBodyExt::updateMassAndInertia(*sceneBox, 1.0f);
	if (kinematic) {
		sceneBox->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
	}
	sceneBox->userData = floor;
	sceneBox->setName("Ground");
	gScene->addActor(*sceneBox);
	return sceneBox;
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
	plane->setName("over"); //over��ʾ��ɫ�Ӵ�������
	gScene->addActor(*plane);
	
}

float center_y(float y) {
	return y + 2 * boxHeight;
}

//tΪ����������ԭ�� vΪ�������ĵ���Գ���ԭ���λ�� scaleΪ�ŵ�����ϵ��
//�ڶ�������Ӧ��ΪPxVec3 v(v_x, �������к��ӵĸ�+10*scale��v_z)
//�������е�
PxRevoluteJoint* createFrontDoor(const PxTransform& t, PxVec3 v, float scale, PxTransform& pose, PxJointAngularLimitPair& limits) {
	PxTransform pos(t.transform(PxTransform(v)));//-17.8  -18.56 -18.561815
	PxRigidDynamic* actor0 = createDynamicBox(false, pos, PxVec3(6.5 * scale, -18.56 * scale, 0), 10 * scale, 5 * scale, 1 * scale, pose);
	PxRigidStatic* actor1 = createStaticBox(pos, PxVec3(0, 0, 0), 6 * scale, 10 * scale, 1 * scale, pose);
	createStaticBox(pos, PxVec3(23 * scale, 0, 0), 6 * scale, 10 * scale, 1 * scale, pose);
	PxTransform localFrame0(PxVec3(0, 5 * scale, 0));
	PxTransform localFrame1(PxVec3(6.5 * scale,0, 0));
	PxRevoluteJoint* revolute = PxRevoluteJointCreate(*gPhysics, actor0, localFrame0, actor1, localFrame1);
	revolute->setLimit(limits);
	revolute->setRevoluteJointFlag(PxRevoluteJointFlag::eLIMIT_ENABLED, true);
	revolute->setLocalPose(PxJointActorIndex::Enum::eACTOR1, PxTransform(PxVec3(6.5 * scale, 0, 0), PxQuat(PxHalfPi, PxVec3(0, 0, 1))));
	return revolute;
}


// joint���е�
PxRevoluteJoint* createSideDoor(const PxTransform& t, PxVec3 v, float scale, PxTransform& pose, PxJointAngularLimitPair& limits) {
	PxTransform pos(t.transform(PxTransform(v)));
	PxRigidDynamic* actor0 = createDynamicBox(false, pos, PxVec3(0, 0, 11.5 * scale), 10 * scale, 1 * scale, 5 * scale, pose);
	PxRigidStatic* actor1 = createStaticBox(pos, PxVec3(0, 0, 0), 1 * scale, 10 * scale, 6 * scale, pose);
	createStaticBox(pos, PxVec3(0, 0, 23 * scale), 1 * scale, 10 * scale, 6 * scale, pose);
	PxTransform localFrame0(PxVec3(0, 0, -5 * scale));
	PxTransform localFrame1(PxVec3(0,0, 6.5 * scale));
	PxRevoluteJoint* revolute = PxRevoluteJointCreate(*gPhysics, actor0, localFrame0, actor1, localFrame1);
	/*PxJointAngularLimitPair limitPair(-PxPi / 4, PxPi / 4, 0.1f);
	limitPair.stiffness = 7.0f;
	limitPair.damping = 100.0f;
	revolute->setLimit(limitPair);*/
	revolute->setLimit(limits);
	revolute->setRevoluteJointFlag(PxRevoluteJointFlag::eLIMIT_ENABLED, true);
	revolute->setLocalPose(PxJointActorIndex::Enum::eACTOR1, PxTransform(PxVec3(0, 0, 6.5 * scale), PxQuat(PxHalfPi, PxVec3(0, 0, 1))));
	return revolute;
}

//tΪ����������ԭ�� vΪ���ΰ�������Գ���ԭ���λ�� x��y��zΪ����ĳ��߿�
//�����ΰ壨��x����ʱ����ת�� 
//pose�����ΰ���x�����ת��
PxRevoluteJoint* createFrontSeesaw(const PxTransform& t, PxVec3 v, float x, float y, float z, PxTransform& pose) {
	PxTransform pos(t.transform(PxTransform(v)));
	PxTransform defaultPose(PxVec3(0, 0, 0), PxQuat(0, PxVec3(0, 1, 0)));
	PxRigidDynamic* actor0 = createDynamicBox(false, pos, PxVec3(0, 0, 0), x, y, z, pose);
	PxRigidStatic* actor1 = createStaticBox(pos, PxVec3(-(x + y + 0.5), 0, 0), y, y, y, defaultPose);
	createStaticBox(pos, PxVec3(x + y + 0.5, 0, 0), y, y, y, defaultPose);
	PxTransform localFrame0(PxVec3(0, 0, 0));
	PxTransform localFrame1(PxVec3(x + y + 0.5, 0, 0));
	PxRevoluteJoint* revolute = PxRevoluteJointCreate(*gPhysics, actor0, localFrame0, actor1, localFrame1);
	//revolute->setLocalPose(PxJointActorIndex::Enum::eACTOR0, PxTransform(PxVec3(0, 0, 0), PxQuat(1.75 * PxHalfPi, PxVec3(1, 0, 0))));
	return revolute;
}


void createGameScene(const PxTransform& t) {
	PxTransform defaultPose(PxQuat(0, PxVec3(0, 1, 0)));

	float r_1_l = 100.0;  //road_1_length 4
	float r_1_w = 100.0;  //road_1_width 8
	float c_1_y = boxHeight;  //the position of the center of road_1
	// create road_1
	createStaticBox(t, PxVec3(0, c_1_y, 0), r_1_l, boxHeight, r_1_w, defaultPose);

	float r_2_w = 2.0;
	float r_2_l = 20.0;
	float c_2_x = r_2_l - r_1_l;
	float c_2_y = center_y(c_1_y);
	float c_2_z = r_1_l + r_2_w;
	createStaticBox(t, PxVec3(c_2_x, c_2_y, c_2_z), r_2_l , boxHeight, r_2_w, defaultPose);

	//���������ೡ��
	createPorp(t, PxVec3(c_2_x, c_2_y + 2.0, c_2_z), r_2_l, boxHeight, r_2_w);
	createPorp(t, PxVec3(c_2_x, c_2_y + 2.5, c_2_z), r_2_l, boxHeight, r_2_w);

	//stairs
	float stairsWidth = 2.0;
	float stairsLength = 1.0;
	float center_x = stairsLength + r_2_l + c_2_x;
	float centerHeight = center_y(c_2_y);
	float center_z = c_2_z;
	for (int i = 0; i <= 5; i++) {
		createStaticBox(t, PxVec3(center_x, centerHeight, center_z), stairsLength, boxHeight, stairsWidth, defaultPose);
		centerHeight = center_y(centerHeight);
		center_x += 2 * stairsLength;
	}
	
	PxTransform pose1(PxQuat(-PxHalfPi/3, PxVec3(1, 0, 0)));
	//�ΰ� PxVec3 v�ĵڶ�������Ӧ��Ϊ �������к��ӵĸ�+ ��ô���z����
	createFrontSeesaw(t, PxVec3(-30, 15 + 2 * boxHeight, -10), 5, 1, 15, pose1);

	PxJointAngularLimitPair close(-PxPi / 130, 0, 0.01f);
	PxJointAngularLimitPair open(-PxPi/2, PxPi/2, 0.01f);
	createFrontDoor(t, PxVec3(-20, 8.5 + 2 * boxHeight, 10), 0.8, defaultPose, open);
	createSideDoor(t, PxVec3(-20 + 30 * 0.8, 8.5 + 2 * boxHeight, 10 + 7 * 0.8), 0.8, defaultPose, close);
	createFrontDoor(t, PxVec3(-20, 8.5 + 2 * boxHeight, 10+37*0.8), 0.8, defaultPose, close);
	createSideDoor(t, PxVec3(-20-7*0.8, 8.5 + 2 * boxHeight, 10+7*0.8), 0.8, defaultPose, open);

	createPlane(PxVec3(0, 0, 0), PxVec3(0, 1, 0));
}