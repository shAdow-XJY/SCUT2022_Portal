//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of NVIDIA CORPORATION nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Copyright (c) 2008-2018 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

// ****************************************************************************
// This snippet illustrates simple use of physx
//
// It creates a number of box stacks on a plane, and if rendering, allows the
// user to create new stacks and fire a ball from the camera position
// ****************************************************************************

#include <ctype.h>
#include "PxPhysicsAPI.h"

#include "../Common/Print.h"
#include "../Common/PVD.h"
#include "../Utils/Utils.h"
#include "../Role/Role.h"
#include "../LoadModel/Model.h"

#include<vector>
#include<string>
#include <glut.h>
#include<iostream>



using namespace physx;
//默认的内存管理和错误报告器
PxDefaultAllocator		gAllocator;
PxDefaultErrorCallback	gErrorCallback;

PxFoundation*			gFoundation = NULL;
PxPhysics*				gPhysics	= NULL;
PxCooking*				gCooking	= NULL;

PxDefaultCpuDispatcher*	gDispatcher = NULL;
PxScene*				gScene		= NULL;

PxMaterial*				gMaterial	= NULL;

PxPvd*                  gPvd        = NULL;

PxReal stackZ = 10.0f;


std::vector<PxRigidDynamic*>			ballToDispear;
std::vector<PxRigidDynamic*>			ballBirdList;
std::vector<PxRigidDynamic*>			ballPigList;
PxRigidDynamic*                         ballBird;
PxRigidDynamic*                         ballPig;


const char* BirdName = "bird";
const char* PigName = "pig";


Role* role = NULL;
PxControllerManager* cManager = NULL;

// 右键鼠标按下
bool press = false;


extern PxVec3 ScenetoWorld(int xCord, int yCord);

const PxTransform t = PxTransform(PxVec3(-100, 0, 0));
extern void createPorp(const PxTransform& t, const PxVec3& v, PxReal x, PxReal y, PxReal z);


// 鼠标点击时的坐标
int mouseX, mouseY;

// 提示字符的位置（测试用）
int textX = 0, textY = 0;


//创建立方体堆
void createStack(const PxTransform& t, PxU32 size, PxReal halfExtent)
{
	/*
	PxShape 碰撞形状的抽象类;形状是共享的，引用计数的对象。
	可以通过调用PxRigidActor类的createShape()方法或PxPhysics类的createShape()方法来创建实例。
	Box,Sphere,Capsule,Plane,ConvexMesh,TriangleMesh,HeightField
	可以设置Material,offset,flags,name
	*/
	//createShape构建形状;(halfExtent x,y,z)
	PxShape* shape = gPhysics->createShape(PxBoxGeometry(halfExtent, halfExtent, halfExtent), *gMaterial);
	


	for(PxU32 i=0; i<size;i++)
	{
		for(PxU32 j=0;j<size;j++)
		{
			
			PxTransform localTm(PxVec3(PxReal(j*2) - PxReal(size), PxReal(i*2+1), 0) * halfExtent);
	
			PxRigidDynamic* body = gPhysics->createRigidDynamic(t.transform(localTm));

			body->attachShape(*shape);

			PxRigidBodyExt::updateMassAndInertia(*body, 1.0f);

			body->setName(PigName);

			body->userData = body;

			
			ballPigList.push_back(body);

			gScene->addActor(*body);
		}
	}
	shape->release();
}

////创建道路方块
//void createRoad(const PxTransform& t,PxReal halfExtent,std::string name) {
//	Block* r = new Block(halfExtent,name);
//	PxShape* shape = gPhysics->createShape(PxBoxGeometry(halfExtent, halfExtent, halfExtent), *gMaterial);
//	shape->setQueryFilterData(collisionGroup);
//	PxRigidDynamic* body = gPhysics->createRigidDynamic(t);
//	body->attachShape(*shape);
//	//更新质量和惯性（数值表示密度）
//	PxRigidBodyExt::updateMassAndInertia(*body, 1.0f);
//	body->setName("Ground");
//	body->userData = r;
//	gScene->addActor(*body);
//}

