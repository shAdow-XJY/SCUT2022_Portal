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
//设定的最大跳跃高度，可调整
float maxJumpHeight = 3.0;
//场景道路盒子的半高
float boxHeight = 0.4 * maxJumpHeight;
//x_distance x轴方向上可跳跃的间隔长度，可根据跳跃跨度调整
float dx = 6.0;
//z_distance z轴方向上可跳跃的间隔长度，可根据跳跃跨度调整
float dz = 6.0;


//迷宫正门可开属性
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
//迷宫侧门可开属性
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

//可选材质：["door"]\["wall"]\[]
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

/*创建盒子形状的静态刚体
t为该刚体构建的相对原点
v为该刚体中心点相对其原点的位置
x, y, z为该盒子的长高宽
pose 刚体的初始朝向*/
PxRigidStatic* createStaticBox(const PxTransform& t, const PxVec3& v, PxReal x, PxReal y, PxReal z, PxTransform& pose, OrganType type = OrganType::block) {
	PxTransform local(v);
	PxShape* shape = gPhysics->createShape(PxBoxGeometry(x, y, z), *gMaterial);
	//碰撞检测的过滤组
	shape->setQueryFilterData(collisionGroup);
	shape->setLocalPose(pose);
	PxRigidStatic* sceneBox = gPhysics->createRigidStatic(t.transform(local));
	sceneBox->attachShape(*shape);
	sceneBox->setName(typeMapName(type));
	gScene->addActor(*sceneBox);
	return sceneBox;
}

/*创建盒子形状的动态刚体
kinematic 动态刚体是否为特殊的kinematic刚体
t为该刚体构建的相对原点
v为该刚体中心点相对其原点的位置
x, y, z为该盒子的长高宽
pose 刚体的初始朝向
velocity 刚体的初始速度，默认为0*/
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


//创建道具类，区别只在Block(OrganType::prop)，试验用，可修改
void createPorp(const PxTransform& t, const PxVec3& v, PxReal x, PxReal y, PxReal z) {
	PxTransform local(v);
	//cout << v.x << " " << v.y << " " << v.z << endl;
	PxShape* shape = gPhysics->createShape(PxBoxGeometry(x, y, z), *gMaterial);
	//碰撞检测的过滤组
	shape->setQueryFilterData(collisionGroup);

	//setupFiltering(shape, FilterGroup::ePIG, FilterGroup::eBIRD);
	PxRigidDynamic* sceneBox = gPhysics->createRigidDynamic(t.transform(local));
	PxRigidBodyExt::updateMassAndInertia(*sceneBox, 1.0f);
	Block* block = new Block("道具", sceneBox->getGlobalPose().p, x, y, z, OrganType::prop);
	sceneBox->attachShape(*shape);
	sceneBox->userData = block;
	sceneBox->setName("Prop");
	gScene->addActor(*sceneBox);
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
	plane->setName("Over"); //over表示角色接触就死亡
	gScene->addActor(*plane);

	
}

/*创建道路
t为该道路构建的相对原点
v为该道路中心点相对其构建原点的位置
x, y, z为该道路的长高宽
pose 道路的初始朝向*/
PxRigidStatic* createRoad(const PxTransform& t, const PxVec3& v, PxReal x, PxReal y, PxReal z, PxTransform& pose) {
	PxRigidStatic* roadActor = createStaticBox(t, v, x, y, z, pose, OrganType::road);
	//std::cout << "v::" <<  v.x << " " << v.y << " " << v.z << endl;
	PxVec3 position = roadActor->getGlobalPose().p;
	Road* road = new Road("路面",position, x, y, z,roadActor);
	roadActor->userData = road;
	roadActor->setName("Road");
	return roadActor;
}

/**
* @brief 创建冰路
**/
PxRigidStatic* createIceRoad(const PxTransform& t, const PxVec3& v, PxReal x, PxReal y, PxReal z, PxTransform& pose) {
	PxRigidStatic* roadActor = createStaticBox(t, v, x, y, z, pose, OrganType::iceroad);
	//std::cout << "v::" <<  v.x << " " << v.y << " " << v.z << endl;
	PxVec3 position = roadActor->getGlobalPose().p;
	Road* road = new Road("冰路面", position, x, y, z, roadActor,OrganType::iceroad);
	roadActor->userData = road;
	return roadActor;
}

