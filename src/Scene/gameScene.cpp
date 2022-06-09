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
	//可以使用多态继承该Block类添加自定义数据
	Block* floor = new Block(y, "地板"+floorCount);
	PxTransform local(v);
	PxShape* shape = gPhysics->createShape(PxBoxGeometry(x, y, z), *gMaterial);
	//碰撞检测的过滤组
	shape->setQueryFilterData(collisionGroup);
	shape->setLocalPose(pose);
	PxRigidStatic* sceneBox = gPhysics->createRigidStatic(t.transform(local));
	sceneBox->attachShape(*shape);
	sceneBox->userData = floor;
	sceneBox->setName("Ground");
	gScene->addActor(*sceneBox);
	return sceneBox;
}


//创建道具类，区别只在Block(BlockType::prop)，试验用，可修改
void createPorp(const PxTransform& t, const PxVec3& v, PxReal x, PxReal y, PxReal z) {
	//可以使用多态继承该Block类添加自定义数据
	Block* floor = new Block(y, "地板" + floorCount,BlockType::prop);
	PxTransform local(v);
	PxShape* shape = gPhysics->createShape(PxBoxGeometry(x, y, z), *gMaterial);
	//碰撞检测的过滤组
	shape->setQueryFilterData(collisionGroup);
	//setupFiltering(shape, FilterGroup::ePIG, FilterGroup::eBIRD);
	PxRigidStatic* sceneBox = gPhysics->createRigidStatic(t.transform(local));
	sceneBox->attachShape(*shape);
	sceneBox->userData = floor;
	sceneBox->setName("Ground");
	gScene->addActor(*sceneBox);
}



PxRigidDynamic* createDynamicBox(bool kinematic, const PxTransform& t, const PxVec3& v, PxReal x, PxReal y, PxReal z, PxTransform& pose, const PxVec3& velocity = PxVec3(0)) {
	Block* floor = new Block(y, "地板" + floorCount);
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
	/*PxTransformFromPlaneEquation()将平面方程转换成等价的transform。
	PxPlaneEquationFromTransform()提供相反的转换。
	PxPlaneGeometry没有attribute，因为形状的位置完全定义了平面的碰撞体积。
	PxPlaneGeometry的shape只能为静态actor创建。*/
	PxPlane p(point, normal);
	PxShape* planeShape = gPhysics->createShape(PxPlaneGeometry(), *gMaterial);	
	PxRigidStatic* plane = gPhysics->createRigidStatic(PxTransformFromPlaneEquation(p));
	plane->attachShape(*planeShape);
	plane->setName("over"); //over表示角色接触就死亡
	gScene->addActor(*plane);
	
}

float center_y(float y) {
	return y + 2 * boxHeight;
}

//t为场景构建的原点 v为该门中心点相对场景原点的位置 scale为门的缩放系数
//第二个参数应该为PxVec3 v(v_x, 底下所有盒子的高+10*scale，v_z)
void createDoor(const PxTransform& t, PxVec3 v, float scale, PxTransform& pose) {
	PxTransform pos(t.transform(PxTransform(v)));
	PxRigidDynamic* actor0 = createDynamicBox(false, pos, PxVec3(10.5 * scale, -5 * scale, 0), 10 * scale, 5 * scale, 1 * scale, pose);
	PxRigidStatic* actor1 = createStaticBox(pos, PxVec3(0, 0, 0), 1 * scale, 10 * scale, 1 * scale, pose);
	PxTransform localFrame0(PxVec3(-10 * scale, 5 * scale, 0));
	PxTransform localFrame1(PxVec3(1.5 * scale, -10*scale, 0));
	PxRevoluteJoint* revolute = PxRevoluteJointCreate(*gPhysics, actor0, localFrame0, actor1, localFrame1);
	/*PxJointAngularLimitPair limitPair(-PxPi / 4, PxPi / 4, 0.1f);
	limitPair.stiffness = 100.0f;
	limitPair.damping = 20.0f;
	revolute->setLimit(limitPair);*/
	revolute->setLimit(PxJointAngularLimitPair(-PxPi / 4, PxPi / 4, 0.1f));
	revolute->setRevoluteJointFlag(PxRevoluteJointFlag::eLIMIT_ENABLED, true);
	revolute->setLocalPose(PxJointActorIndex::Enum::eACTOR1, PxTransform(PxVec3(1.5 * scale, -10 * scale, 0), PxQuat(PxHalfPi, PxVec3(0, 0, 1))));
}