///实例化物理
void initPhysics(bool interactive)
{
	//PxFoundation(版本号,内存回调,错误回调)
	gFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, gAllocator, gErrorCallback);
	//PVD
	gPvd = PxCreatePvd(*gFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);
	//创建顶级PxPhysics对象
	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, gPvd);

	// 创建Cooking对象
	gCooking = PxCreateCooking(PX_PHYSICS_VERSION, *gFoundation, PxCookingParams(PxTolerancesScale()));

	//?缩放
	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	//重力
	sceneDesc.gravity = PxVec3(0.0f, -9.8f, 0.0f);
	gDispatcher = PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher = gDispatcher;
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	gScene = gPhysics->createScene(sceneDesc);



	PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
	if (pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}
	cManager = PxCreateControllerManager(*gScene);
	cManager->setOverlapRecoveryModule(true);
	//静摩擦，动摩擦，restitution恢复原状(弹性)
	gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.0f);
	PxRigidStatic* groundPlane = PxCreatePlane(*gPhysics, PxPlane(0, 1, 0, 0), *gMaterial);

	groundPlane->setName("over");
	gScene->addActor(*groundPlane);

	// 测试代码
	Model testModel("../../models/paimon/paimon.obj");
	//testModel.createMeshActor((PxTransform(PxVec3(20, 30, 30))).transform(PxTransform(PxQuat(-PxHalfPi,PxVec3(1.0f,0.0f,0.0f)))));
	testModel.createMeshActor(PxTransform(20, 30, 30));
	// end

	extern void createGameScene(const PxTransform & t);
	createGameScene(t);
	

	role = new Role();
	role->setFootPosition(PxVec3(-100, 20, 0));

	role->fall();

}


/**
* @brief 发送射线
* @param origin 发送射线的坐标 unitDir 发送射线的方向 Road游戏道路基类
**/

PxRigidActor* RayCast(PxVec3 origin, PxVec3 unitDir)
{

	PxRaycastHit hitInfo; // 返回的点，法向量信息等都在这里
	PxReal maxDist = unitDir.normalize(); // 最大射线距离

	// 获取点， 法向量，还有UV信息
	PxHitFlags hitFlags = PxHitFlag::ePOSITION;


	//设置射线碰撞的组
	PxQueryFilterData filterdata = PxQueryFilterData();
	filterdata.data = collisionGroup;

	bool isRayHit = PxSceneQueryExt::raycastSingle(
		*gScene,
		origin,
		unitDir,
		maxDist,
		hitFlags,
		hitInfo,
		filterdata
	);

	if (isRayHit) {
		//std::cout << hitInfo.actor->getName() << std::endl;
		PxVec3 pose = hitInfo.actor->getGlobalPose().p;
		return hitInfo.actor;
		//std::cout << "hit position x:" << pose.x << " y:" << pose.y << " z:" << pose.z << std::endl;
	}
	else {
		return NULL;
	}
	//std::cout << "rayHit is not hit" << std::endl;
	return NULL;
}

/**
* @brief 角色底部发送射线
* @desc  用于角色模拟重力和给物体施加重力
**/
void RayCastByRole() {
	PxVec3 origin = role->getFootPosition();
	PxVec3 unitDir = PxVec3(0, -1.0f, 0);
	PxRigidActor* actor = NULL;
	if (actor = RayCast(origin, unitDir)) {
		//碰撞到物体
		//std::cout << "碰到地面" << std::endl;
		//cout << role->standingBlock.getName() << endl;
		Block* block = (Block*) actor->userData ;
		if (block != NULL) {
			if (block->getType() == BlockType::road) {
				//std::cout << role->standingBlock.getName()<<std::endl;

			}
			else if (block->getType() == BlockType::seesaw) {
				//cout << "施加重力" << endl;
				Seesaw* seesaw = (Seesaw*)block;
				PxRigidBody* seesawBody = seesaw->getSeesawActor();
				PxVec3 force = PxVec3(0, -1, 0) * 1000.0f;
				PxRigidBodyExt::addForceAtPos(*seesawBody, force, role->getFootPosition());
				//seesawBody->addForce()
			}
			role->standingBlock = *block;
		}
	}
	else {
		if (role->standingBlock.getType() != BlockType::error) {
			role->setFootPosition(role->getFootPosition() + role->getSpeed() * 5.0f);
		}
		role->standingBlock = Block();
		//std::cout << "未碰到地面" << std::endl;	
		//role->gameOver();
		role->fall();
	}
}

/**
* @brief 角色道具拾取
**/
void PickPropByRole() {
	PxVec3 origin = role->getPosition();
	//确定role的前方方向
	PxVec3 forwardDir = role->getFaceDir() * 2;
	PxRigidActor* actor = NULL;
	if (actor = RayCast(origin, forwardDir)) {
		Block* block = (Block*)actor->userData;
		if (block->getType() == BlockType::prop) {
			actor->release();
			role->setEquiped(true);
			std::cout << "拾取道具成功" << std::endl;
		}
		else
		{
			std::cout << "不是道具" << std::endl;
		}
	}
	else
	{
		std::cout << "射线没有找到目标" << std::endl;
	}
}

