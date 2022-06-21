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
#include "../Render/BMPLoader.h"
#include "../Sound/SoundTools.h"
#include<vector>
#include<string>
#include <glut.h>
#include<iostream>
#include<time.h>
#include<map>



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



Role* role = NULL;
PxControllerManager* cManager = NULL;


extern PxVec3 ScenetoWorld(int xCord, int yCord);

const PxTransform t = PxTransform(PxVec3(-50, 0, -250));
extern void createPorp(const PxTransform& t, const PxVec3& v, PxReal x, PxReal y, PxReal z);

//材质贴图ID数组
std::map<string, unsigned int> textureMap;
std::string texture[] = { "Door","Wall","Road","SeesawBox","Seesaw","Ice"};

//音频类
SoundTool soundtool = SoundTool();



//加载纹理
void loadTexture() {	
	for (auto name : texture) {
		string baseUrl = "../../texture/";
		CBMPLoader* BMPLoader = new CBMPLoader();
		unsigned int id = BMPLoader->generateID((baseUrl + name + ".bmp").c_str());
		textureMap.insert(std::pair<string, unsigned int>(name, id));
		std::cout << id << std::endl;
	}
	textureMap.insert(std::pair<string, unsigned int>("Block", 0));
}

//初始化游戏场景和人物
void initGame() {
	extern void createGameScene(const PxTransform & t);
	createGameScene(t);

	role = new Role();
		//初始位置
		//role->setFootPosition(PxVec3(-50, 20, -250));
		//摆锤前位置
		//role->setFootPosition(t.transform(PxVec3(58, 20, 19)));
		//迷宫前位置
		//role->setFootPosition(t.transform(PxVec3(183, 20, 64)));
		//迷宫出口位置
		//role->setFootPosition(t.transform(PxVec3(35, 20, 385.8)));
		//平移路段前位置
		role->setFootPosition(t.transform(PxVec3(3, 45, 416.8)));
		//旋转杆关卡角落位置
		//role->setFootPosition(t.transform(PxVec3(-17, 45, 406.8)));
		//role->attachModel("../../models/paimon/paimon.obj");
		role->attachModel("../../models/human.obj");
		role->fall();
}

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

	// 给PVD注册扩展
	PxInitExtensions(*gPhysics, gPvd);

	//?缩放
	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	//重力
	sceneDesc.gravity = PxVec3(0.0f, -29.4f, 0.0f);
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
	groundPlane->setName("Over");
	gScene->addActor(*groundPlane);

	//// 测试代码
	//Model testModel("../../models/paimon/paimon.obj");
	//testModel.attachMeshes((PxTransform(PxVec3(20, 30, 30))).transform(PxTransform(PxQuat(-PxHalfPi,PxVec3(1.0f,0.0f,0.0f)))));
	//testModel.createMeshActor(PxTransform(20, 30, 30));
	// end

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