//t为场景构建的原点 v为该跷板中心相对场景原点的位置 x、y、z为长板的长高宽
//跷板 PxVec3 v的第二个参数应该为 底下所有盒子的高+ y
void createSeesaw(const PxTransform& t,PxVec3 v,float x, float y, float z, PxTransform& pose) {
	PxTransform pos(t.transform(PxTransform(v)));//PxVec3(-20, 10 + 2 * boxHeight, 0)
	PxRigidDynamic* actor0 = createDynamicBox(false, pos, PxVec3(0, 0, 0), x, y, z, pose);//5 10 1
	PxRigidStatic* actor1 = createStaticBox(pos, PxVec3(-(x+z+1), 0, 0), z, z, z, pose);
	createStaticBox(pos, PxVec3(x+z+1, 0, 0), z, z, z, pose);
	PxTransform localFrame0(PxVec3(-(x+0.5), 0, 0));
	PxTransform localFrame1(PxVec3(z+0.5, 0, 0));
	PxRevoluteJoint* revolute = PxRevoluteJointCreate(*gPhysics, actor0, localFrame0, actor1, localFrame1);
	PxJointAngularLimitPair limitPair(-PxPi / 4, PxPi / 4, 2.0f);
	limitPair.damping = 0.5;
	revolute->setLimit(limitPair);
	revolute->setRevoluteJointFlag(PxRevoluteJointFlag::eLIMIT_ENABLED, true);
	revolute->setLocalPose(PxJointActorIndex::Enum::eACTOR1, PxTransform(PxVec3(z+0.5, 0, 0), PxQuat(1.75 * PxHalfPi, PxVec3(1, 0, 0))));
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

	//创建道具类场景
	createPorp(t, PxVec3(c_2_x, c_2_y + 2.0, c_2_z), boxHeight, boxHeight, boxHeight);
	createPorp(t, PxVec3(c_2_x, c_2_y + 2.5, c_2_z), boxHeight, boxHeight, boxHeight);

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
	
	PxTransform pose1(PxQuat(PxHalfPi/3+ PxHalfPi/3, PxVec3(0, 1, 0)));
	PxTransform pose2(PxQuat(PxPi + PxPi / 3 + PxPi / 3, PxVec3(0, 1, 0)));
	PxTransform pose(PxQuat(0, PxVec3(0, 1, 0)));
	PxRigidDynamic* actor = createDynamicBox(false, t, PxVec3(-50, 12 + 2 * boxHeight, 50), 5, 10, 1, pose);
	PxRigidDynamic* actor1 = createDynamicBox(false, t, PxVec3(-50, 12 + 2 * boxHeight, 70), 5, 10, 1, pose1);
	PxRigidDynamic* actor2 = createDynamicBox(false, t, PxVec3(-50, 12 + 2 * boxHeight, 60), 5, 10, 1, pose2);

	//跷板 PxVec3 v的第二个参数应该为 底下所有盒子的高+ y
	createSeesaw(t, PxVec3(30, 15 + 2 * boxHeight, 50), 5, 15, 1, defaultPose);
	
	//第二个参数应该为PxVec3 v(v_x, 底下所有盒子的高+10*scale+0.5，v_z)
	createDoor(t, PxVec3(-20, 8.5 + 2 * boxHeight, 10), 0.8, defaultPose);
	createDoor(t, PxVec3(-50, 15.5 + 2 * boxHeight, 20), 1.5, defaultPose);

	createPlane(PxVec3(0, 0, 0), PxVec3(0, 1, 0));
	//createPlane(PxVec3(0, 100, 0), PxVec3(0, 1, 0));
}