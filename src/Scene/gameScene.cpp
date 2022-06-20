#include <ctype.h>
#include "PxPhysicsAPI.h"
#include "../Block/Door.h"
#include "../Block/Road.h"
#include "../Block/Seesaw.h"
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
float maxJumpHeight = 3.0;
float boxHeight = 0.4 * maxJumpHeight;
float dx = 6.0;  //x_distance x轴方向上可跳跃的间隔长度
float dz = 6.0;  //z_distance z轴方向上可跳跃的间隔长度


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

const char* typeMapName(BlockType type) {
	switch (type)
	{
	case BlockType::road: {
		return "Road";
		break;
	}
	case BlockType::wall: {
		return "Wall";
		break;
	}
	case BlockType::door: {
		return "Door";
		break;
	}
	case BlockType::seesaw:{
		return "Seesaw";
	}
	case BlockType::seesawbox:{
		return "SeesawBox";
	}
	default:
		return "Block";
		break;
	}
}

//RenderBox renderbox;
PxRigidStatic* createStaticBox(const PxTransform& t, const PxVec3& v, PxReal x, PxReal y, PxReal z, PxTransform& pose, BlockType type = BlockType::block) {
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

PxRigidDynamic* createDynamicBox(bool kinematic, const PxTransform& t, const PxVec3& v, PxReal x, PxReal y, PxReal z, PxTransform& pose, BlockType type = BlockType::block, const PxVec3& velocity = PxVec3(0)) {
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


//创建道具类，区别只在Block(BlockType::prop)，试验用，可修改
void createPorp(const PxTransform& t, const PxVec3& v, PxReal x, PxReal y, PxReal z) {
	PxTransform local(v);
	//cout << v.x << " " << v.y << " " << v.z << endl;
	PxShape* shape = gPhysics->createShape(PxBoxGeometry(x, y, z), *gMaterial);
	//碰撞检测的过滤组
	shape->setQueryFilterData(collisionGroup);

	//setupFiltering(shape, FilterGroup::ePIG, FilterGroup::eBIRD);
	PxRigidDynamic* sceneBox = gPhysics->createRigidDynamic(t.transform(local));
	PxRigidBodyExt::updateMassAndInertia(*sceneBox, 1.0f);
	Block* block = new Block("道具", sceneBox->getGlobalPose().p, x, y, z);
	block->setType(BlockType::prop);
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


/**
* @brief 创建道路
**/
PxRigidStatic* createRoad(const PxTransform& t, const PxVec3& v, PxReal x, PxReal y, PxReal z, PxTransform& pose) {
	PxRigidStatic* roadActor = createStaticBox(t, v, x, y, z, pose, BlockType::road);
	//std::cout << "v::" <<  v.x << " " << v.y << " " << v.z << endl;
	PxVec3 position = roadActor->getGlobalPose().p;
	Road* road = new Road("路面",position, x, y, z,roadActor);
	roadActor->userData = road;
	return roadActor;
}

float center_y(float y) {
	return y + 2 * boxHeight;
}


//t为场景构建的原点 v为该门中心点相对场景原点的位置 scale为门的缩放系数
//第二个参数应该为PxVec3 v(v_x, 底下所有盒子的高+10*scale，v_z)
//轴心在中点
void createFrontDoor(const PxTransform& t, PxVec3 v, float scale, PxTransform& pose,bool canOpen = true) {
	PxTransform pos(t.transform(PxTransform(v)));//-17.8  -18.56 -18.561815

	PxReal x = 10 * scale;
	PxReal y = 5 * scale;
	PxReal z = 1 * scale;
	//实际-5  旋转角度小：-17.8  -18.56 -18.561815
	PxRigidStatic* actor1 = createStaticBox(pos, PxVec3(0, 0, 0), 6 * scale, 10 * scale, 1 * scale, pose, BlockType::wall);
	createStaticBox(pos, PxVec3(23 * scale, 0, 0), 6 * scale, 10 * scale, 1 * scale, pose, BlockType::wall);
	if (canOpen) {
		PxRigidDynamic* actor0 = createDynamicBox(false, pos, PxVec3(6.5 * scale, -5 * scale, 0), x, y, z, pose, BlockType::door);
		PxTransform localFrame0(PxVec3(0, 5 * scale, 0));
		PxTransform localFrame1(PxVec3(6.5 * scale, 0, 0));
		PxRevoluteJoint* revolute = PxRevoluteJointCreate(*gPhysics, actor0, localFrame0, actor1, localFrame1);
		Door* door = new Door("正门", actor0->getGlobalPose().p, x, y, z, actor0, canOpen, revolute);
		actor0->userData = door;
		PxJointAngularLimitPair limits(-PxPi / 130, 0, 0.01f);
		revolute->setLimit(limits);
		/*if (canOpen) {
			PxJointAngularLimitPair limits(-PxPi / 2, PxPi / 2, 0.01f);
			revolute->setLimit(limits);
		}
		else {
			PxJointAngularLimitPair limits(-PxPi / 130, 0, 0.01f);
			revolute->setLimit(limits);
		}*/
		revolute->setRevoluteJointFlag(PxRevoluteJointFlag::eLIMIT_ENABLED, true);
		revolute->setLocalPose(PxJointActorIndex::Enum::eACTOR1, PxTransform(PxVec3(6.5 * scale, 0, 0), PxQuat(PxHalfPi, PxVec3(0, 0, 1))));
		//return revolute;
	}
	else {
		PxRigidStatic* actor0 = createStaticBox(pos, PxVec3(11.5 * scale, 0, 0), y, x, z, pose, BlockType::door);
		Door* door = new Door("假门", actor0->getGlobalPose().p, x, y, z, false);
		actor0->userData = door;
	}
}


// joint在中点
void createSideDoor(const PxTransform& t, PxVec3 v, float scale, PxTransform& pose, bool canOpen = true) {
	PxTransform pos(t.transform(PxTransform(v)));
	PxReal x = 10 * scale;
	PxReal y = 1 * scale;
	PxReal z = 5 * scale;
	PxRigidStatic* actor1 = createStaticBox(pos, PxVec3(0, 0, 0), 1 * scale, 10 * scale, 6 * scale, pose, BlockType::wall);
	createStaticBox(pos, PxVec3(0, 0, 23 * scale), 1 * scale, 10 * scale, 6 * scale, pose, BlockType::wall);
	if (canOpen) {
		PxRigidDynamic* actor0 = createDynamicBox(false, pos, PxVec3(0, 0, 11.5 * scale), x, y, z, pose, BlockType::door);
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
		PxRigidStatic* actor0 =  createStaticBox(pos, PxVec3(0, 0, 11.5 * scale), y, x, z, pose, BlockType::door);
		Door* door = new Door("假门", actor0->getGlobalPose().p, x, y, z, false);
		actor0->userData = door;
	}
}

//创建迷宫关卡
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

//t为场景构建的原点 v为该跷板中心相对场景原点的位置 x、y、z为长板的长高宽
//跷板 PxVec3 v的第二个参数应该为 底下所有盒子的高+ y
PxRevoluteJoint* createSeesaw(const PxTransform& t,PxVec3 v,float x, float y, float z, PxTransform& pose) {
	PxTransform pos(t.transform(PxTransform(v)));
	PxRigidDynamic* actor0 = createDynamicBox(false, pos, PxVec3(0, 0, 0), x, y, z, pose, BlockType::seesaw);
	Seesaw* seesaw = new Seesaw("翘板",actor0->getGlobalPose().p,x,y,z,actor0);
	actor0->userData = seesaw;
	PxRigidStatic* actor1 = createStaticBox(pos, PxVec3(-(x+y+0.5), 0, 0), y, y, y, pose, BlockType::seesawbox);
	createStaticBox(pos, PxVec3(x+y+0.5, 0, 0), y, y, y, pose, BlockType::seesawbox);
	PxTransform localFrame0(PxVec3(0, 0, 0));
	PxTransform localFrame1(PxVec3(x+y+0.5, 0, 0));
	PxRevoluteJoint* revolute = PxRevoluteJointCreate(*gPhysics, actor0, localFrame0, actor1, localFrame1);
	//revolute->setLocalPose(PxJointActorIndex::Enum::eACTOR0, PxTransform(PxVec3(0, 0, 0), PxQuat(1.75 * PxHalfPi, PxVec3(1, 0, 0))));
	return revolute;
}


//创建跷跷板关卡 v为跷板0的中心点 sx、sy、sz为跷板的半长
float createSeesawLevel(const PxTransform& t, PxVec3 v,float sx, float sy, float sz, PxTransform& pose) {
	//第一块跷板0
	float x0 = v.x;
	float y0 = v.y;
	float z0 = v.z;
	createSeesaw(t, v, sx, sy, sz, pose);

	//第二块跷板1
	float x1 = x0 + 2*sx + dx;
	float z1 = z0 + sz + dz / 2;
	createSeesaw(t, PxVec3(x1, y0, z1), sx, sy, sz, pose);
	
	float x2 = x1;
	float z2 = z0 - sz - dz / 2;
	createSeesaw(t, PxVec3(x2, y0, z2), sx, sy, sz, pose);

	float x3 = x0 + 4 * sx + 2 * dx;
	float z3 = z0;
	createSeesaw(t, PxVec3(x3, y0, z3), sx, sy, sz, pose);

	float x4 = x3;
	float z4 = z0 + 2 * sz + dz;
	createSeesaw(t, PxVec3(x4, y0, z4), sx, sy, sz, pose);

	float x5 = x3;
	float z5 = z0 - 2 * sz - dz;
	createSeesaw(t, PxVec3(x5, y0, z5), sx, sy, sz, pose);

	float x6 = x1 + 4 * sx + 2 * dx;
	float z6 = z1;
	createSeesaw(t, PxVec3(x6, y0, z6), sx, sy, sz, pose);

	float x7 = x6;
	float z7 = z2;
	createSeesaw(t, PxVec3(x7, y0, z7), sx, sy, sz, pose);
	
	float x8 = x3 + 4 * sx + 2 * dx;
	float z8 = z0;
	createSeesaw(t, PxVec3(x8, y0, z8), sx, sy, sz, pose);

	return x8;
 }

/* creates particles in the PhysX SDK */
void createParticles(PxVec3 positions,PxVec3 velocities,PxF32 restOffsets, PxU32 maxParticles,PxU32 numParticles)
{
	// set immutable properties.
	PxU32 maxParticles = 2048;
	bool perParticleRestOffset = false;

	// create particle system in PhysX SDK
	PxParticleSystem* ps = gPhysics->createParticleSystem(maxParticles, perParticleRestOffset);

	// declare particle descriptor for creating new particles
// based on numNewAppParticles count and newAppParticleIndices,
// newAppParticlePositions arrays and newAppParticleVelocity
	PxParticleExt::IndexPool* indexPool = PxParticleExt::createIndexPool(maxParticles);
	

	PxParticleCreationData particleCreationData;
	particleCreationData.numParticles = 1024;
	PxU32 numAllocated = indexPool->allocateIndices(2048, PxStrideIterator<PxU32>(&particleCreationData.numParticles));
	particleCreationData.indexBuffer = PxStrideIterator<const PxU32>(&numAllocated);
	particleCreationData.positionBuffer = PxStrideIterator<const PxVec3>(newAppParticlePositions);

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
			}
		}

		// return ownership of the buffers back to the SDK
		rd->unlock();
	}


	// add particle system to scene, in case creation was successful
	if (ps)
		gScene->addActor(*ps);

}