//计算更高一层组件的y坐标的函数
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
//	Door* door = new Door("侧门", actor0->getGlobalPose().p, x, y, z, actor0, canOpen, revolute);
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
//	//实际-5  旋转角度小：-17.8  -18.56 -18.561815
//	PxRigidStatic* actor1 = createStaticBox(pos, PxVec3(0, 0, 0), 6 * scale, 10 * scale, 1 * scale, pose);
//	PxRigidDynamic* actor0 = createDynamicBox(false, pos, PxVec3(6.5 * scale, -5 * scale, 0), x, y, z, pose);
//	createStaticBox(pos, PxVec3(23 * scale, 0, 0), 6 * scale, 10 * scale, 1 * scale, pose);
//	PxTransform localFrame0(PxVec3(0, 5 * scale, 0));
//	PxTransform localFrame1(PxVec3(6.5 * scale, 0, 0));
//	PxRevoluteJoint* revolute = PxRevoluteJointCreate(*gPhysics, actor0, localFrame0, actor1, localFrame1);
//	Door* door = new Door("正门", actor0->getGlobalPose().p, x, y, z, actor0, canOpen, revolute);
//	actor0->setName("Door");
//	actor0->userData = door;
//	PxJointAngularLimitPair limits(-PxPi / 130, 0, 0.01f);
//	revolute->setLimit(limits);
//	revolute->setRevoluteJointFlag(PxRevoluteJointFlag::eLIMIT_ENABLED, true);
//	revolute->setLocalPose(PxJointActorIndex::Enum::eACTOR1, PxTransform(PxVec3(6.5 * scale, 0, 0), PxQuat(PxHalfPi, PxVec3(0, 0, 1))));
//	return revolute;
//}

/*正门
t为场景构建的原点
v为该正门与joint连接的门框的中心点相对场景原点的位置 v(v_x, 底下所有盒子的高+10*scale，v_z)
scale 门的缩放系数
pose 刚体朝向
canOpen 门的可开属性*/
void createFrontDoor(const PxTransform& t, PxVec3 v, float scale, PxTransform& pose,bool canOpen = true) {
	PxTransform pos(t.transform(PxTransform(v)));//-17.8  -18.56 -18.561815

	PxReal x = 10 * scale;
	PxReal y = 5 * scale;
	PxReal z = 1 * scale;
	//实际-5  旋转角度小：-17.8  -18.56 -18.561815
	PxRigidStatic* actor1 = createStaticBox(pos, PxVec3(0, 0, 0), 6 * scale, 10 * scale, 1 * scale, pose, OrganType::wall);
	createStaticBox(pos, PxVec3(23 * scale, 0, 0), 6 * scale, 10 * scale, 1 * scale, pose, OrganType::wall);
	if (canOpen) {
		PxRigidDynamic* actor0 = createDynamicBox(false, pos, PxVec3(6.5 * scale, -5 * scale, 0), x, y, z, pose, OrganType::door);
		PxTransform localFrame0(PxVec3(0, 5 * scale, 0));
		PxTransform localFrame1(PxVec3(6.5 * scale, 0, 0));
		PxRevoluteJoint* revolute = PxRevoluteJointCreate(*gPhysics, actor0, localFrame0, actor1, localFrame1);
		Door* door = new Door("正门", actor0->getGlobalPose().p, x, y, z, actor0, canOpen, revolute);
		actor0->userData = door;
		PxJointAngularLimitPair limits(-PxPi / 130, 0, 0.01f);
		revolute->setLimit(limits);
		revolute->setRevoluteJointFlag(PxRevoluteJointFlag::eLIMIT_ENABLED, true);
		revolute->setLocalPose(PxJointActorIndex::Enum::eACTOR1, PxTransform(PxVec3(6.5 * scale, 0, 0), PxQuat(PxHalfPi, PxVec3(0, 0, 1))));
		//return revolute;
	}
	else {
		PxRigidStatic* actor0 = createStaticBox(pos, PxVec3(11.5 * scale, 0, 0), y, x, z, pose, OrganType::door);
		Door* door = new Door("假门", actor0->getGlobalPose().p, x, y, z, false);
		actor0->userData = door;
	}
}

/*侧门
t为场景构建的原点
v为该侧门与joint连接的门框的中心点相对场景原点的位置
scale 门的缩放系数
pose 刚体朝向
canOpen 门的可开属性*/
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
		Door* door = new Door("侧门", actor0->getGlobalPose().p, x, y, z, actor0, canOpen, revolute);
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
		Door* door = new Door("假门", actor0->getGlobalPose().p, x, y, z, false);
		actor0->userData = door;
	}
}

