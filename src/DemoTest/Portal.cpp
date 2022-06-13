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
#include<vector>
#include<string>
#include <glut.h>
#include<iostream>
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


std::vector<PxRigidDynamic*>			ballToDispear;
std::vector<PxRigidDynamic*>			ballBirdList;
std::vector<PxRigidDynamic*>			ballPigList;
PxRigidDynamic*                         ballBird;
PxRigidDynamic*                         ballPig;


const char* BirdName = "bird";
const char* PigName = "pig";


Role* role = NULL;
PxControllerManager* cManager = NULL;


// �Ҽ���갴��
bool press = false;


extern PxVec3 ScenetoWorld(int xCord, int yCord);

const PxTransform t = PxTransform(PxVec3(-100, 0, 0));


// �����ʱ������
int mouseX, mouseY;

// ��ʾ�ַ���λ�ã������ã�
int textX = 0, textY = 0;

std::map<string, unsigned int> textureMap;

//�����������
void createStack(const PxTransform& t, PxU32 size, PxReal halfExtent)
{
	/*
	PxShape ��ײ��״�ĳ�����;��״�ǹ���ģ����ü����Ķ���
	����ͨ������PxRigidActor���createShape()������PxPhysics���createShape()����������ʵ����
	Box,Sphere,Capsule,Plane,ConvexMesh,TriangleMesh,HeightField
	��������Material,offset,flags,name
	*/
	//createShape������״;(halfExtent x,y,z)
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

////������·����
//void createRoad(const PxTransform& t,PxReal halfExtent,std::string name) {
//	Block* r = new Block(halfExtent,name);
//	PxShape* shape = gPhysics->createShape(PxBoxGeometry(halfExtent, halfExtent, halfExtent), *gMaterial);
//	shape->setQueryFilterData(collisionGroup);
//	PxRigidDynamic* body = gPhysics->createRigidDynamic(t);
//	body->attachShape(*shape);
//	//���������͹��ԣ���ֵ��ʾ�ܶȣ�
//	PxRigidBodyExt::updateMassAndInertia(*body, 1.0f);
//	body->setName("Ground");
//	body->userData = r;
//	gScene->addActor(*body);
//}

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

	// ���Դ���
	Model testModel("../../models/paimon/paimon.obj");
	//testModel.createMeshActor((PxTransform(PxVec3(20, 30, 30))).transform(PxTransform(PxQuat(-PxHalfPi,PxVec3(1.0f,0.0f,0.0f)))));
	testModel.createMeshActor(PxTransform(20, 30, 30));
	// end
	std::string textture[] = { "door","wall","road" };
	for (auto name : textture) {
		CBMPLoader* BMPLoader = new CBMPLoader();
		unsigned int id = BMPLoader->generateID((name + ".bmp").c_str());
		textureMap.insert(std::pair<string, unsigned int>(name, id));
		std::cout << id << std::endl;
	}

	extern void createGameScene(const PxTransform & t);
	createGameScene(t);

	role = new Role();
	role->setFootPosition(PxVec3(-100, 20, 0));

	role->fall();

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

	printf("HelloWorld done.\n");
}

//��������
void keyPress(unsigned char key, const PxTransform& camera)
{
	switch (toupper(key))
	{
	case 'B':	createStack(PxTransform(PxVec3(-100, 0, 0)).transform(PxTransform(PxVec3(-30, 10 + 2 * boxHeight, 0))), 10, 2.0f); break;
	//case 'B':	createStack(PxTransform(PxVec3(0, 0, stackZ -= 10.0f)), 10, 2.0f); break;
		//PxSphereGeometry Transform,geometry,velocity���ٶȣ�t.transform(PxTransform(PxVec3(-20, 10+2*boxHeight, 0)))
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
			role->layDownObj();

		}
		else {
			role->pickUpObj();

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

//���������
void specialKeyPress(GLint key) {
	switch (key) {
	default: {
		return;
	}
	}
}

void specialKeyRelease(GLint key) {

}

//�����
void mousePress(int button, int state, int x, int y) {
	switch (button)
	{
		//����Ҽ�
	case 0: {
		//�Ҽ�̧��
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
			//	std::cout << "���ɵ��";
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