void createGameScene(const PxTransform& t) {
	time_t startTime = time(NULL);
	PxTransform defaultPose(PxQuat(0, PxVec3(0, 1, 0)));  //刚体默认pose

	float r_1_l = 4.0;  //road_1_length 4 100
	float r_1_w = 15.0;  //road_1_width 8 100
	float c_1_y = boxHeight;  //the position of the center of road_1
	// create road_1
	createRoad(t, PxVec3(0, c_1_y, 0), r_1_l, boxHeight, r_1_w, defaultPose);

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
	//悬空路段
	float roadblock_length = 3.0;
	float roadblock_width = 4.0;
	float rb_x = center_x - 2 * stairsLength + dx + roadblock_length;
	float rb_y = centerHeight - 2 * boxHeight;
	float rb_z = center_z;
	for (int i = 0; i < 3; i++) {
		createRoad(t, PxVec3(rb_x, rb_y, rb_z), roadblock_length, boxHeight, roadblock_width, defaultPose);
		rb_x = rb_x + 2 * roadblock_length + dx;
	}

	//最后一块悬空路段的中心点（rb_x-2 * roadblock_length - dx, rb_y, rb_z）
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
	PxVec3 globalPos = r_3->getGlobalPose().p;
	std::cout <<"连接路段世界坐标:"<< globalPos.x << "," << globalPos.y << "," << globalPos.z << endl;

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



	//PxTransform pose1(PxQuat(-PxHalfPi/3, PxVec3(1, 0, 0)));
	////跷板 PxVec3 v的第二个参数应该为 底下所有盒子的高+ 最好大于z的数
	//createSeesaw(t, PxVec3(-30, 15 + 2 * boxHeight, -10), 5, 1, 15, pose1);
	//createFrontDoor(t, PxVec3(-20, 8.5 + 2 * boxHeight, 10), 0.8, defaultPose, true);
	//createSideDoor(t, PxVec3(-20 + 30 * 0.8, 8.5 + 2 * boxHeight, 10 + 7 * 0.8), 0.8, defaultPose, false);
	//createFrontDoor(t, PxVec3(-20, 8.5 + 2 * boxHeight, 10+37*0.8), 0.8, defaultPose, false);
	//createSideDoor(t, PxVec3(-20-7*0.8, 8.5 + 2 * boxHeight, 10+7*0.8), 0.8, defaultPose, true);

	createPlane(PxVec3(0, 0, 0), PxVec3(0, 1, 0));
}