/*创建迷宫关卡
t为场景构建的原点
v为该迷宫中心相对场景原点的位置
scale 门的缩放系数
pose 刚体的朝向*/
void createMaze(const PxTransform& t, PxVec3 v, float scale, PxTransform& pose) {
	//迷宫左下角坐标
	float x = v.x + 4 * 37 * scale;
	float z = v.z - 4 * 37 * scale;

	//迷宫门的坐标
	float door_x;
	float door_y = v.y + boxHeight + 0.5 + 10 * scale;
	float door_z;

	//正门逻辑
	//从左到右
	for (int i = 0; i < 8; i++) {
		door_x = x - i * 37 * scale - 30 * scale;
		//从下到上
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

	//侧门逻辑
	//从下到上
	for (int i = 0; i < 8; i++) {
		door_z = z + i * 37 * scale + 7 * scale;
		//从左到右
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

/*跷板
t为场景构建的原点
v为该跷板中心相对场景原点的位置 v的第二个参数应该为 底下所有盒子的高 + y
x、y、z为长板的长高宽
pose 刚体的初始朝向*/
PxRevoluteJoint* createSeesaw(const PxTransform& t,PxVec3 v,float x, float y, float z, PxTransform& pose) {
	PxTransform pos(t.transform(PxTransform(v)));
	PxRigidDynamic* actor0 = createDynamicBox(false, pos, PxVec3(0, 0, 0), x, y, z, pose, OrganType::seesaw);
	PxVec3 position = PxVec3(actor0->getGlobalPose().p.x, actor0->getGlobalPose().p.y, actor0->getGlobalPose().p.z);
	Seesaw* seesaw = new Seesaw("翘板",position,x,y,z,actor0);
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
	Seesaw* seesaw = new Seesaw("跷板", position, x, y, z, actor1);
	actor1->userData = seesaw;
	seesaw->attachRevolute(revolute);
	revolute->setLimit(PxJointAngularLimitPair(-PxPi / 130, 0, 0.01f));
	revolute->setRevoluteJointFlag(PxRevoluteJointFlag::eLIMIT_ENABLED, true);
}

/*创建跷跷板关卡
v为跷板0的中心点
sx、sy、sz为跷板的半长
pose 刚体可用的朝向*/
float createSeesawLevel(const PxTransform& t, PxVec3 v,float sx, float sy, float sz, PxTransform& pose) {
	//第一块跷板0
	float x0 = v.x;
	float y0 = v.y;
	float z0 = v.z;
	PxTransform pose0(PxQuat(PxHalfPi / 6, PxVec3(1, 0, 0)));
	//createSeesaw(t, v, sx, sy, sz, pose0);
	//跷板水平位置
	createSeesaw(t, v, sx, sy, sz, pose);

	//第二块跷板1
	float x1 = x0 + 2*sx + dx;
	float z1 = z0 + sz + dz / 2;
	PxTransform pose1(PxQuat(PxHalfPi / 6, PxVec3(1, 0, 0)));
	//createSeesaw(t, PxVec3(x1, y0, z1), sx, sy, sz, pose1);
	//跷板水平位置
	createSeesaw(t, PxVec3(x1, y0, z1), sx, sy, sz, pose);
	
	float x2 = x1;
	float z2 = z0 - sz - dz / 2;
	PxTransform pose2(PxQuat(-PxHalfPi / 6, PxVec3(1, 0, 0)));
	//createSeesaw(t, PxVec3(x2, y0, z2), sx, sy, sz, pose2);
	//跷板水平位置
	createSeesaw(t, PxVec3(x2, y0, z2), sx, sy, sz, pose);

	float x3 = x0 + 4 * sx + 2 * dx;
	float z3 = z0;
	createSeesaw(t, PxVec3(x3, y0, z3), sx, sy, sz, pose);

	float x4 = x3;
	float z4 = z0 + 2 * sz + dz;
	PxTransform pose4(PxQuat(PxHalfPi / 6, PxVec3(1, 0, 0)));
	//createSeesaw(t, PxVec3(x4, y0, z4), sx, sy, sz, pose4);
	//跷板水平位置
	createSeesaw(t, PxVec3(x4, y0, z4), sx, sy, sz, pose);

	float x5 = x3;
	float z5 = z0 - 2 * sz - dz;
	PxTransform pose5(PxQuat(PxHalfPi / 6, PxVec3(1, 0, 0)));
	//createSeesaw(t, PxVec3(x5, y0, z5), sx, sy, sz, pose5);
	//跷板水平位置
	createSeesaw(t, PxVec3(x5, y0, z5), sx, sy, sz, pose);

	float x6 = x1 + 4 * sx + 2 * dx;
	float z6 = z1;
	PxTransform pose6(PxQuat(PxHalfPi / 6, PxVec3(1, 0, 0)));
	//createSeesaw(t, PxVec3(x6, y0, z6), sx, sy, sz, pose6);
	//跷板水平位置
	createSeesaw(t, PxVec3(x6, y0, z6), sx, sy, sz, pose);

	float x7 = x6;
	float z7 = z2;
	PxTransform pose7(PxQuat(-PxHalfPi / 6, PxVec3(1, 0, 0)));
	//createSeesaw(t, PxVec3(x7, y0, z7), sx, sy, sz, pose7);
	//跷板水平位置
	createSeesaw(t, PxVec3(x7, y0, z7), sx, sy, sz, pose);
	
	float x8 = x3 + 4 * sx + 2 * dx;
	float z8 = z0;
	PxTransform pose8(PxQuat(-PxHalfPi / 6, PxVec3(1, 0, 0)));
	//createSeesaw(t, PxVec3(x8, y0, z8), sx, sy, sz, pose8);
	//跷板水平位置
	createSeesaw(t, PxVec3(x8, y0, z8), sx, sy, sz, pose);

	return x8;
 }

/*创建球体形状的动态刚体
t为该刚体构建的相对原点
v为该刚体中心点相对其构建原点的位置
halfExtend球体半径
velocity 刚体的初始速度，默认为0*/
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

/*创建球体形状的静态刚体
t为该刚体构建的相对原点
v为该刚体中心点相对其构建原点的位置
halfExtend球体半径*/
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

/*摆锤
halfExtend 摆锤底部球体的半径
rod_x,rod_y,rod_z 连接杆的长高宽
pose 刚体（连接杆）的初始朝向
velocity 摆锤的初始速度*/
void createPendulum(const PxTransform& t, PxVec3 v, float halfExtend, float rod_x, float rod_y, float rod_z, PxTransform& pose,PxVec3 velocity) {
	PxTransform pos(t.transform(PxTransform(v)));
	PxRigidDynamic* actor0 = createDynamicSphere(pos, PxVec3(0, 0, 0), halfExtend);
	actor0->setMass(30.0f);
	actor0->setAngularDamping(0.f);
	//actor0->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	Pendulum* pendulum = new Pendulum("摆锤", actor0->getGlobalPose().p, halfExtend, actor0);
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

//创建平移路面 属于动态刚体(区别于road静态刚体)
void createPrismaticRoad(const PxTransform& t, PxVec3 v0, PxReal x0, PxReal y0, PxReal z0, PxTransform& pose0, PxVec3 v1, PxReal x1, PxReal y1, PxReal z1, PxTransform& pose1, PxJointLinearLimitPair& limits, const PxVec3& velocity = PxVec3(0)) {
	PxTransform pos(t.transform(PxTransform(v0)));
	PxRigidStatic* actor0 = createRoad(pos, PxVec3(0, 0, 0), x0, y0, z0, pose0);
	PxRigidDynamic* actor1 = createDynamicBox(false, pos, v1, x1, y1, z1, pose1, OrganType::prismaticRoad, velocity);
	PxVec3 position = actor1->getGlobalPose().p;
	PrismaticRoad* prismaticRoad = new PrismaticRoad("平移路面", position, x1, y1, z1, actor1);
	actor1->userData = prismaticRoad;
	actor1->setName("PrismaticRoad");
	PxTransform localFrame0(PxVec3(0, 0, 0));
	PxTransform localFrame1(PxVec3(-v1.x, 0, -v1.z));
	PxPrismaticJoint* prismatic = PxPrismaticJointCreate(*gPhysics, actor0, localFrame0, actor1, localFrame1);
	//PxSpring spring(40.0f, 0.f);  //stiffness、dampling
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
	RotateRod* rotateRod0 = new RotateRod("转杆", rod0->getGlobalPose().p, rod0_x, rod0_y, rod0_z, rod0);
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
	RotateRod* rotateRod1 = new RotateRod("转杆", rod1->getGlobalPose().p, rod1_x, rod1_y, rod1_z, rod1);
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
	//上 左 右 下
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

	//竖 左 右
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

/*风扇（旋转杆）
t为该刚体构建的相对原点
v为该刚体中心点相对其构建原点的位置
abgularVelocity 风扇旋转角速度，这里扇面水平，通过角速度PxVec3(0,y,0)中y的大小控制其旋转速度*/
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
	RotateRod* rotateRod = new RotateRod("转杆", fan->getGlobalPose().p, 25.0, 1.0, 1.0, fan);
    fan->setName("RotateRod");
	fan->userData = rotateRod;
	PxRigidStatic* sphere = createStaticSphere(pos, PxVec3(0, 0, 0), 1.8);
	PxTransform localFrame(PxVec3(0, 0, 0));
	PxFixedJoint* fixed = PxFixedJointCreate(*gPhysics, fan, localFrame, sphere, localFrame);
}


void createRoTateRodLevel(const PxTransform& t, PxVec3 v,float rod_length,float boxHeight,PxTransform& pose) {
	//createRoad(t, v, rod_length, boxHeight, rod_length, pose);
	//createRotateRod(t, PxVec3(v.x, v.y + boxHeight + 2.0, v.z), 1.5, pose, PxVec3(1, 0, 0) * 70, PxVec3(0, 0, -1) * 70);
	createFan(t, PxVec3(v.x, v.y + 2 * boxHeight, v.z), PxVec3(0, 4, 0));

	//旋转杆1中心点
	float rr1_x = v.x - 2 * rod_length;
	float rr1_y = v.y;
	float rr1_z = v.z + rod_length + 0.5;
	//中间路段
	createRoad(t, PxVec3(rr1_x, rr1_y, v.z), rod_length, boxHeight, 2 * rod_length + 1.0, pose);
	//createRotateRod(t, PxVec3(rr1_x, rr1_y + 2 * boxHeight, rr1_z), 1.5, pose, PxVec3(1, 0, 0) * 70, PxVec3(0, 0, -1) * 70);
	createFan(t, PxVec3(rr1_x, rr1_y + boxHeight + 2.0, rr1_z), PxVec3(0, 4, 0));

	//旋转杆2中心点
	float rr2_x = rr1_x;
	float rr2_y = rr1_y;
	float rr2_z = v.z - rod_length - 0.5;
	//createRotateRod(t, PxVec3(rr2_x, rr2_y + 2 * boxHeight, rr2_z), 1.5, pose, PxVec3(-1, 0, 0) * 70, PxVec3(0, 0, 1) * 70);
	createFan(t, PxVec3(rr2_x, rr2_y + boxHeight + 2.0, rr2_z), PxVec3(0, -4, 0));

	//旋转杆3中心点
	float rr3_x = v.x - 4 * rod_length;
	float rr3_y = v.y;
	float rr3_z = v.z;
	createRoad(t, PxVec3(rr3_x, rr3_y, rr3_z), rod_length, boxHeight, rod_length, pose);
	//createRotateRod(t, PxVec3(rr3_x, rr3_y + 2 * boxHeight, rr3_z), 1.5, pose, PxVec3(-1, 0, 0) * 70, PxVec3(0, 0, 1) * 70);
	createFan(t, PxVec3(rr3_x, rr3_y + boxHeight + 2.0, rr3_z), PxVec3(0, -4, 0));
}

//创建水池
void createPool(const PxTransform& t, PxVec3 bottom, float poolLength, float poolHeight,float poolWidth, PxTransform& pose) {
	PxTransform pos(t.transform(PxTransform(bottom)));
	//底部
	createStaticBox(pos, PxVec3(0, 0, 0), poolLength + 2.0, 1.0, poolWidth + 2.0, pose);
	//左侧
	createStaticBox(pos, PxVec3(1.0 + poolLength, 1.0 + poolHeight, 0), 1.0, poolHeight, poolWidth + 2.0, pose);
	//右侧
	createStaticBox(pos, PxVec3(- 1.0 - poolLength,  1.0 + poolHeight, 0), 1.0, poolHeight, poolWidth + 2.0, pose);
	//前侧
	createStaticBox(pos, PxVec3(0,  1.0 + poolHeight, - 1.0 - poolWidth), poolLength, poolHeight, 1.0, pose);
	//后侧
	createStaticBox(pos, PxVec3(0,  1.0 + poolHeight, 1.0 + poolWidth), poolLength, poolHeight, 1.0, pose);
}

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
	/*RotateRod* rotateRod = new RotateRod("转杆", fan->getGlobalPose().p, 25.0, 1.0, 1.0, fan);
	fan->setName("RotateRod");
	fan->userData = rotateRod;*/
}

void createFerrisWheel(const PxTransform& t, PxVec3 v, float x, float y, float z, PxVec3 angularVelocity) {
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
	float seatLength = 2.5;
	float seatWidth = 1.5;
	PxTransform pose(PxVec3(0));
	//seat0左
	PxRigidDynamic* seat0 = createDynamicBox(false, pos, PxVec3(0.9 * x, -2.0, -z - seatWidth - 0.1), seatLength, 0.4, seatWidth, pose, OrganType::prismaticRoad);
	seat0->setMass(1.0);
	seat0->setMassSpaceInertiaTensor(PxVec3(0.f));
	PxTransform fw0(PxVec3(0.9 * x, 0, -z - seatWidth - 0.1));
	PxTransform s0(PxVec3(0, 2.0, 0));
	PxDistanceJoint* d0 = PxDistanceJointCreate(*gPhysics, ferrisWheel, fw0, seat0, s0);
	/*d0->setMaxDistance(2.0f);
	d0->setDistanceJointFlag(PxDistanceJointFlag::eMAX_DISTANCE_ENABLED, true);*/
	//seat1右
	PxRigidDynamic* seat1 = createDynamicBox(false, pos, PxVec3(-0.9 * x, -2.0, -z - seatWidth - 0.1), seatLength, 0.4, seatWidth, pose, OrganType::prismaticRoad);
	seat1->setMass(1.0);
	seat1->setMassSpaceInertiaTensor(PxVec3(0.f));
	PxTransform fw1(PxVec3(-0.9 * x, 0, -z - seatWidth - 0.1));
	PxTransform s1(PxVec3(0, 2.0, 0));
	PxDistanceJoint* d1 = PxDistanceJointCreate(*gPhysics, ferrisWheel, fw1, seat1, s1);
	/*d1->setMaxDistance(2.0f);
	d1->setDistanceJointFlag(PxDistanceJointFlag::eMAX_DISTANCE_ENABLED, true);*/
	//seat2左上
	PxRigidDynamic* seat2 = createDynamicBox(false, pos, PxVec3(0.9 * x * cos(PxPi / 3), 0.9 * x * sin(PxPi / 3) - 2.0, -z - seatWidth - 0.1),seatLength, 0.4, seatWidth, pose, OrganType::prismaticRoad);
	seat2->setMass(1.0);
	seat2->setMassSpaceInertiaTensor(PxVec3(0.f));
	PxTransform fw2(PxVec3(0.9 * x * cos(PxPi / 3), 0.9 * x * sin(PxPi / 3), -z - seatWidth - 0.1));
	PxTransform s2(PxVec3(0, 2.0, 0));
	PxDistanceJoint* d2 = PxDistanceJointCreate(*gPhysics, ferrisWheel, fw2, seat2, s2);
	/*d2->setMaxDistance(2.0f);
	d2->setDistanceJointFlag(PxDistanceJointFlag::eMAX_DISTANCE_ENABLED, true);*/
	//seat3右上
	PxRigidDynamic* seat3 = createDynamicBox(false, pos, PxVec3(-0.9 * x * cos(PxPi / 3), 0.9 * x * sin(PxPi / 3) - 2.0, -z - seatWidth - 0.1), seatLength, 0.4, seatWidth, pose, OrganType::prismaticRoad);
	seat3->setMass(1.0);
	seat3->setMassSpaceInertiaTensor(PxVec3(0.f));
	PxTransform fw3(PxVec3(-0.9 * x * cos(PxPi / 3), 0.9 * x * sin(PxPi / 3), -z - seatWidth - 0.1));
	PxTransform s3(PxVec3(0, 2.0, 0));
	PxDistanceJoint* d3 = PxDistanceJointCreate(*gPhysics, ferrisWheel, fw3, seat3, s3);
	/*d3->setMaxDistance(2.0f);
	d3->setDistanceJointFlag(PxDistanceJointFlag::eMAX_DISTANCE_ENABLED, true);*/
	//seat4左下
	PxRigidDynamic* seat4 = createDynamicBox(false, pos, PxVec3(0.9 * x * cos(PxPi / 3), -0.9 * x * sin(PxPi / 3) - 2.0, -z - seatWidth - 0.1), seatLength, 0.4, seatWidth, pose, OrganType::prismaticRoad);
	seat4->setMass(1.0);
	seat4->setMassSpaceInertiaTensor(PxVec3(0.f));
	PxTransform fw4(PxVec3(0.9 * x * cos(PxPi / 3), -0.9 * x * sin(PxPi / 3), -z - seatWidth - 0.1));
	PxTransform s4(PxVec3(0, 2.0, 0));
	PxDistanceJoint* d4 = PxDistanceJointCreate(*gPhysics, ferrisWheel, fw4, seat4, s4);
	/*d4->setMaxDistance(2.0f);
	d4->setDistanceJointFlag(PxDistanceJointFlag::eMAX_DISTANCE_ENABLED, true);*/
	//seat5右下
	PxRigidDynamic* seat5 = createDynamicBox(false, pos, PxVec3(-0.9 * x * cos(PxPi / 3), -0.9 * x * sin(PxPi / 3) - 2.0, -z - seatWidth - 0.1), seatLength, 0.4, seatWidth, pose, OrganType::prismaticRoad);
	seat5->setMass(1.0);
	seat5->setMassSpaceInertiaTensor(PxVec3(0.f));
	PxTransform fw5(PxVec3(-0.9 * x * cos(PxPi / 3), -0.9 * x * sin(PxPi / 3), -z - seatWidth - 0.1));
	PxTransform s5(PxVec3(0, 2.0, 0));
	PxDistanceJoint* d5 = PxDistanceJointCreate(*gPhysics, ferrisWheel, fw5, seat5, s5);
	/*d5->setMaxDistance(2.0f);
	d5->setDistanceJointFlag(PxDistanceJointFlag::eMAX_DISTANCE_ENABLED, true);*/
	//中心球体
	PxRigidDynamic* sphere = createDynamicSphere(pos, PxVec3(0, 0, 0), 2 * y);
	PxTransform center(PxVec3(0));
	PxFixedJointCreate(*gPhysics, ferrisWheel, center, sphere, center);
}

//创建游戏场景
void createGameScene(const PxTransform& t) {
	time_t startTime = time(NULL);
	PxTransform defaultPose(PxQuat(0, PxVec3(0, 1, 0)));  //刚体默认pose

	float r_1_l = 4.0;  //road_1_length 4 100
	float r_1_w = 15.0;  //road_1_width 8 100
	float c_1_y = boxHeight;  //the position of the center of road_1
	// create road_1
	//createRoad(t, PxVec3(0, c_1_y, 0), r_1_l, boxHeight, r_1_w, defaultPose);
	createRoad(t, PxVec3(0, c_1_y, 0), 100, boxHeight, 100, defaultPose);

	float r_2_w = 4.0;
	float r_2_l = 20.0;
	float c_2_x = r_2_l - r_1_l;
	float c_2_y = center_y(c_1_y);
	float c_2_z = r_1_w + r_2_w;
	createRoad(t, PxVec3(c_2_x, c_2_y, c_2_z), r_2_l , boxHeight, r_2_w, defaultPose);

	//创建道具类场景
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
	
	//最后一节台阶的中心点（center_x-2*stairsLength,centerHeight-2*boxHeight,center_z）
	std::cout << "摆锤前位置相对场景原点的坐标为:" << center_x - 2 * stairsLength << "," << centerHeight - 2 * boxHeight << "," << center_z << endl;
	
	//悬空路段
	float roadblock_length = 3.0;
	float roadblock_width = 4.0;
	float rb_x = center_x - 2 * stairsLength + dx + roadblock_length;
	float rb_y = centerHeight - 2 * boxHeight;
	float rb_z = center_z;
	for (int i = 0; i < 3; i++) {
		createRoad(t, PxVec3(rb_x, rb_y, rb_z), roadblock_length, boxHeight, roadblock_width, defaultPose);
		if (i % 2 == 0) {
			//摆锤坐标y为 地下所有盒子高+halfExtend+0.5（间隔）
			createPendulum(t, PxVec3(rb_x, rb_y + boxHeight + 5.5, rb_z), 5.0, 0.6, 10.0, 0.6, defaultPose, PxVec3(0, 0, -1));
		}
		else {
			createPendulum(t, PxVec3(rb_x, rb_y + boxHeight + 5.5, rb_z), 5.0, 0.6, 10.0, 0.6, defaultPose, PxVec3(0, 0, 1));
		}
		rb_x = rb_x + 2 * roadblock_length + dx;
	}

	//最后一块悬空路段的中心点（rb_x-2 * roadblock_length - dx, rb_y, rb_z）
	std::cout << "最后一块悬空路段的中心点相对场景原点的坐标为:" << rb_x - 2 * roadblock_length - dx << "," << rb_y << "," << rb_z << endl;
	
	//第一块跷板的坐标
	float seesawpos_x = rb_x - roadblock_length + 0.5 * dx;
	float seesawpos_y = rb_y;
	float seesawpos_z = rb_z;
	float sx = 5.0;
	float sy = 1.0;
	float sz = 15.0;
	PxVec3 seesawpos_v(seesawpos_x, seesawpos_y, seesawpos_z);
	float lastSeesaw_x = createSeesawLevel(t, seesawpos_v, sx, sy, sz, defaultPose);
	//最后一块跷板的中心点（lastSeesaw_x,seesawpos_y, seesawpos_z）

	//连接跷板关卡与迷宫关卡的路段
	float r_3_l = 5.0;
	float r_3_w = 50.0;
	float c_3_x = lastSeesaw_x + sx + dx + r_3_l;
	float c_3_y = seesawpos_y;
	float c_3_z = seesawpos_z + r_3_w - 5.0;
	PxRigidStatic* r_3 = createRoad(t, PxVec3(c_3_x, c_3_y, c_3_z), r_3_l, boxHeight, r_3_w, defaultPose);
	std::cout <<"连接路段相对场景原点的坐标:"<< c_3_x << "," << c_3_y << "," << c_3_z << endl;

	//迷宫边长缩放系数
	float scale = 0.8;
	//迷宫小正方形边长37*scale
	float sideLength = 37.0 * scale;
	//迷宫中心点坐标
	float mazePos_x = c_3_x - 1.5 * sideLength;
	float mazePos_y = c_3_y;
	float mazePos_z = c_3_z + r_3_w + 4 * sideLength;
	PxVec3 maze_v(mazePos_x, mazePos_y, mazePos_z);
	//迷宫地板边长
	float mazeLength = 4 * sideLength+1;
	//创建迷宫地板
	createRoad(t, maze_v, mazeLength, boxHeight, mazeLength, defaultPose);
	createMaze(t, maze_v, scale, defaultPose);
	//迷宫出口坐标
	float mazeOut_x = mazePos_x - 3.5 * sideLength;
	float mazeOut_y = mazePos_y;
	float mazeOut_z = mazePos_z + 4 * sideLength;

	//迷宫出口路段
	float r_4_l = 5.0;
	float r_4_w = 35.0;
	float c_4_x = mazeOut_x;
	float c_4_y = mazeOut_y;
	float c_4_z = mazeOut_z + r_4_w;
	createRoad(t, PxVec3(c_4_x, c_4_y, c_4_z), r_4_l, boxHeight, r_4_w, defaultPose);
	std::cout << "迷宫出口路段相对场景原点的坐标为:" <<c_4_x << "," << c_4_y << "," << c_4_z << endl;

	//楼梯1
	//stairsWidth ： 4.0
	//stairsLength ： 2.0
	float center_x1 = c_4_x - r_4_l - stairsLength;
	float centerHeight1 = center_y(c_4_y);
	float center_z1 = c_4_z + r_4_w - stairsWidth;
	for (int i = 0; i <= 5; i++) {
		createRoad(t, PxVec3(center_x1, centerHeight1, center_z1), stairsLength, boxHeight, stairsWidth, defaultPose);
		centerHeight1 = center_y(centerHeight1);
		center_x1 -= 2 * stairsLength;
	}
    //最后一节台阶的中心点（center_x1+2*stairsLength,centerHeight1-2*boxHeight,center_z1）
	
	//平移路段0 prismaticRoad
	//roadblock_length : 3.0
	//roadblock_width : 4.0
	float c_5_x = center_x1 + stairsLength - roadblock_length;
	float c_5_y = centerHeight1;
	float c_5_z = center_z1;
	PxVec3 pr_v0(-2.5 * roadblock_length, 0, 0);
	PxJointLinearLimitPair limits0(-30.0, -2.5 * roadblock_length, PxSpring(20.0, 0));
	PxVec3 velocity0((-1, 0, 0) * 30);
	createPrismaticRoad(t, PxVec3(c_5_x, c_5_y, c_5_z), roadblock_length, boxHeight, roadblock_width, defaultPose, pr_v0, roadblock_length, boxHeight, roadblock_width, defaultPose, limits0, velocity0);
	std::cout << "平移路段前相对场景原点的坐标为:" << c_5_x << "," << c_5_y << "," << c_5_z << endl;

	//旋转杆0中心点
	float rod_length = 25.0;
	float c_6_x = c_5_x - 45.0;
	float c_6_y = c_5_y;
	float c_6_z = c_5_z - roadblock_width - dz - rod_length;
	createRoTateRodLevel(t, PxVec3(c_6_x, c_6_y, c_6_z), rod_length, boxHeight, defaultPose);
	std::cout << "旋转杆关卡角落位置相对于场景原点的坐标为" << c_6_x + rod_length << "," << c_6_y << "," << c_6_z + rod_length << endl;

	//风扇关卡与水池连接齿轮
	float gearLength = 5.0;
	float gearHeight = 0.4;
	float gearWidth = 10.0;
	float gear0_x = c_6_x - 5 * rod_length - dx * 0.8 - gearLength;
	float gear0_y = c_6_y;
	float gear0_z = c_6_z;
	createGear(t, PxVec3(gear0_x, gear0_y, gear0_z), gearLength, gearHeight, gearWidth, PxVec3(0, 0, 4));
	float gear1_x = gear0_x - 2 * gearLength - dx * 0.8;
	createGear(t, PxVec3(gear1_x, gear0_y, gear0_z), gearLength, gearHeight, gearWidth, PxVec3(0, 0, 4));

	//水池
	float poolLength = 50.0;
	float poolHeight = 10.0;
	float poolWidth = 25.0;
	float bottom_x = gear1_x - gearLength - dx * 0.8 - poolLength - 2.0;
	float bottom_y = gear0_y + boxHeight - 2 * poolHeight - 1.0;
	float bottom_z = gear0_z;
	createPool(t, PxVec3(bottom_x, bottom_y, bottom_z), poolLength, poolHeight, poolWidth, defaultPose);
	//水池底部的相对于场景原点t的位置 PxVec3 localPose(bottom_x,bottom_y,bottom_z)
	//全局位置 t.transform(PxTransform(localPose)).p

	//createRoad(t, PxVec3(-4, 20, -6), 4.0, 1.0, 2.0, defaultPose);
	createFerrisWheel(t, PxVec3(center_x-12.0, centerHeight, center_z+10.0), 15.0, 0.4, 0.4, PxVec3(0, 0,0.5));
	//createSideSeesaw(t, PxVec3(-2, 20, 0), 5.0, 1.0, 15.0, defaultPose);
	createPlane(PxVec3(0, 0, 0), PxVec3(0, 1, 0));

}