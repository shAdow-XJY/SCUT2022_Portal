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
float dx = 6.0;  //x_distance x�᷽���Ͽ���Ծ�ļ������
float dz = 6.0;  //z_distance z�᷽���Ͽ���Ծ�ļ������


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
	//��ײ���Ĺ�����
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


//���������࣬����ֻ��Block(BlockType::prop)�������ã����޸�
void createPorp(const PxTransform& t, const PxVec3& v, PxReal x, PxReal y, PxReal z) {
	PxTransform local(v);
	//cout << v.x << " " << v.y << " " << v.z << endl;
	PxShape* shape = gPhysics->createShape(PxBoxGeometry(x, y, z), *gMaterial);
	//��ײ���Ĺ�����
	shape->setQueryFilterData(collisionGroup);

	//setupFiltering(shape, FilterGroup::ePIG, FilterGroup::eBIRD);
	PxRigidDynamic* sceneBox = gPhysics->createRigidDynamic(t.transform(local));
	PxRigidBodyExt::updateMassAndInertia(*sceneBox, 1.0f);
	Block* block = new Block("����", sceneBox->getGlobalPose().p, x, y, z);
	block->setType(BlockType::prop);
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


/**
* @brief ������·
**/
PxRigidStatic* createRoad(const PxTransform& t, const PxVec3& v, PxReal x, PxReal y, PxReal z, PxTransform& pose) {
	PxRigidStatic* roadActor = createStaticBox(t, v, x, y, z, pose, BlockType::road);
	//std::cout << "v::" <<  v.x << " " << v.y << " " << v.z << endl;
	PxVec3 position = roadActor->getGlobalPose().p;
	Road* road = new Road("·��",position, x, y, z,roadActor);
	roadActor->userData = road;
	return roadActor;
}

float center_y(float y) {
	return y + 2 * boxHeight;
}


//tΪ����������ԭ�� vΪ�������ĵ���Գ���ԭ���λ�� scaleΪ�ŵ�����ϵ��
//�ڶ�������Ӧ��ΪPxVec3 v(v_x, �������к��ӵĸ�+10*scale��v_z)
//�������е�
void createFrontDoor(const PxTransform& t, PxVec3 v, float scale, PxTransform& pose,bool canOpen = true) {
	PxTransform pos(t.transform(PxTransform(v)));//-17.8  -18.56 -18.561815

	PxReal x = 10 * scale;
	PxReal y = 5 * scale;
	PxReal z = 1 * scale;
	//ʵ��-5  ��ת�Ƕ�С��-17.8  -18.56 -18.561815
	PxRigidStatic* actor1 = createStaticBox(pos, PxVec3(0, 0, 0), 6 * scale, 10 * scale, 1 * scale, pose, BlockType::wall);
	createStaticBox(pos, PxVec3(23 * scale, 0, 0), 6 * scale, 10 * scale, 1 * scale, pose, BlockType::wall);
	if (canOpen) {
		PxRigidDynamic* actor0 = createDynamicBox(false, pos, PxVec3(6.5 * scale, -5 * scale, 0), x, y, z, pose, BlockType::door);
		PxTransform localFrame0(PxVec3(0, 5 * scale, 0));
		PxTransform localFrame1(PxVec3(6.5 * scale, 0, 0));
		PxRevoluteJoint* revolute = PxRevoluteJointCreate(*gPhysics, actor0, localFrame0, actor1, localFrame1);
		Door* door = new Door("����", actor0->getGlobalPose().p, x, y, z, actor0, canOpen, revolute);
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
		Door* door = new Door("����", actor0->getGlobalPose().p, x, y, z, false);
		actor0->userData = door;
	}
}


// joint���е�
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
		PxRigidStatic* actor0 =  createStaticBox(pos, PxVec3(0, 0, 11.5 * scale), y, x, z, pose, BlockType::door);
		Door* door = new Door("����", actor0->getGlobalPose().p, x, y, z, false);
		actor0->userData = door;
	}
}

//�����Թ��ؿ�
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

