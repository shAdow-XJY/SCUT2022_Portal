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
//Ĭ�ϵ��ڴ����ʹ��󱨸���
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

const PxTransform t = PxTransform(PxVec3(-100, 0, -200));
extern void createPorp(const PxTransform& t, const PxVec3& v, PxReal x, PxReal y, PxReal z);


std::map<string, unsigned int> textureMap;
std::string texture[] = { "Door","Wall","Road","SeesawBox","Seesaw","Ice"};

//��Ƶ��
SoundTool soundtool = SoundTool();

//��������
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

//��ʼ����Ϸ����������
void initGame() {
	extern void createGameScene(const PxTransform & t);
	createGameScene(t);

	role = new Role();
	//��ʼλ��
	role->setFootPosition(PxVec3(-100, 20, -210));
	//�Թ�ǰλ��
	//role->setFootPosition(PxVec3(83, 20, -136));

	//role->attachModel("../../models/paimon/paimon.obj");
	role->attachModel("../../models/human.obj");
	role->fall();
}

///ʵ��������
void initPhysics(bool interactive)
{
	//PxFoundation(�汾��,�ڴ�ص�,����ص�)
	gFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, gAllocator, gErrorCallback);
	//PVD
	gPvd = PxCreatePvd(*gFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);
	//��������PxPhysics����
	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, gPvd);

	// ����Cooking����
	gCooking = PxCreateCooking(PX_PHYSICS_VERSION, *gFoundation, PxCookingParams(PxTolerancesScale()));

	// ��PVDע����չ
	PxInitExtensions(*gPhysics, gPvd);

	//?����
	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	//����
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
	//��Ħ������Ħ����restitution�ָ�ԭ״(����)
	gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.0f);
	PxRigidStatic* groundPlane = PxCreatePlane(*gPhysics, PxPlane(0, 1, 0, 0), *gMaterial);

	groundPlane->setName("Over");
	gScene->addActor(*groundPlane);

	//// ���Դ���
	//Model testModel("../../models/paimon/paimon.obj");
	//testModel.attachMeshes((PxTransform(PxVec3(20, 30, 30))).transform(PxTransform(PxQuat(-PxHalfPi,PxVec3(1.0f,0.0f,0.0f)))));
	//testModel.createMeshActor(PxTransform(20, 30, 30));
	// end

}


/**
* @brief ��������
* @param origin �������ߵ����� unitDir �������ߵķ��� Road��Ϸ��·����
**/

PxRigidActor* RayCast(PxVec3 origin, PxVec3 unitDir)
{

	PxRaycastHit hitInfo; // ���صĵ㣬��������Ϣ�ȶ�������
	PxReal maxDist = unitDir.normalize(); // ������߾���

	// ��ȡ�㣬 ������������UV��Ϣ
	PxHitFlags hitFlags = PxHitFlag::ePOSITION;


	//����������ײ����
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





//����render�е��ã�
void stepPhysics(bool interactive)
{
	PX_UNUSED(interactive);
	//ʱ��
	gScene->simulate(1.0f / 60.0f);
	gScene->fetchResults(true);
}

//���������render�е��ã�
void cleanupPhysics(bool interactive)
{
	//release()���ٶ����Լ����������ж���
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