/**
* @brief 角色道具放置
**/
void LayPropByRole() {
	PxVec3 origin = role->getPosition();
	//确定role的前方方向
	PxVec3 forwardDir = PxVec3(role->getFaceDir().x, -3, role->getFaceDir().z) ;
	PxRigidActor* actor = NULL;
	if (actor = RayCast(origin, forwardDir)) {
		Block* block = (Block*)actor->userData;
		cout << block->getType() << endl;
		if (block->getType() == BlockType::road) {
			role->setEquiped(false);
			//cout << role->getPosition().x << " " << role->getPosition().y << " " << role->getPosition().z << endl;
			//cout << role->getFaceDir().x << " " << role->getFaceDir().y << " " << role->getFaceDir().z << endl;
			createPorp(PxTransform(PxVec3(0, 0, 0)), role->getPosition() + role->getFaceDir()*2.5, boxHeight, boxHeight, boxHeight);
			std::cout << "放置道具成功" << std::endl;
		}
		else
		{
			std::cout << "不是可放置道具的地方" << std::endl;
		}
	}
	else
	{
		std::cout << "射线没有找到目标" << std::endl;
	}
}

//（在render中调用）
void stepPhysics(bool interactive)
{
	PX_UNUSED(interactive);
	//时间
	gScene->simulate(1.0f / 60.0f);
	gScene->fetchResults(true);
}

//清空物理（在render中调用）
void cleanupPhysics(bool interactive)
{
	//release()销毁对象以及包含的所有对象
	PX_UNUSED(interactive);
	gScene->release();
	gDispatcher->release();
	gPhysics->release();
	PxPvdTransport* transport = gPvd->getTransport();
	gPvd->release();
	transport->release();

	gFoundation->release();

	printf("HelloWorld done.\n");
}

//按键设置
void keyPress(unsigned char key, const PxTransform& camera)
{
	switch (toupper(key))
	{
	case 'B':	createStack(PxTransform(PxVec3(-100, 0, 0)).transform(PxTransform(PxVec3(-30, 10 + 2 * boxHeight, 0))), 10, 2.0f); break;
	//case 'B':	createStack(PxTransform(PxVec3(0, 0, stackZ -= 10.0f)), 10, 2.0f); break;
		//PxSphereGeometry Transform,geometry,velocity（速度）t.transform(PxTransform(PxVec3(-20, 10+2*boxHeight, 0)))
	case ' ':
	{
		role->tryJump(false);
		break;
	}
	case 'Z':
	{
		role->roleCrouch();
		break;
	}
	case 'E':
	{
		if (role->getEquiped()) {
			LayPropByRole();
		}
		else {
			PickPropByRole();
		}

		break;
	}
	default:
		break;
	}
}

void keyRelease(unsigned char key)
{
	switch (toupper(key))
	{
	case ' ':
	{
		role->tryJump(true);
		break;
	}
	case 'Z':
	{
		role->roleNoCrouch();
		break;
	}
	default:
		break;
	}
}

//特殊键设置
void specialKeyPress(GLint key) {
	switch (key) {
	default: {
		return;
	}
	}
}

void specialKeyRelease(GLint key) {

}

//鼠标点击
void mousePress(int button, int state, int x, int y) {
	switch (button)
	{
		//点击右键
	case 0: {
		//右键抬起
		if (state == 1) {
			//if (role->getMovingStatus())return;
			////role->roleMoveByMouse(x, y);
			//PxVec3 position = ScenetoWorld(x, y);
			//Block road;
			//if (RayCast(position, PxVec3(0, 5, 0), road))
			//{
			//	PxVec3 blockPosition = road.getPosition();
			//	//role->roleMoveByMouse(PxVec3(blockPosition.x, role->getFootPosition().y, blockPosition.z));
			//	role->roleMoveByMouse(position);
			//}
			//else {
			//	std::cout << "不可点击";
			//}
		}
		break;
	}
	case 2: {
		if (state == 1) {
			press = true;
			mouseX = x;
			mouseY = y;
			std::cout << "mouseX: " << x << std::endl;
			std::cout << "mouseY: " << y << std::endl;
		}
		break;
	}
	default:
		break;
	}
}

#define RENDER_SNIPPET 1
//main
int snippetMain(int, const char* const*)
{
#ifdef RENDER_SNIPPET
	extern void renderLoop();
	renderLoop();
#else
	static const PxU32 frameCount = 100;
	initPhysics(false);
	for (PxU32 i = 0; i < frameCount; i++)
		stepPhysics(false);
	cleanupPhysics(false);
#endif

	return 0;
}