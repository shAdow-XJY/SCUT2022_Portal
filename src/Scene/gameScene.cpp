#include <ctype.h>
#include "PxPhysicsAPI.h"
#include "../Sphere/Pendulum.h"
#include "../Block/Door.h"
#include "../Block/Road.h"
#include "../Block/PipeWall.h"
#include "../Block/Seesaw.h"
#include "../Block/RotateRod.h"
#include "../Block/PrismaticRoad.h"
#include "../Block/FerrisWheel.h"
#include<vector>
#include<iostream>
#include <glut.h>
#include <../Particles/PxParticleGeometry.h>
#include <Render/BMPLoader.h>
#include <map>
#include <time.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


using namespace physx;

extern PxScene* gScene;
extern PxMaterial* gMaterial;
extern PxMaterial* pMaterial;
extern PxPhysics* gPhysics;
extern vector<PxVec3> checkpoints;
extern int primaryJumpHeight;
extern bool openDynamicBall;

//设定的最大跳跃高度，可调整
float maxJumpHeight = 6.0;
//场景道路盒子的半高
float boxHeight = 0.4 * maxJumpHeight;
//x_distance x轴方向上可跳跃的间隔长度，可根据跳跃跨度调整
float dx = 8.0;
//z_distance z轴方向上可跳跃的间隔长度，可根据跳跃跨度调整
float dz = 8.0;

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
	case OrganType::keyDoor: {
		return "KeyDoor";
	}
	case OrganType::poolWall: {
		return "PoolWall";
	}
	case OrganType::ground: {
		return "Ground";
	}	
	case OrganType::finalLine: {
		return "FinalLine";
	}
	case OrganType::pipeWall: {
		return "PipeWall";
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
void createFrontDoor(const PxTransform& t, PxVec3 v, float scale, PxTransform& pose,bool canOpen = true, bool needKey = false) {
	PxTransform pos(t.transform(PxTransform(v)));//-17.8  -18.56 -18.561815

	PxReal x = 10 * scale;
	PxReal y = 5 * scale;
	PxReal z = 1 * scale;
	//实际-5  旋转角度小：-17.8  -18.56 -18.561815
	PxRigidStatic* actor1 = createStaticBox(pos, PxVec3(0, 0, 0), 6 * scale, 10 * scale, 1 * scale, pose, OrganType::wall);
	createStaticBox(pos, PxVec3(23 * scale, 0, 0), 6 * scale, 10 * scale, 1 * scale, pose, OrganType::wall);
	if (canOpen) {
		PxRigidDynamic* actor0;
		if (needKey) {
			actor0 = createDynamicBox(false, pos, PxVec3(6.5 * scale, -5 * scale, 0), x, y, z, pose, OrganType::keyDoor);
		}
		else
		{
			actor0 = createDynamicBox(false, pos, PxVec3(6.5 * scale, -5 * scale, 0), x, y, z, pose, OrganType::door);
		}
		PxTransform localFrame0(PxVec3(0, 5 * scale, 0));
		PxTransform localFrame1(PxVec3(6.5 * scale, 0, 0));
		PxRevoluteJoint* revolute = PxRevoluteJointCreate(*gPhysics, actor0, localFrame0, actor1, localFrame1);

		Door* door;
		if (needKey) {
			door = new Door("正门", actor0->getGlobalPose().p, x, y, z, actor0, revolute, needKey);
		}
		else
		{
			door = new Door("正门", actor0->getGlobalPose().p, x, y, z, actor0, canOpen, revolute);
		}

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
	//迷宫小房间边长
	float roomLength = 37 * scale;
	
	//迷宫左下角坐标
	float x = v.x + 4 * roomLength;
	float z = v.z - 4 * roomLength;

	//迷宫门的坐标
	float door_x;
	float door_y = v.y + boxHeight + 0.5 + 10 * scale;
	float door_z;

	//钥匙存放处 21 82 13 86 18
	vector<PxVec3> keyPositions = {
		PxVec3(x - 1.5 * roomLength,door_y,z + 0.5 * roomLength),
		PxVec3(x - 7.5 * roomLength,door_y,z + 1.5 * roomLength),
		PxVec3(x - 0.5 * roomLength,door_y,z + 2.5 * roomLength),
		PxVec3(x - 7.5 * roomLength,door_y,z + 5.5 * roomLength),
		PxVec3(x - 0.5 * roomLength,door_y,z + 7.5 * roomLength)
	};

	//随机生成钥匙
	srand(time(NULL));
	int index = rand() % keyPositions.size();
	std::cout << "钥匙" << index << endl;
	createPorp(t, keyPositions[3], 1.0, 1.0, 1.0);

	//正门逻辑
	//从左到右
	for (int i = 0; i < 8; i++) {
		door_x = x - i * 37 * scale - 30 * scale;
		//从下到上
		for (int j = 0; j < 9; j++) {
			door_z = z + j * 37 * scale;
			if (frontDoorCanOpen[i][j]) {
				if ((i == 7 && j == 8)||(i == 2 && j == 0)) {
					createFrontDoor(t, PxVec3(door_x, door_y, door_z), scale, pose, true, true);
					//创建道具类场景
					/*if(i == 2 && j == 0)
					createPorp(t, PxVec3(door_x + 5.0f , door_y , door_z-5.0f), 1.0, 1.0, 1.0);*/
				}
				else
				{
					createFrontDoor(t, PxVec3(door_x, door_y, door_z), scale, pose, true);
				}
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
	Seesaw* seesaw = new Seesaw("跷板", position, x, y, z, actor1,1);
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


/*创建球体形状的粒子
halfExtend球体半径
velocity 刚体的初始速度，默认为0*/
PxRigidDynamic* createParticleSphere(const PxVec3& v, PxReal halfExtend, const PxVec3& velocity = PxVec3(0)) {
	PxTransform local(v);
	PxShape* shape = gPhysics->createShape(PxSphereGeometry(halfExtend), *pMaterial);
	shape->setQueryFilterData(collisionGroup);
	PxRigidDynamic* sceneBox = gPhysics->createRigidDynamic(local);
	sceneBox->attachShape(*shape);
	sceneBox->setAngularDamping(1.0f);
	sceneBox->setLinearVelocity(velocity);
	sceneBox->setName("Particle");
	PxRigidBodyExt::updateMassAndInertia(*sceneBox, 10.0f);
	gScene->addActor(*sceneBox);
	return sceneBox;
}


/*摆锤
halfExtend 摆锤底部球体的半径
rod_x,rod_y,rod_z 连接杆的长高宽
pose 刚体（连接杆）的初始朝向
velocity 摆锤的初始速度*/
void createPendulum(const PxTransform& t, PxVec3 v, float halfExtend, float rod_x, float rod_y, float rod_z, const char* name, PxTransform& pose,PxVec3 velocity) {
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
	actor1->setName(name);
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

void createNewPendulum(const PxTransform& t, PxVec3 v, float halfExtend, float rod_x, float rod_y, float rod_z, const char* name, PxTransform& pose, PxVec3 velocity) {
	PxTransform pos(t.transform(PxTransform(v)));
	PxRigidDynamic* newPendulum = gPhysics->createRigidDynamic(pos);
	PxShape* shape0 = PxRigidActorExt::createExclusiveShape(*newPendulum, PxSphereGeometry(halfExtend), *gMaterial);
	PxShape* shape1 = PxRigidActorExt::createExclusiveShape(*newPendulum, PxBoxGeometry(rod_x, rod_y, rod_z), *gMaterial);
	shape1->setLocalPose(PxTransform(PxVec3(0, halfExtend + rod_y - 2.0, 0)));
	shape0->setQueryFilterData(collisionGroup);
	shape1->setQueryFilterData(collisionGroup);
	newPendulum->setMass(1.0f);
	newPendulum->setAngularDamping(0.f);
	Pendulum* pendulum = new Pendulum(name, newPendulum->getGlobalPose().p, halfExtend, newPendulum);
	newPendulum->setName(name);
	newPendulum->userData = pendulum;
	newPendulum->setLinearVelocity(velocity * 35);
	PxRigidStatic* actor2 = createStaticSphere(pos, PxVec3(0, halfExtend + rod_y * 2 + rod_x - 3, 0), rod_x * 2);
	actor2->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	PxTransform localFrame2(PxVec3(0, halfExtend + rod_y - 2.0 + rod_y + rod_x - 1.0, 0));
	PxTransform localFrame3(PxVec3(0, 0, 0));
	PxSphericalJoint* spherical = PxSphericalJointCreate(*gPhysics, newPendulum, localFrame2, actor2, localFrame3);
	gScene->addActor(*newPendulum);
}

//创建平移路面 属于动态刚体(区别于road静态刚体)
/*/void createPrismaticRoad(const PxTransform& t, PxVec3 v0, PxReal x0, PxReal y0, PxReal z0, PxTransform& pose0, PxVec3 v1, PxReal x1, PxReal y1, PxReal z1, PxTransform& pose1, PxJointLinearLimitPair& limits, const PxVec3& velocity = PxVec3(0)) {
	PxTransform pos(t.transform(PxTransform(v0)));
	PxRigidStatic* actor0 = createRoad(pos, PxVec3(0, 0, 0), x0, y0, z0, pose0);
	PxRigidDynamic* actor1 = createDynamicBox(false, pos, v1, x1, y1, z1, pose1, OrganType::prismaticRoad, velocity);
	//PxRigidDynamic* actor1 = createDynamicBox(false, pos, v1, x1, y1, z1, pose1, OrganType::prismaticRoad);
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
}*/

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
	RotateRod* rotateRod1 = new RotateRod("RotateRod", rod1->getGlobalPose().p, rod1_x, rod1_y, rod1_z, rod1);
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

/*风扇（旋转杆）
t为该刚体构建的相对原点
v为该刚体中心点相对其构建原点的位置
abgularVelocity 风扇旋转角速度，这里扇面水平，通过角速度PxVec3(0,y,0)中y的大小控制其旋转速度*/
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
	RotateRod* rotateRod = new RotateRod("RotateRod", fan->getGlobalPose().p, x,y,z, fan);
    fan->setName("RotateRod");
	fan->userData = rotateRod;
	PxRigidStatic* sphere = createStaticSphere(pos, PxVec3(0, 0, 0), 2*y);
	PxTransform localFrame(PxVec3(0, 0, 0));
	PxFixedJoint* fixed = PxFixedJointCreate(*gPhysics, fan, localFrame, sphere, localFrame);
}

/*风扇（旋转杆）关卡
t为该刚体构建的相对原点
v为该刚体中心点相对其构建原点的位置
rod_length, rod_height, rod_width为杆的长、高、宽
abgularVelocity 风扇旋转角速度，这里扇面水平，通过角速度PxVec3(0,y,0)中y的大小控制其旋转速度*/
void createRoTateRodLevel(const PxTransform& t, PxVec3 v,float rod_length,float rod_height,float rod_width, float boxHeight, PxVec3 angularVelocity, PxTransform& pose) {
	createIceRoad(t, v, rod_length, boxHeight, rod_length, pose);
	//createRotateRod(t, PxVec3(v.x, v.y + boxHeight + 2.0, v.z), 1.5, pose, PxVec3(1, 0, 0) * 70, PxVec3(0, 0, -1) * 70);
	createFan(t, PxVec3(v.x, v.y + 2 * boxHeight + 1.7f, v.z), rod_length, rod_height, rod_width, angularVelocity);

	//旋转杆1中心点
	float rr1_x = v.x - 2 * rod_length;
	float rr1_y = v.y;
	float rr1_z = v.z + rod_length + 0.5;
	//中间路段
	createIceRoad(t, PxVec3(rr1_x, rr1_y, v.z), rod_length, boxHeight, 2 * rod_length + 1.0, pose);
	//createRotateRod(t, PxVec3(rr1_x, rr1_y + 2 * boxHeight, rr1_z), 1.5, pose, PxVec3(1, 0, 0) * 70, PxVec3(0, 0, -1) * 70);
	createFan(t, PxVec3(rr1_x, rr1_y + 2 * boxHeight + 1.7f, rr1_z), rod_length, rod_height, rod_width, angularVelocity);

	//旋转杆2中心点
	float rr2_x = rr1_x;
	float rr2_y = rr1_y;
	float rr2_z = v.z - rod_length - 0.5;
	//createRotateRod(t, PxVec3(rr2_x, rr2_y + 2 * boxHeight, rr2_z), 1.5, pose, PxVec3(-1, 0, 0) * 70, PxVec3(0, 0, 1) * 70);
	createFan(t, PxVec3(rr2_x, rr2_y + 2 * boxHeight + 1.7f, rr2_z), rod_length, rod_height, rod_width, -angularVelocity);

	//旋转杆3中心点
	float rr3_x = v.x - 4 * rod_length;
	float rr3_y = v.y;
	float rr3_z = v.z;
	createIceRoad(t, PxVec3(rr3_x, rr3_y, rr3_z), rod_length, boxHeight, rod_length, pose);
	//createRotateRod(t, PxVec3(rr3_x, rr3_y + 2 * boxHeight, rr3_z), 1.5, pose, PxVec3(-1, 0, 0) * 70, PxVec3(0, 0, 1) * 70);
	createFan(t, PxVec3(rr3_x, rr3_y + 2 * boxHeight + 1.7f, rr3_z), rod_length, rod_height, rod_width, -angularVelocity);
}

/*创建水池
t为该水池构建的相对原点
bottom为该水池底部中心点相对其构建原点的位置
poolLength, poolHeight, poolWidth为杆的长、高、宽
pose刚体初始姿态*/
void createPool(const PxTransform& t, PxVec3 bottom, float poolLength, float poolHeight,float poolWidth, PxTransform& pose) {
	PxTransform pos(t.transform(PxTransform(bottom)));
	//PxTransform rotate = PxTransform(PxQuat(PxHalfPi / 10, PxVec3(0, 0, -1.0f)));
	//底部
	createStaticBox(pos, PxVec3(0, poolHeight + 6.0f, 0), poolLength + 2.0, 1.0, poolWidth + 2.0, pose, OrganType::poolWall);
	//左侧
	createStaticBox(pos, PxVec3(1.0 + poolLength, 1.0 + poolHeight, 0), 5.0, poolHeight, poolWidth + 2.0, pose, OrganType::poolWall);
	//右侧
	createStaticBox(pos, PxVec3(- 1.0 - poolLength,  1.0 + poolHeight, 0), 1.0, poolHeight, poolWidth + 2.0 , pose, OrganType::poolWall);
	//前侧
	createStaticBox(pos, PxVec3(0,  1.0 + poolHeight, - 1.0 - poolWidth), poolLength, poolHeight, 1.0, pose, OrganType::poolWall);
	//后侧
	createStaticBox(pos, PxVec3(0,  1.0 + poolHeight, 1.0 + poolWidth), poolLength, poolHeight, 1.0, pose, OrganType::poolWall);
}
/*创建泳池球体形状的动态刚体
*/
PxRigidDynamic* createParticleSphere(const PxTransform& t, const PxVec3& v, PxReal halfExtend) {
	PxTransform local(v);
	PxShape* shape = gPhysics->createShape(PxSphereGeometry(halfExtend), *gMaterial);
	shape->setQueryFilterData(collisionGroup);
	PxRigidDynamic* sceneBox;
	for (int i = 0; i < 100; i++) {
		sceneBox = gPhysics->createRigidDynamic(t.transform(local));
		sceneBox->attachShape(*shape);
		sceneBox->setAngularDamping(1.0f);
		sceneBox->setName("Particle");
		gScene->addActor(*sceneBox);
	}
	return sceneBox;
}


/*旋转杆关卡与水池连接处的齿轮
t为该刚体构建的相对原点
v为该刚体中心点相对其构建原点的位置
x,y,z为组成齿轮盒子的长、高、宽
abgularVelocity 齿轮旋转角速度，这里扇面竖直，通过角速度PxVec3(0,0,z)中z的大小控制其旋转速度*/
void createGear(const PxTransform& t, PxVec3 v, float x, float y, float z, PxVec3 angularVelocity) {
	PxTransform pos(t.transform(v));
	PxRigidDynamic* gear = gPhysics->createRigidDynamic(pos);
	gear->setName("Gear");
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
	RotateRod* rotateRod = new RotateRod("Gear", gear->getGlobalPose().p, 25.0, 1.0, 1.0, gear);
	gear->userData = rotateRod;
}

/*创建摩天轮
t为该刚体构建的相对原点
v为该刚体中心点相对其构建原点的位置
x,y,z为杆的长、高、宽
seatLength,seatHeight,seatWidth为旋转踏板的长高宽
distance为骨架与踏板间缝隙大小
abgularVelocity 摩天轮旋转角速度，这里扇面竖直，通过角速度PxVec3(0,0,z)中z的大小控制其旋转速度*/
void createFerrisWheel(const PxTransform& t, PxVec3 v, float x, float y, float z, float seatLength, float seatHeight, float seatWidth, float distance, PxVec3 angularVelocity) {
	PxTransform pos(t.transform(v));
	PxRigidDynamic* ferrisWheelActor = gPhysics->createRigidDynamic(pos);
	PxShape* shape0 = PxRigidActorExt::createExclusiveShape(*ferrisWheelActor, PxBoxGeometry(x, y, z), *gMaterial);
	PxShape* shape1 = PxRigidActorExt::createExclusiveShape(*ferrisWheelActor, PxBoxGeometry(x, y, z), *gMaterial);
	PxShape* shape2 = PxRigidActorExt::createExclusiveShape(*ferrisWheelActor, PxBoxGeometry(x, y, z), *gMaterial);
	shape1->setLocalPose(PxTransform(PxQuat(PxPi / 3, PxVec3(0, 0, 1))));
	shape2->setLocalPose(PxTransform(PxQuat(2 * PxPi / 3, PxVec3(0, 0, 1))));
	shape0->setQueryFilterData(collisionGroup);
	shape1->setQueryFilterData(collisionGroup);
	shape2->setQueryFilterData(collisionGroup);
	ferrisWheelActor->setName("FerrisWheel");
	FerrisWheel* ferrisWheel = new FerrisWheel("FerrisWheel", ferrisWheelActor->getGlobalPose().p, x, y, z, ferrisWheelActor);
	ferrisWheelActor->userData = ferrisWheel;
	//这里加一个ferrisWheel类或者用现有的的绑到userdata里面即可，把userdata的类的name设为angularVelocity即可在release识别
	ferrisWheelActor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	ferrisWheelActor->setAngularVelocity(angularVelocity);
	ferrisWheelActor->setAngularDamping(0.f);
	gScene->addActor(*ferrisWheelActor);
	ferrisWheelActor->setMass(0.f);
	ferrisWheelActor->setMassSpaceInertiaTensor(PxVec3(0.f));
	PxTransform pose(PxVec3(0));
	//seat0左
	PxRigidDynamic* seat0 = createDynamicBox(false, pos, PxVec3(0.9 * x, -2.0, -z - seatWidth - distance), seatLength, seatHeight, seatWidth, pose, OrganType::prismaticRoad);
	seat0->setMass(1.0);
	seat0->setMassSpaceInertiaTensor(PxVec3(0.f));
	PxTransform fw0(PxVec3(0.9 * x, 0, -z - seatWidth - distance));
	PxTransform s0(PxVec3(0, 2.0, 0));
	PxDistanceJoint* d0 = PxDistanceJointCreate(*gPhysics, ferrisWheelActor, fw0, seat0, s0);
	/*d0->setMaxDistance(2.0f);
	d0->setDistanceJointFlag(PxDistanceJointFlag::eMAX_DISTANCE_ENABLED, true);*/
	//seat1右
	PxRigidDynamic* seat1 = createDynamicBox(false, pos, PxVec3(-0.9 * x, -2.0, -z - seatWidth - distance), seatLength, seatHeight, seatWidth, pose, OrganType::prismaticRoad);
	seat1->setMass(1.0);
	seat1->setMassSpaceInertiaTensor(PxVec3(0.f));
	PxTransform fw1(PxVec3(-0.9 * x, 0, -z - seatWidth - distance));
	PxTransform s1(PxVec3(0, 2.0, 0));
	PxDistanceJoint* d1 = PxDistanceJointCreate(*gPhysics, ferrisWheelActor, fw1, seat1, s1);
	/*d1->setMaxDistance(2.0f);
	d1->setDistanceJointFlag(PxDistanceJointFlag::eMAX_DISTANCE_ENABLED, true);*/
	//seat2左上
	PxRigidDynamic* seat2 = createDynamicBox(false, pos, PxVec3(0.9 * x * cos(PxPi / 3), 0.9 * x * sin(PxPi / 3) - 2.0, -z - seatWidth - distance),seatLength, seatHeight, seatWidth, pose, OrganType::prismaticRoad);
	seat2->setMass(1.0);
	seat2->setMassSpaceInertiaTensor(PxVec3(0.f));
	PxTransform fw2(PxVec3(0.9 * x * cos(PxPi / 3), 0.9 * x * sin(PxPi / 3), -z - seatWidth - distance));
	PxTransform s2(PxVec3(0, 2.0, 0));
	PxDistanceJoint* d2 = PxDistanceJointCreate(*gPhysics, ferrisWheelActor, fw2, seat2, s2);
	/*d2->setMaxDistance(2.0f);
	d2->setDistanceJointFlag(PxDistanceJointFlag::eMAX_DISTANCE_ENABLED, true);*/
	//seat3右上
	PxRigidDynamic* seat3 = createDynamicBox(false, pos, PxVec3(-0.9 * x * cos(PxPi / 3), 0.9 * x * sin(PxPi / 3) - 2.0, -z - seatWidth - distance), seatLength, seatHeight, seatWidth, pose, OrganType::prismaticRoad);
	seat3->setMass(1.0);
	seat3->setMassSpaceInertiaTensor(PxVec3(0.f));
	PxTransform fw3(PxVec3(-0.9 * x * cos(PxPi / 3), 0.9 * x * sin(PxPi / 3), -z - seatWidth - distance));
	PxTransform s3(PxVec3(0, 2.0, 0));
	PxDistanceJoint* d3 = PxDistanceJointCreate(*gPhysics, ferrisWheelActor, fw3, seat3, s3);
	/*d3->setMaxDistance(2.0f);
	d3->setDistanceJointFlag(PxDistanceJointFlag::eMAX_DISTANCE_ENABLED, true);*/
	//seat4左下
	PxRigidDynamic* seat4 = createDynamicBox(false, pos, PxVec3(0.9 * x * cos(PxPi / 3), -0.9 * x * sin(PxPi / 3) - 2.0, -z - seatWidth - distance), seatLength, seatHeight, seatWidth, pose, OrganType::prismaticRoad);
	seat4->setMass(1.0);
	seat4->setMassSpaceInertiaTensor(PxVec3(0.f));
	PxTransform fw4(PxVec3(0.9 * x * cos(PxPi / 3), -0.9 * x * sin(PxPi / 3), -z - seatWidth - distance));
	PxTransform s4(PxVec3(0, 2.0, 0));
	PxDistanceJoint* d4 = PxDistanceJointCreate(*gPhysics, ferrisWheelActor, fw4, seat4, s4);
	/*d4->setMaxDistance(2.0f);
	d4->setDistanceJointFlag(PxDistanceJointFlag::eMAX_DISTANCE_ENABLED, true);*/
	//seat5右下
	PxRigidDynamic* seat5 = createDynamicBox(false, pos, PxVec3(-0.9 * x * cos(PxPi / 3), -0.9 * x * sin(PxPi / 3) - 2.0, -z - seatWidth - distance), seatLength, seatHeight, seatWidth, pose, OrganType::prismaticRoad);
	seat5->setMass(1.0);
	seat5->setMassSpaceInertiaTensor(PxVec3(0.f));
	PxTransform fw5(PxVec3(-0.9 * x * cos(PxPi / 3), -0.9 * x * sin(PxPi / 3), -z - seatWidth - distance));
	PxTransform s5(PxVec3(0, 2.0, 0));
	PxDistanceJoint* d5 = PxDistanceJointCreate(*gPhysics, ferrisWheelActor, fw5, seat5, s5);
	/*d5->setMaxDistance(2.0f);
	d5->setDistanceJointFlag(PxDistanceJointFlag::eMAX_DISTANCE_ENABLED, true);*/
	//中心球体
	PxRigidDynamic* sphere = createDynamicSphere(pos, PxVec3(0, 0, 0), 2 * y);
	PxTransform center(PxVec3(0));
	PxFixedJointCreate(*gPhysics, ferrisWheelActor, center, sphere, center);
	//对称骨架
	PxTransform pos1(pos.transform(PxTransform(PxVec3(0, 0, -2 * z - 2 * distance - 2 * seatWidth))));
	PxRigidDynamic* ferrisWheelActor1 = gPhysics->createRigidDynamic(pos1);
	PxShape* shape3 = PxRigidActorExt::createExclusiveShape(*ferrisWheelActor1, PxBoxGeometry(x, y, z), *gMaterial);
	PxShape* shape4 = PxRigidActorExt::createExclusiveShape(*ferrisWheelActor1, PxBoxGeometry(x, y, z), *gMaterial);
	PxShape* shape5 = PxRigidActorExt::createExclusiveShape(*ferrisWheelActor1, PxBoxGeometry(x, y, z), *gMaterial);
	shape4->setLocalPose(PxTransform(PxQuat(PxPi / 3, PxVec3(0, 0, 1))));
	shape5->setLocalPose(PxTransform(PxQuat(2 * PxPi / 3, PxVec3(0, 0, 1))));
	shape3->setQueryFilterData(collisionGroup);
	shape4->setQueryFilterData(collisionGroup);
	shape5->setQueryFilterData(collisionGroup);
	ferrisWheelActor1->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	ferrisWheelActor1->setAngularVelocity(angularVelocity);
	ferrisWheelActor1->setAngularDamping(0.f);
	gScene->addActor(*ferrisWheelActor1);
	ferrisWheelActor1->setMass(0.f);
	ferrisWheelActor1->setMassSpaceInertiaTensor(PxVec3(0.f));
	ferrisWheelActor1->setName("FerrisWheel");
	FerrisWheel* ferrisWheel1 = new FerrisWheel("FerrisWheel", ferrisWheelActor1->getGlobalPose().p, x, y, z, ferrisWheelActor1);
	ferrisWheelActor1->userData = ferrisWheel1;
	//另一中心球体
	PxRigidDynamic* sphere1 = createDynamicSphere(pos1, PxVec3(0, 0, 0), 2 * y);
	PxFixedJointCreate(*gPhysics, ferrisWheelActor1, center, sphere1, center);
}

/*旋转路段关卡（由fan组件构成）
t为该关卡构建的相对原点
v为第一个旋转机关中心点相对其构建原点的位置
x,y,z为风扇杆的长、高、宽
abgularVelocity 风扇旋转角速度，这里扇面水平，通过角速度PxVec3(0,y,0)中y的大小控制其旋转速度
pose为刚体初始姿态*/
float createFanRoadLevel(const PxTransform& t, PxVec3 v, float x, float y, float z,PxVec3 angularVelocity, PxTransform& pose) {
	PxTransform pos(t.transform(v));
	float scale = 1.15;
	createFan(pos, PxVec3(0), x, y, z, angularVelocity);
	/*PxVec3 fanPos1(scale * 2 * x * cos(PxHalfPi / 2), 0, scale * 2 * x * cos(PxHalfPi / 2));
	createFan(pos, fanPos1, x, y, z, -angularVelocity);
	PxVec3 fanPos2(0, 0, scale * 4 * x * cos(PxHalfPi / 2));
	createFan(pos, fanPos2, x, y, z, angularVelocity);
	return v.z + fanPos2.z;*/
	PxVec3 fanPos2(0, 0, scale * 2.7 * x * cos(PxHalfPi / 2));
	createFan(pos, fanPos2, x, y, z, angularVelocity);
	return v.z + fanPos2.z;
}

void createPrismatic(const PxTransform& t, PxVec3 v, const char* name, float x, float y, float z, PxVec3 startPos, PxVec3 endPos, PxTransform& pose) {
	PxRigidDynamic* actor1 = createDynamicBox(false, t, v, x, y, z, pose, OrganType::prismaticRoad,PxVec3(0, 0, 0));
	PxVec3 position = actor1->getGlobalPose().p;
	PrismaticRoad* prismaticRoad = new PrismaticRoad(name, position, x, y, z, startPos, endPos, actor1);
	actor1->userData = prismaticRoad;
	actor1->setName(name);
	actor1->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	actor1->setMass(0.f);
	actor1->setMassSpaceInertiaTensor(PxVec3(0.f));
}

void createSyntheticLevel(const PxTransform& t, PxVec3 v, float halfExtend, float roadblock_length, float roadblock_width, float sideSeesaw_length, float sideSeesaw_Height, float sideSeesaw_width, PxTransform& pose) {
	PxTransform pos(t.transform(PxTransform(v)));
	createNewPendulum(pos, PxVec3(0, boxHeight + halfExtend + 0.5, 0), halfExtend, 0.6, 10.0, 0.6, "Pendulum0", pose, PxVec3(0, 0, -2));
	createSideSeesaw(pos, PxVec3(-halfExtend - 0.7 * dx, 0, -roadblock_width - 0.7*dz), sideSeesaw_width, 1.0, sideSeesaw_length, pose);
	createNewPendulum(pos, PxVec3(dx + 4 * halfExtend, boxHeight + halfExtend + 0.5, 0), halfExtend, 0.6, 10.0, 0.6, "Pendulum1", pose, PxVec3(0, 0, 2));
	createSideSeesaw(pos, PxVec3(dx + 4 * halfExtend + 0.7 * dx, 0, -roadblock_width - 0.7*dz), sideSeesaw_width, 1.0, sideSeesaw_length, pose);
}

PxVec3 positions[20000];
//为每一个粒子生成坐标信息
PxVec3* createPositions(PxVec3 &p)
{
	int t = 0;

	for (int x = 0; x < 20000; x++)
	{
		positions[x] = p;
	}
	while(t<=2000)
	for (float m = -2; m <= 5; m += 4.0)
	{
		for (float l = -10; l <= 10; l += 4.0)
		{
			for (float n = -15; n <= 10; n += 4.0)
			{
					positions[t].x += n;
					positions[t].y += m;
					positions[t].z += l;
					t++;
			}

		}
	}
	return positions;
}

PxU32 indices[20000];
//为每一个粒子生成索引
PxU32* createParticleIndices(PxU32 num)
{
	
	for (PxU32 k = 0; k < num; k++)
	{
		indices[k] = k;
	}
	return indices;
}

//创建粒子
void createParticles(PxVec3 v)
{
	// set immutable properties.
	PxU32 maxParticles = 5000;
	bool perParticleRestOffset = false;

	// create particle system in PhysX SDK
	PxParticleSystem* ps = gPhysics->createParticleSystem(maxParticles, perParticleRestOffset);

	PxParticleCreationData particleCreationData;
	particleCreationData.numParticles = 42;
	PxVec3* p = createPositions(v);
	PxU32* indic = createParticleIndices(maxParticles);
	particleCreationData.indexBuffer = PxStrideIterator<const PxU32>(indic);
	particleCreationData.positionBuffer = PxStrideIterator<const PxVec3>(p);

	// create particles in *PxParticleSystem* ps
	bool success = ps->createParticles(particleCreationData);
	std::cout << "创建粒子成功！" << std::endl;

	// add particle system to scene, in case creation was successful
	if (ps)
		gScene->addActor(*ps);

	// lock SDK buffers of *PxParticleSystem* ps for reading
	PxParticleReadData* rd = ps->lockParticleReadData();

	// access particle data from PxParticleReadData
	if (rd)
	{
		PxStrideIterator<const PxParticleFlags> flagsIt(rd->flagsBuffer);
		PxStrideIterator<const PxVec3> positionIt(rd->positionBuffer);

		for (unsigned i = 0; i < rd->validParticleRange; ++i, ++positionIt)
		{
			if (*flagsIt & PxParticleFlag::eVALID)
			{
				// access particle position
				const PxVec3& position = *positionIt;
				std::cout << position.x << "," << position.y << "," << position.z << std::endl;
				createParticleSphere(position, 2.0);
			}
		}

		// return ownership of the buffers back to the SDK
		rd->unlock();
	}
}
//创建管道壁
PxRigidStatic* createPipeWall(const PxTransform& t, const PxVec3& v, PxReal x, PxReal y, PxReal z, PxTransform& pose) {
	PxRigidStatic* pipeWallActor = createStaticBox(t, v, x, y, z, pose, OrganType::pipeWall);
	PxVec3 position = pipeWallActor->getGlobalPose().p;
	PipeWall* pipeWall = new PipeWall("路面", position, x, y, z, pipeWallActor);
	pipeWallActor->userData = pipeWall;
	pipeWallActor->setName("PipeWall");
	return pipeWallActor;
}

//创建z轴方向管道
void createZPipe(const PxTransform& t, PxVec3 v,float pipeWidth, float pipeHeight, float length, PxTransform& pose) {
	PxTransform pos(t.transform(v));
	float tbWidth = pipeWidth + 1.0;
	createPipeWall(pos, PxVec3(0, 0, 0), tbWidth, 0.5, length, pose);
	createPipeWall(pos, PxVec3(0, 1.0 + 2 * pipeHeight, 0), tbWidth, 0.5, length, pose);
	createPipeWall(pos, PxVec3(pipeWidth + 0.5, pipeHeight + 0.5, 0), 0.5, pipeHeight, length, pose);
	createPipeWall(pos, PxVec3(-pipeWidth - 0.5, pipeHeight + 0.5, 0), 0.5, pipeHeight, length, pose);
}

//创建x轴方向管道
void createXPipe(const PxTransform& t, PxVec3 v, float pipeWidth, float pipeHeight, float length, PxTransform& pose) {
	PxTransform pos(t.transform(v));
	float tbWidth = pipeWidth + 1.0;
	createPipeWall(pos, PxVec3(0, 0, 0), length, 0.5, tbWidth, pose);
	createPipeWall(pos, PxVec3(0, 1.0 + 2 * pipeHeight, 0), length, 0.5, tbWidth, pose);
	createPipeWall(pos, PxVec3(0, 0.5 + pipeHeight, - 0.5 - pipeWidth), length, pipeHeight, 0.5, pose);
	createPipeWall(pos, PxVec3(0, 0.5 + pipeHeight, 0.5 + pipeWidth), length, pipeHeight, 0.5, pose);
}

//创建y轴方向管道
void createYPipe(const PxTransform& t, PxVec3 v, float pipeWidth, float pipeHeight, float length, PxTransform& pose) {
	PxTransform pos(t.transform(v));
	float tbWidth = pipeWidth + 1.0;
	createPipeWall(pos, PxVec3(0, 0, 0.5+pipeWidth), tbWidth, length, 0.5, pose);
	createPipeWall(pos, PxVec3(0, 0, -0.5-pipeWidth), tbWidth, length, 0.5, pose);
	createPipeWall(pos, PxVec3(0.5+pipeWidth, 0, 0), 0.5, length, pipeWidth, pose);
	createPipeWall(pos, PxVec3(-0.5-pipeWidth, 0, 0), 0.5, length, pipeWidth, pose);
}


void createPipeLevel(const PxTransform& t, PxVec3 v, PxTransform& pose) {
	PxTransform pos(t.transform(v));
	float pipeWidth = 4.0;
	float pipeHeight = 4.0;
	float standHeight = 6.0;
	float heightDistance = 15.0;
	//上下管壁的宽度
	float tbWidth = pipeWidth + 1.0;
	float p0_x = 0;
	float p0_y = 0;
	float p0_width = 20.0;
	float p0_z = p0_width;
	//入口分支0
	createZPipe(pos,PxVec3(p0_x, p0_y, p0_z), pipeWidth, pipeHeight, p0_width, pose);
	/*createPipeWall(pos, PxVec3(0, 0, p0_z), tbWidth, 0.5, p0_width, pose);
	createPipeWall(pos, PxVec3(0, 1.0 + 2 * pipeHeight, p0_z), tbWidth, 0.5, p0_width, pose);
	createPipeWall(pos, PxVec3(pipeWidth+0.5, pipeHeight+0.5, p0_z), 0.5, pipeHeight, p0_width, pose);
	createPipeWall(pos, PxVec3(-pipeWidth - 0.5, pipeHeight + 0.5, p0_z), 0.5, pipeHeight, p0_width, pose);*/
	//第一个分岔口 cross
	float cross_x = p0_x;
	float cross_y = p0_y;
	float cross_z = p0_z + p0_width + pipeWidth;
	createPipeWall(pos, PxVec3(cross_x, cross_y, cross_z), tbWidth, 0.5, pipeWidth, pose);
	createPipeWall(pos, PxVec3(cross_x, cross_y+ 1.0 + 2 * pipeHeight, cross_z), tbWidth, 0.5, pipeWidth, pose);
	createPipeWall(pos, PxVec3(cross_x - pipeWidth - 0.5, cross_y + pipeHeight + 0.5, cross_z), 0.5, pipeHeight, pipeWidth, pose);
	//第一个分岔口左边分支1
	float p1_l = 20.0;
	float p1_x = cross_x + 1.0 + pipeWidth + p1_l;
	float p1_y = cross_y;
	float p1_z = cross_z;
	createXPipe(pos, PxVec3(p1_x, p1_y, p1_z), pipeWidth, pipeHeight, p1_l, pose);
	//分支1分岔口cross1
	float cross1_x = p1_x + p1_l + pipeWidth;
	float cross1_y = p1_y;
	float cross1_z = p1_z;
	createPipeWall(pos, PxVec3(cross1_x, cross1_y, cross1_z), pipeWidth, 0.5, tbWidth, pose);
	createPipeWall(pos, PxVec3(cross1_x, cross1_y + 1.0 + 2 * pipeHeight, cross1_z), pipeWidth, 0.5, tbWidth, pose);
	createPipeWall(pos, PxVec3(cross1_x+pipeWidth+0.5, cross1_y + 0.5 + pipeHeight, cross1_z), 0.5, tbWidth, tbWidth, pose);
	createPipeWall(pos, PxVec3(cross1_x, cross1_y + 0.5 + pipeHeight, cross1_z - pipeWidth-0.5), pipeWidth, pipeHeight, 0.5, pose);
	float p2_width = 15.0;
	float p2_x = cross1_x;
	float p2_y = cross1_y;
	float p2_z = cross1_z + 1.0 + pipeWidth + p2_width;
	createZPipe(pos, PxVec3(p2_x, p2_y, p2_z), pipeWidth, pipeHeight, p2_width, pose);
	//分支1分岔口cross2
	float cross2_x = p2_x;
	float cross2_y = p2_y;
	float cross2_z = p2_z + p2_width + pipeWidth;
	createPipeWall(pos, PxVec3(cross2_x, cross2_y, cross2_z), tbWidth, 0.5, pipeWidth, pose);
	createPipeWall(pos, PxVec3(cross2_x, cross2_y + 1.0 + 2 * pipeHeight, cross2_z), tbWidth, 0.5, pipeWidth, pose);
	createPipeWall(pos, PxVec3(cross2_x, cross2_y+0.5+pipeHeight, cross2_z+pipeWidth+0.5), tbWidth, tbWidth, 0.5, pose);
	createPipeWall(pos, PxVec3(cross2_x - pipeWidth - 0.5, cross2_y + 0.5 +pipeHeight, cross2_z), 0.5, pipeHeight, pipeWidth, pose);
	float p3_l = 15.0;
	float p3_x = cross2_x + 1.0 + pipeWidth + p3_l;
	float p3_y = cross2_y;
	float p3_z = cross2_z;
	float pr3_l = 6.0;
	float pr3_w = 6.0;
	PxVec3 pos1(pos.transform(PxTransform(PxVec3(p3_x+p3_l+pr3_l+0.2*dx, p3_y-2.0, p3_z))).p);
	createXPipe(pos, PxVec3(p3_x, p3_y, p3_z), pipeWidth, pipeHeight, p3_l, pose);
	//分支2
	float p4_width = 35.0;
	float p4_x = cross_x;
	float p4_y = cross_y;
	float p4_z = cross_z + pipeWidth + p4_width;
	createZPipe(pos, PxVec3(p4_x, p4_y, p4_z), pipeWidth, pipeHeight, p4_width, pose);
	//分支2分岔口cross3
	float cross3_x = p4_x;
	float cross3_y = p4_y;
	float cross3_z = p4_z + p4_width + pipeWidth;
	createPipeWall(pos, PxVec3(cross3_x, cross3_y, cross3_z), pipeWidth, 0.5, pipeWidth, pose);
	createPipeWall(pos, PxVec3(cross3_x, cross3_y + 0.5 + pipeHeight, cross3_z + 0.5 + pipeWidth), tbWidth, pipeHeight + 1.0, 0.5, pose);
	createPipeWall(pos, PxVec3(cross3_x + pipeWidth + 0.5, cross3_y + 0.5 + pipeHeight, cross3_z), 0.5, pipeHeight + 1.0, pipeWidth, pose);
	createPipeWall(pos, PxVec3(cross3_x - pipeWidth - 0.5, cross3_y + 0.5 + pipeHeight, cross3_z), 0.5, pipeHeight + 1.0, pipeWidth, pose);
	//两层连接管道
	float p5_x = cross3_x;
	float p5_y = cross3_y + 2*pipeHeight + 1.0 + heightDistance;
	float p5_z = cross3_z;
	createYPipe(pos, PxVec3(p5_x, p5_y, p5_z), pipeWidth, pipeHeight, heightDistance, pose);
	//连接管道另一头cross4
	float cross4_x = p5_x;
	float cross4_y = p5_y + heightDistance + pipeWidth;
	float cross4_z = p5_z;
	createPipeWall(pos, PxVec3(cross4_x, cross4_y+0.5+pipeWidth, cross4_z), tbWidth, 0.5, tbWidth, pose);
	createPipeWall(pos, PxVec3(cross4_x, cross4_y, cross4_z+0.5+pipeWidth), tbWidth, pipeWidth, 0.5, pose);
	createPipeWall(pos, PxVec3(cross4_x, cross4_y, cross4_z-0.5-pipeWidth), tbWidth, pipeWidth, 0.5, pose);
	createPipeWall(pos, PxVec3(cross4_x-0.5-pipeWidth, cross4_y, cross4_z), 0.5, pipeWidth, pipeWidth, pose);
	//二层入一层前连接管道
	float p6_l = 10.0;
	float p6_x = cross4_x + 1.0 + pipeWidth + p6_l;
	float p6_y = cross4_y - pipeWidth - 0.5;
	float p6_z = cross4_z;
	createXPipe(pos, PxVec3(p6_x, p6_y, p6_z), pipeWidth, pipeWidth, p6_l, pose);
	//第二层两分支交叉口cross5
	float cross5_x = p6_x + p6_l + pipeWidth;
	float cross5_y = p6_y;
	float cross5_z = p6_z;
	createPipeWall(pos, PxVec3(cross5_x, cross5_y, cross5_z), pipeWidth, 0.5, tbWidth, pose);
	createPipeWall(pos, PxVec3(cross5_x, cross5_y+1.0+2*pipeWidth, cross5_z), pipeWidth, 0.5, tbWidth, pose);
	createPipeWall(pos, PxVec3(cross5_x, cross5_y+0.5+pipeWidth, cross5_z-pipeWidth-0.5), pipeWidth, pipeWidth, 0.5, pose);
	//分支3
	float p7_l = p3_l+p1_l-p6_l;
	float p7_x = cross5_x + pipeWidth + p7_l;
	float p7_y = cross5_y;
	float p7_z = cross5_z;
	createXPipe(pos, PxVec3(p7_x, p7_y, p7_z), pipeWidth, pipeWidth, p7_l, pose);
	float sr_l = 6.0;
	float sr_w = 8.0;
	createRoad(pos, PxVec3(p7_x + p7_l + sr_l, p7_y, p7_z), sr_l, 1.0, sr_w, pose);
	PxVec3 pos2(pos.transform(PxTransform(PxVec3(p7_x + p7_l + pr3_l+0.2*dx, p7_y-3.0, p7_z - 0.5 * dz - pr3_w-pipeWidth))).p);
	createPrismatic(pos, PxVec3(p3_x + p3_l + pr3_l + 0.2 * dx, p3_y - 2.0, p3_z), "PrismaticRoad2", pr3_l, 1.0, pr3_w, pos1, pos2, pose);
	//cout << "平移路段3:" << p3_y - 2.0 << "," << p7_y - 3.0 << "," << p3_z << "," << p7_z - 0.5 * dz - pr3_w - pipeWidth << endl;
	//分支4
	float p8_width = 20.0;
	float p8_x = cross5_x;
	float p8_y = cross5_y;
	float p8_z = cross5_z + pipeWidth + p8_width;
	createZPipe(pos, PxVec3(p8_x, p8_y, p8_z), pipeWidth, pipeWidth, p8_width, pose);
	//分支4转弯 cross6
	float cross6_x = p8_x;
	float cross6_y = p8_y;
	float cross6_z = p8_z + p8_width + pipeWidth;
	createPipeWall(pos, PxVec3(cross6_x, cross6_y, cross6_z), tbWidth, 0.5, pipeWidth, pose);
	createPipeWall(pos, PxVec3(cross6_x, cross6_y+1.0+2*pipeWidth, cross6_z), tbWidth, 0.5, pipeWidth, pose);
	createPipeWall(pos, PxVec3(cross6_x, cross6_y+0.5+pipeWidth, cross6_z+0.5+pipeWidth), tbWidth, tbWidth, 0.5, pose);
	createPipeWall(pos, PxVec3(cross6_x + pipeWidth + 0.5, cross6_y+0.5+pipeWidth, cross6_z), 0.5, pipeWidth, pipeWidth, pose);
	float p9_l = 18.0;
	float p9_x = cross6_x - pipeWidth - 1.0 - p9_l;
	float p9_y = cross6_y;
	float p9_z = cross6_z;
	createXPipe(pos, PxVec3(p9_x, p9_y, p9_z), pipeWidth, pipeWidth, p9_l, pose);
	//转弯cross7
	float cross7_x = p9_x - p9_l - pipeWidth;
	float cross7_y = p9_y;
	float cross7_z = p9_z;
	createPipeWall(pos, PxVec3(cross7_x, cross7_y, cross7_z), pipeWidth, 0.5, tbWidth, pose);
	createPipeWall(pos, PxVec3(cross7_x, cross7_y+1.0+2*pipeWidth, cross7_z), pipeWidth, 0.5, tbWidth, pose);
	createPipeWall(pos, PxVec3(cross7_x-0.5-pipeWidth, cross7_y+0.5+pipeWidth, cross7_z), 0.5, tbWidth, tbWidth, pose);
	createPipeWall(pos, PxVec3(cross7_x, cross7_y+0.5+pipeWidth, cross7_z+0.5+pipeWidth), pipeWidth,pipeWidth,0.5, pose);
	float p10_width = 35.0;
	float p10_x = cross7_x;
	float p10_y = cross7_y;
	float p10_z = cross7_z-pipeWidth-1.0-p10_width;
	createZPipe(pos, PxVec3(p10_x, p10_y, p10_z), pipeWidth, pipeWidth, p10_width, pose);
	float p11_l = 6.0;
	float p11_w = 8.0;
	createRoad(pos, PxVec3(p10_x, p10_y-3.0, p10_z - p10_width - p11_w), p11_l, 1.0, p11_w, pose);
	createPorp(pos, PxVec3(p10_x, p10_y, p10_z - p10_width - p11_w), 1.0, 1.0, 1.0);

}

//创建游戏场景
void createGameScene(const PxTransform& t) {
	PxTransform defaultPose(PxQuat(0, PxVec3(0, 1, 0)));  //刚体默认pose
	
	//地面 接触判定死亡用
	PxRigidStatic* ground = createStaticBox(t, PxVec3(50, 1.0, 300), 500, 1.0, 500, defaultPose,OrganType::ground);
	ground->setName("Ground0");

	//#Checkpoint1
	totalCheckpoint = 1;

	float r_1_l = 10.0;  //road_1_length 4
	float r_1_w = 20.0;  //road_1_width 15
	float c_1_y = 5.0 + boxHeight;  //the position of the center of road_1
	// create road_1
	createRoad(t, PxVec3(0, c_1_y, 0), r_1_l, boxHeight, r_1_w, defaultPose);

	//初始位置
	checkpoints.push_back(t.transform(PxVec3(0, c_1_y + 7.0, 0)));

	float r_2_w = 6.0; //4
	float r_2_l = 25.0; //20
	float c_2_x = r_2_l - r_1_l;
	float c_2_y = center_y(c_1_y);
	float c_2_z = r_1_w + r_2_w;
	createRoad(t, PxVec3(c_2_x, c_2_y, c_2_z), r_2_l , boxHeight, r_2_w, defaultPose);

	createPipeLevel(t, PxVec3(c_2_x, c_2_y+boxHeight-0.5, c_2_z+r_2_w), defaultPose);

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
	
	//最后一节台阶的中心点（center_x-2*stairsLength,centerHeight-2*boxHeight,center_z）
	cout << "摆锤前位置相对场景原点的坐标为:" << center_x - 2 * stairsLength << "," << centerHeight - 2 * boxHeight << "," << center_z << endl;
	
	//#Checkpoint2
	totalCheckpoint++;
	checkpoints.push_back(t.transform(PxVec3(center_x - 2 * stairsLength, centerHeight - 2 * boxHeight + 7.0, center_z)));

	//悬空路段/改版平移路段+摆锤
	float roadblock_length = 10.0;
	float roadblock_width = 8.0;
	//平移路段初始相对位置 PrismaticRoad1
	float rb_x = center_x - stairsLength + 0.7 * dx + roadblock_length;
	float rb_y = centerHeight - 2 * boxHeight;
	float rb_z = center_z;
	cout << "摆锤关卡平移路段初始x值" << t.transform(PxVec3(rb_x, rb_y, rb_z)).x << endl;
	//createRoad(t, PxVec3(rb_x, rb_y, rb_z), roadblock_length, boxHeight, roadblock_width, defaultPose);
	/*原悬空路段+摆锤关卡
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
	std::cout << "最后一块悬空路段的中心点相对场景原点的坐标为:" << rb_x - 2 * roadblock_length - dx << "," << rb_y << "," << rb_z << endl;*/
	//摆锤+跷跷板综合关卡
	float syn_x = center_x - 2 * stairsLength + 4 * dx + 2 * roadblock_length;
	float syn_y = rb_y;
	float syn_z = rb_z;
	createSyntheticLevel(t, PxVec3(syn_x, syn_y, syn_z), 5.0, roadblock_length, roadblock_width, 20.0, 1.0, 5.0, defaultPose);
	
	//第一块跷板的坐标
	/*float seesawpos_x = rb_x + roadblock_length + 3.5 * dx;*/
	float seesawpos_x = rb_x + roadblock_length + 13.5 * dx;
	float seesawpos_y = rb_y;
	float seesawpos_z = rb_z;
	float sx = 5.0;
	float sy = 1.0;
	float sz = 15.0;
	float rb_x1 = seesawpos_x - sx - 2 * sy - 0.5 - 0.7 * dx - roadblock_length;
	//平移路段始末位置(全局位置)及创建 PrismaticRoad1
	PxVec3 pr1_startPos(t.transform(PxVec3(rb_x, rb_y, rb_z)));
	PxVec3 pr1_endPos(t.transform(PxVec3(rb_x1, rb_y, rb_z)));
	createPrismatic(t, PxVec3(rb_x, rb_y, rb_z), "PrismaticRoad1", roadblock_length, boxHeight, roadblock_width, pr1_startPos, pr1_endPos, defaultPose);
	cout << "摆锤关卡平移路段终点x值" << t.transform(PxVec3(rb_x1, rb_y, rb_z)).x << endl;
	cout << "startpos.x" << pr1_startPos.x << endl;
	cout << "endpos.x" << pr1_endPos.x << endl;
	PxVec3 seesawpos_v(seesawpos_x, seesawpos_y, seesawpos_z);
	float lastSeesaw_x = createSeesawLevel(t, seesawpos_v, sx, sy, sz, defaultPose);
	//最后一块跷板的中心点（lastSeesaw_x,seesawpos_y, seesawpos_z）
	cout << "最后一块跷板的中心点相对场景原点的坐标为:" << lastSeesaw_x << "," << seesawpos_y << "," << seesawpos_z << endl;

	//#Checkpoint3
	totalCheckpoint++;

	//跷板与摩天轮之间连接路段
	float r_8_l = 15.0;
	float r_8_w = 6.0;
	float c_8_x = lastSeesaw_x + sx + dx + r_8_l;
	float c_8_y = seesawpos_y;
	float c_8_z = seesawpos_z;
	createRoad(t, PxVec3(c_8_x, c_8_y, c_8_z), r_8_l, boxHeight, r_8_w, defaultPose);

	checkpoints.push_back(t.transform(PxVec3(c_8_x, c_8_y + 7.0, c_8_z)));

	//跷板关卡后的摩天轮
	//杆的参数
	float fw_l = 40.0;//30
	float fw_h = 0.8;
	float fw_w = 0.8;
	//seat参数
	float seatLength = 8.0;//4
	float seatHeight = 0.4;
	float seatWidth = 6.0;//3
	float seatDistance = 0.4;
	float fw_x = c_8_x + r_8_l + 0.75 * fw_l;
	float fw_y = c_8_y + fw_l;
	float fw_z = c_8_z + seatWidth + seatDistance + fw_w;
	createFerrisWheel(t, PxVec3(fw_x, fw_y, fw_z), fw_l, fw_h, fw_w, seatLength, seatHeight, seatWidth, seatDistance, PxVec3(0, 0, 0));
	/*if (openDynamicBall) {
		//开启渲染圈
		createFerrisWheel(t, PxVec3(fw_x, fw_y, fw_z), fw_l, fw_h, fw_w, seatLength, seatHeight, seatWidth, seatDistance, PxVec3(0, 0, 0));
	}else{
		//关闭渲染圈
		createFerrisWheel(t, PxVec3(fw_x, fw_y, fw_z), fw_l, fw_h, fw_w, seatLength, seatHeight, seatWidth, seatDistance, PxVec3(0, 0, 0));
	}*/

	//#Checkpoint4
	totalCheckpoint++;

	/*原跷板与迷宫连接路段
	float r_3_l = 5.0;
	float r_3_w = 50.0;
	float c_3_x = lastSeesaw_x + sx + dx + r_3_l;
	float c_3_y = seesawpos_y;
	float c_3_z = seesawpos_z + r_3_w - 5.0;
	PxRigidStatic* r_3 = createRoad(t, PxVec3(c_3_x, c_3_y, c_3_z), r_3_l, boxHeight, r_3_w, defaultPose);
	std::cout << "连接路段相对场景原点的坐标:" << c_3_x << "," << c_3_y << "," << c_3_z << endl;
	checkpoints.push_back(t.transform(PxVec3(c_3_x, c_3_y + 7.0, c_3_z)));*/

	//连接摩天轮与旋转路关卡的路段
	float r_3_l = 6.0;
	float r_3_w = 10.0;
	float c_3_x = fw_x + 0.8 * fw_l + r_3_l;
	float c_3_y = fw_y + 0.7 * fw_l;
	float c_3_z = seesawpos_z + r_3_w - 2 * seatWidth;
	PxRigidStatic* r_3 = createRoad(t, PxVec3(c_3_x, c_3_y, c_3_z), r_3_l, boxHeight*0.5, r_3_w, defaultPose);
	cout <<"连接摩天轮与旋转路关卡的路段相对场景原点的坐标:"<< c_3_x << "," << c_3_y << "," << c_3_z << endl;

	checkpoints.push_back(t.transform(PxVec3(c_3_x, c_3_y + 7.0, c_3_z)));

	//风扇组件构建的旋转路段关卡
	float fan_l = 30.0;
	float fan_h = 1.0;
	float fan_w = 6.0;
	float fan0_x = c_3_x;
	float fan0_y = c_3_y;
	float fan0_z = c_3_z + r_3_w + 0.5 * dz + fan_l;
	float fan2_z;
	/*if (openDynamicBall) {
		//开启渲染圈速度
		fan2_z = createFanRoadLevel(t, PxVec3(fan0_x, fan0_y, fan0_z), fan_l, fan_h, fan_w, PxVec3(0, 0.3, 0), defaultPose);
	}
	else {
		//关闭渲染圈速度
		fan2_z = createFanRoadLevel(t, PxVec3(fan0_x, fan0_y, fan0_z), fan_l, fan_h, fan_w, PxVec3(0, 0.4, 0), defaultPose);
	}*/
	fan2_z = createFanRoadLevel(t, PxVec3(fan0_x, fan0_y, fan0_z), fan_l, fan_h, fan_w, PxVec3(0, 0, 0), defaultPose);
	//最后一个旋转路的中心点（fan0_x,fan0_y,fan2_z）

	//连接旋转路关卡与迷宫的路段
	float r_7_l = 6.0;
	float r_7_w = 10.0;
	float c_7_x = fan0_x;
	float c_7_y = fan0_y;
	float c_7_z = fan2_z + r_7_w + 0.5 * dz + fan_l;
	createRoad(t, PxVec3(c_7_x, c_7_y, c_7_z), r_7_l, boxHeight, r_7_w, defaultPose);
	cout << "连接旋转路关卡与迷宫的路段相对场景原点的坐标:" << c_7_x << "," << c_7_y << "," << c_7_z << endl;

	checkpoints.push_back(t.transform(PxVec3(c_7_x, c_7_y + 7.0, c_7_z)));

	//创建道具类场景
	//createPorp(t, PxVec3(c_2_x, c_2_y + boxHeight + 1.0, c_2_z), 1.0, 1.0, 1.0);
	
	//#Checkpoint5
	totalCheckpoint++;
	//迷宫边长缩放系数
	float scale = 0.8;
	//迷宫小正方形边长37*scale
	float sideLength = 37.0 * scale;
	//迷宫中心点坐标
	/*float mazePos_x = c_3_x - 1.5 * sideLength;
	float mazePos_y = c_3_y;
	float mazePos_z = c_3_z + r_3_w + 4 * sideLength;*/
	float mazePos_x = c_7_x - 1.5 * sideLength;
	float mazePos_y = c_7_y;
	float mazePos_z = c_7_z + r_7_w + 4 * sideLength;
	PxVec3 maze_v(mazePos_x, mazePos_y, mazePos_z);
	//迷宫地板边长
	//float mazeLength = 4 * sideLength+1;
	float mazeLength =  2 * sideLength;
	//创建迷宫地板
	//createRoad(t, maze_v, mazeLength, boxHeight, mazeLength, defaultPose);
	createRoad(t, maze_v+PxVec3(mazeLength,0.0f, mazeLength), mazeLength, boxHeight, mazeLength, defaultPose);
	createRoad(t, maze_v+ PxVec3(mazeLength, 0.0f, -mazeLength), mazeLength, boxHeight, mazeLength, defaultPose);
	createRoad(t, maze_v+ PxVec3(-mazeLength, 0.0f, mazeLength), mazeLength, boxHeight, mazeLength, defaultPose);
	createRoad(t, maze_v+ PxVec3(-mazeLength, 0.0f, -mazeLength), mazeLength, boxHeight, mazeLength, defaultPose);

	createMaze(t, maze_v, scale, defaultPose);
	//#Checkpoint6
	totalCheckpoint++;
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
	cout << "迷宫出口路段相对场景原点的坐标为:" <<c_4_x << "," << c_4_y << "," << c_4_z << endl;


	//楼梯1
	//stairsWidth ： 4.0
	//stairsLength ： 2.0
	float center_x1 = c_4_x - r_4_l - stairsLength;
	float centerHeight1 = center_y(c_4_y);
	float center_z1 = c_4_z + r_4_w - stairsWidth;
	checkpoints.push_back(t.transform(PxVec3(center_x1, centerHeight1 + 7.0, center_z1)));
	for (int i = 0; i <= 5; i++) {
		createRoad(t, PxVec3(center_x1, centerHeight1, center_z1), stairsLength, boxHeight, stairsWidth, defaultPose);
		centerHeight1 = center_y(centerHeight1);
		center_x1 -= 2 * stairsLength;
	}
    //最后一节台阶的中心点（center_x1+2*stairsLength,centerHeight1-2*boxHeight,center_z1）
	
	//平移路段0 prismaticRoad
	//roadblock_length : 5.0
	//roadblock_width : 6.0
	float c_5_l = 5.0;
	float c_5_w = 6.0;
	float c_5_x = center_x1 + stairsLength - c_5_l;
	float c_5_y = centerHeight1;
	float c_5_z = center_z1;
	float prismaticRoad0_length = 6.0;
	float prismaticRoad0_width = 8.0;
	/*使用平移关节实现
	PxVec3 pr_v0(-2.5 * roadblock_length, 0, 0);
	PxJointLinearLimitPair limits0(-30.0, -2.5 * roadblock_length, PxSpring(20.0, 0));
	PxVec3 velocity0((-1, 0, 0) * 30);
	createPrismaticRoad(t, PxVec3(c_5_x, c_5_y, c_5_z), roadblock_length, boxHeight, roadblock_width, defaultPose, pr_v0, roadblock_length, boxHeight, roadblock_width, defaultPose, limits0, velocity0);
	std::cout << "平移路段前相对场景原点的坐标为:" << c_5_x << "," << c_5_y << "," << c_5_z << endl;*/
	createRoad(t, PxVec3(c_5_x, c_5_y, c_5_z), c_5_l, boxHeight, c_5_w, defaultPose);
	float pr0_x = c_5_x - c_5_l - prismaticRoad0_length - 0.7 * dx;
	float pr0_y = c_5_y;
	float pr0_z = c_5_z;
	PxVec3 pr0_startPos(t.transform(PxVec3(pr0_x, pr0_y, pr0_z)));
	PxVec3 pr0_endPos(t.transform(PxVec3(pr0_x - 4.0 * dx, pr0_y, pr0_z)));
	createPrismatic(t, PxVec3(pr0_x, pr0_y, pr0_z), "PrismaticRoad0", prismaticRoad0_length, boxHeight, prismaticRoad0_width, pr0_startPos, pr0_endPos, defaultPose);

	//#Checkpoint7
	totalCheckpoint++;

	//旋转杆0中心点
	float rod_length = 25.0;
	float rod_height = 0.5;
	float rod_width = 1.0;
	float c_6_x = c_5_x - 45.0 - roadblock_length;
	float c_6_y = c_5_y;
	float c_6_z = c_5_z - roadblock_width - dz - rod_length;
	/*if (openDynamicBall) {
		//开启渲染圈速度
		createRoTateRodLevel(t, PxVec3(c_6_x, c_6_y, c_6_z), rod_length, rod_height, rod_width, boxHeight, PxVec3(0, 2, 0), defaultPose);
	}
	else {
		//关闭渲染圈速度
		createRoTateRodLevel(t, PxVec3(c_6_x, c_6_y, c_6_z), rod_length, rod_height, rod_width, boxHeight, PxVec3(0, 4, 0), defaultPose);
	}*/
	createRoTateRodLevel(t, PxVec3(c_6_x, c_6_y, c_6_z), rod_length, rod_height, rod_width, boxHeight, PxVec3(0, 0, 0), defaultPose);
	cout << "旋转杆关卡角落位置相对于场景原点的坐标为" << c_6_x + rod_length << "," << c_6_y << "," << c_6_z + rod_length << endl;
	//旋转关卡角落坐标添加到checkpoints
	checkpoints.push_back(t.transform(PxVec3(c_6_x + rod_length, c_6_y + 7.0, c_6_z + rod_length)));

	
	//风扇关卡与水池连接齿轮
	float gearLength = 10.0;
	float gearHeight = 0.4;
	float gearWidth = 16.0;
	float gear0_x = c_6_x - 5 * rod_length - dx * 0.5 - gearLength;
	float gear0_y = c_6_y;
	float gear0_z = c_6_z;
	float gear1_x = gear0_x - 2 * gearLength - dx * 0.5;
	createGear(t, PxVec3(gear0_x, gear0_y, gear0_z), gearLength, gearHeight, gearWidth, PxVec3(0, 0, 0));
	createGear(t, PxVec3(gear1_x, gear0_y, gear0_z), gearLength, gearHeight, gearWidth, PxVec3(0, 0, 0));
	/*if (openDynamicBall) {
		//开启渲染圈速度
		createGear(t, PxVec3(gear0_x, gear0_y, gear0_z), gearLength, gearHeight, gearWidth, PxVec3(0, 0, 2));
		createGear(t, PxVec3(gear1_x, gear0_y, gear0_z), gearLength, gearHeight, gearWidth, PxVec3(0, 0, 2));
	}
	else {
		//关闭渲染圈速度
		createGear(t, PxVec3(gear0_x, gear0_y, gear0_z), gearLength, gearHeight, gearWidth, PxVec3(0, 0, 4));
		createGear(t, PxVec3(gear1_x, gear0_y, gear0_z), gearLength, gearHeight, gearWidth, PxVec3(0, 0, 4));
	}*/

	//#Checkpoint8
	totalCheckpoint++;
	
	//水池
	float poolLength = 20.0;
	float poolHeight = 10.0;
	float poolWidth = 20.0;
	float bottom_x = gear1_x - gearLength - dx * 0.8 - poolLength - 2.0;
	float bottom_y = gear0_y + boxHeight - 2 * poolHeight - 1.0;
	float bottom_z = gear0_z;
	createPool(t, PxVec3(bottom_x, bottom_y, bottom_z), poolLength, poolHeight, poolWidth, defaultPose);
	//createParticleSphere(t, PxVec3(bottom_x, bottom_y + 15.0f , bottom_z), 0.6f);
	//水池底部的相对于场景原点t的位置 PxVec3 localPose(bottom_x,bottom_y,bottom_z)
	//全局位置 t.transform(PxTransform(localPose)).p
	//泳池关卡角落坐标添加到checkpoints
	//createSideSeesaw(t, PxVec3(-2, 20, 0), 5.0, 1.0, 15.0, defaultPose);
	//createPlane(PxVec3(0, 0, 0), PxVec3(0, 1, 0));

	
	createParticles(t.transform(PxVec3(bottom_x, bottom_y + 25.0f, bottom_z)));
	checkpoints.push_back(t.transform(PxVec3(bottom_x + 20.0f, bottom_y + 22.0f, bottom_z - 1.0f)));
	
	
	//#Checkpoint9
	totalCheckpoint++;
	//终点
	float r_9_l = 15.0;
	float r_9_w = 6.0;
	float c_9_x = bottom_x - poolLength - 2.0 - r_9_l * cos(PxHalfPi / 4);
	float c_9_y = bottom_y + 2 * poolHeight + 1.0 + r_9_l * sin(PxHalfPi / 4);
	float c_9_z = bottom_z;
	PxTransform r_9_pose(PxQuat(PxHalfPi / 4, PxVec3(0, 0, -1)));
	createRoad(t, PxVec3(c_9_x, c_9_y, c_9_z), r_9_l, 1.0, r_9_w, r_9_pose);

	float finalLine_l = 10;
	float finalLine_w = 10;
	float finalLine_x = c_9_x - r_9_l * cos(PxHalfPi / 4) - finalLine_l;
	float finalLine_y = c_9_y + r_9_l * sin(PxHalfPi / 4);
	float finalLine_z = c_9_z;
	createStaticBox(t, PxVec3(finalLine_x, finalLine_y, finalLine_z), finalLine_l, 1.0, finalLine_w, defaultPose, OrganType::finalLine);

	
	checkpoints.push_back(t.transform(PxVec3(finalLine_x, finalLine_y, finalLine_z)));
}