//tΪ����������ԭ�� vΪ���ΰ�������Գ���ԭ���λ�� x��y��zΪ����ĳ��߿�
//�ΰ� PxVec3 v�ĵڶ�������Ӧ��Ϊ �������к��ӵĸ�+ y
PxRevoluteJoint* createSeesaw(const PxTransform& t,PxVec3 v,float x, float y, float z, PxTransform& pose) {
	PxTransform pos(t.transform(PxTransform(v)));
	PxRigidDynamic* actor0 = createDynamicBox(false, pos, PxVec3(0, 0, 0), x, y, z, pose, BlockType::seesaw);
	Seesaw* seesaw = new Seesaw("�̰�",actor0->getGlobalPose().p,x,y,z,actor0);
	actor0->userData = seesaw;
	PxRigidStatic* actor1 = createStaticBox(pos, PxVec3(-(x+y+0.5), 0, 0), y, y, y, pose, BlockType::seesawbox);
	createStaticBox(pos, PxVec3(x+y+0.5, 0, 0), y, y, y, pose, BlockType::seesawbox);
	PxTransform localFrame0(PxVec3(0, 0, 0));
	PxTransform localFrame1(PxVec3(x+y+0.5, 0, 0));
	PxRevoluteJoint* revolute = PxRevoluteJointCreate(*gPhysics, actor0, localFrame0, actor1, localFrame1);
	//revolute->setLocalPose(PxJointActorIndex::Enum::eACTOR0, PxTransform(PxVec3(0, 0, 0), PxQuat(1.75 * PxHalfPi, PxVec3(1, 0, 0))));
	return revolute;
}


//�������ΰ�ؿ� vΪ�ΰ�0�����ĵ� sx��sy��szΪ�ΰ�İ볤
float createSeesawLevel(const PxTransform& t, PxVec3 v,float sx, float sy, float sz, PxTransform& pose) {
	//��һ���ΰ�0
	float x0 = v.x;
	float y0 = v.y;
	float z0 = v.z;
	createSeesaw(t, v, sx, sy, sz, pose);

	//�ڶ����ΰ�1
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
				std::cout <<"���ӵ�λ�ã�"<< position.x << std::endl;
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
	PxTransform defaultPose(PxQuat(0, PxVec3(0, 1, 0)));  //����Ĭ��pose

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
	//����·��
	float roadblock_length = 3.0;
	float roadblock_width = 4.0;
	float rb_x = center_x - 2 * stairsLength + dx + roadblock_length;
	float rb_y = centerHeight - 2 * boxHeight;
	float rb_z = center_z;
	for (int i = 0; i < 3; i++) {
		createRoad(t, PxVec3(rb_x, rb_y, rb_z), roadblock_length, boxHeight, roadblock_width, defaultPose);
		rb_x = rb_x + 2 * roadblock_length + dx;
	}

	//���һ������·�ε����ĵ㣨rb_x-2 * roadblock_length - dx, rb_y, rb_z��
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
	PxVec3 globalPos = r_3->getGlobalPose().p;
	std::cout <<"����·����������:"<< globalPos.x << "," << globalPos.y << "," << globalPos.z << endl;

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



	//PxTransform pose1(PxQuat(-PxHalfPi/3, PxVec3(1, 0, 0)));
	////�ΰ� PxVec3 v�ĵڶ�������Ӧ��Ϊ �������к��ӵĸ�+ ��ô���z����
	//createSeesaw(t, PxVec3(-30, 15 + 2 * boxHeight, -10), 5, 1, 15, pose1);
	//createFrontDoor(t, PxVec3(-20, 8.5 + 2 * boxHeight, 10), 0.8, defaultPose, true);
	//createSideDoor(t, PxVec3(-20 + 30 * 0.8, 8.5 + 2 * boxHeight, 10 + 7 * 0.8), 0.8, defaultPose, false);
	//createFrontDoor(t, PxVec3(-20, 8.5 + 2 * boxHeight, 10+37*0.8), 0.8, defaultPose, false);
	//createSideDoor(t, PxVec3(-20-7*0.8, 8.5 + 2 * boxHeight, 10+7*0.8), 0.8, defaultPose, true);

	createPlane(PxVec3(0, 0, 0), PxVec3(0, 1, 0));
}