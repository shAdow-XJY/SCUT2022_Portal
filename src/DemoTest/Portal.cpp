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

// �����갴��
bool press = false;
//���ڹ��˵�����
const PxFilterData collisionGroupIgnore(0, 0, 0, 1);  // ��ײ�ᱻ���Ե���
const PxFilterData collisionGroup(1, 0, 0, 0); // ��Ҫ��ײ����



extern PxVec3 ScenetoWorld(int xCord, int yCord);



// �����ʱ������
int mouseX, mouseY;

// ��ʾ�ַ���λ�ã������ã�
int textX = 0, textY = 0;


struct FilterGroup
{
	enum Enum
	{
		eBIRD = (1 << 0),
		ePIG = (1 << 1),
		eMINE_LINK = (1 << 2),
		eCRAB = (1 << 3),
		eHEIGHTFIELD = (1 << 4),
	};
};


void setupFiltering(PxShape* shape, PxU32 filterGroup, PxU32 filterMask)
{
	PxFilterData filterData;
	filterData.word0 = filterGroup; // word0 = own ID
	filterData.word1 = filterMask;	// word1 = ID mask to filter pairs that trigger a contact callback;
	shape->setSimulationFilterData(filterData);
}


PxFilterFlags contactReportFilterShader(PxFilterObjectAttributes attributes0, PxFilterData filterData0,
	PxFilterObjectAttributes attributes1, PxFilterData filterData1,
	PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
{
	PX_UNUSED(attributes0);
	PX_UNUSED(attributes1);
	PX_UNUSED(filterData0);
	PX_UNUSED(filterData1);
	PX_UNUSED(constantBlockSize);
	PX_UNUSED(constantBlock);

	//
	// Enable CCD for the pair, request contact reports for initial and CCD contacts.
	// Additionally, provide information per contact point and provide the actor
	// pose at the time of contact.
	//

	pairFlags = PxPairFlag::eCONTACT_DEFAULT
		| PxPairFlag::eDETECT_CCD_CONTACT
		| PxPairFlag::eNOTIFY_TOUCH_CCD
		| PxPairFlag::eNOTIFY_TOUCH_FOUND
		| PxPairFlag::eNOTIFY_CONTACT_POINTS
		| PxPairFlag::eCONTACT_EVENT_POSE;
	return PxFilterFlag::eDEFAULT;
}



class ContactReportCallback : public PxSimulationEventCallback
{
	void onConstraintBreak(PxConstraintInfo* constraints, PxU32 count) { PX_UNUSED(constraints); PX_UNUSED(count); }
	void onWake(PxActor** actors, PxU32 count) { PX_UNUSED(actors); PX_UNUSED(count); }
	void onSleep(PxActor** actors, PxU32 count) { PX_UNUSED(actors); PX_UNUSED(count); }
	void onTrigger(PxTriggerPair* pairs, PxU32 count) { PX_UNUSED(pairs); PX_UNUSED(count); }
	void onAdvance(const PxRigidBody*const*, const PxTransform*, const PxU32) {}
	void onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs)
	{
		//std::vector<PxContactPairPoint> contactPoints;

		//PxTransform spherePose(PxIdentity);
		//PxU32 nextPairIndex = 0xffffffff;

		//PxContactPairExtraDataIterator iter(pairHeader.extraDataStream, pairHeader.extraDataStreamSize);
		//bool hasItemSet = iter.nextItemSet();
		//if (hasItemSet)
		//	nextPairIndex = iter.contactPairIndex;

		//for (PxU32 i = 0; i < nbPairs; i++)
		//{
		//	//
		//	// Get the pose of the dynamic object at time of impact.
		//	//
		//	if (nextPairIndex == i)
		//	{
		//		if (pairHeader.actors[0]->is<PxRigidDynamic>())
		//			spherePose = iter.eventPose->globalPose[0];
		//		else
		//			spherePose = iter.eventPose->globalPose[1];

		//		gContactSphereActorPositions.push_back(spherePose.p);

		//		hasItemSet = iter.nextItemSet();
		//		if (hasItemSet)
		//			nextPairIndex = iter.contactPairIndex;
		//	}

		//	//
		//	// Get the contact points for the pair.
		//	//
		//	const PxContactPair& cPair = pairs[i];
		//	if (cPair.events & (PxPairFlag::eNOTIFY_TOUCH_FOUND | PxPairFlag::eNOTIFY_TOUCH_CCD))
		//	{
		//		PxU32 contactCount = cPair.contactCount;
		//		contactPoints.resize(contactCount);
		//		cPair.extractContacts(&contactPoints[0], contactCount);

		//		for (PxU32 j = 0; j < contactCount; j++)
		//		{
		//			gContactPositions.push_back(contactPoints[j].position);
		//			gContactImpulses.push_back(contactPoints[j].impulse);
		//		}
		//	}
		//}

		//PxActorFlag
		for (PxU32 i = 0; i < nbPairs; i++)
		{
			const PxContactPair& cp = pairs[i];

			if (cp.events & PxPairFlag::eNOTIFY_TOUCH_FOUND)
			{
				if ((pairHeader.actors[0]->getName() == BirdName && pairHeader.actors[1]->getName() == PigName) || (pairHeader.actors[1]->getName() == BirdName && pairHeader.actors[0]->getName() == PigName))
				{
					PxActor* otherActor = (pairHeader.actors[0]->getName() == BirdName) ? pairHeader.actors[1] : pairHeader.actors[0];
					PxRigidDynamic* ballPig1 = (PxRigidDynamic*)(otherActor->userData);


					// insert only once
					if (std::find(ballToDispear.begin(), ballToDispear.end(), ballPig1) == ballToDispear.end())
						ballToDispear.push_back(ballPig1);

					std::vector<PxRigidDynamic*>::iterator ballIter = std::find(ballPigList.begin(), ballPigList.end(), ballPig1);
					if (ballIter != ballPigList.end())
					{
						//ballPigList.erase(ballIter);
						//gScene->removeActor(*ballPig1);
					}



					break;
				}
			}
		}
	}
};

ContactReportCallback gContactReportCallback;



/*
PxBase->PxActor->PxRigidActor->PxRigidBody->PxRigidDynamic
*/
//���춯̬����
PxRigidDynamic* createDynamic( PxReal radius, const PxTransform& t, const PxVec3& velocity=PxVec3(0))
{


	PxShape* shape = gPhysics->createShape(PxSphereGeometry(radius), *gMaterial);
	setupFiltering(shape, FilterGroup::eBIRD, FilterGroup::ePIG);


	//PxPhysics object��transform of the new object ��shape of the new object ��the density of the new object(>0)
	PxRigidDynamic* dynamic = gPhysics->createRigidDynamic(t);
	dynamic->attachShape(*shape);

	
	

	//���ý�����ϵ����������������linearDamping����������������������Լ���ֿ�ƽ�Ƶ���,�������������ֿ���ת�������������Ϊ0�������һֱ��ת/ƽ��
	dynamic->setAngularDamping(10.0f);


	//���������ٶ� 
	dynamic->setLinearVelocity(velocity);

	PxRigidBodyExt::updateMassAndInertia(*dynamic, 1.0f);
	//���������
	ballBirdList.push_back(dynamic);

	dynamic->setName(BirdName);

	gScene->addActor(*dynamic);
	return dynamic;
}

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
	
	setupFiltering(shape, FilterGroup::ePIG, FilterGroup::eBIRD);


	for(PxU32 i=0; i<size;i++)
	{
		for(PxU32 j=0;j<size;j++)
		{
			//ָ��λ��(-10/-7..9,1,0)(-7..,3,0)(-4..,5,0)...
			PxTransform localTm(PxVec3(PxReal(j*2) - PxReal(size), PxReal(i*2+1), 0) * halfExtent);
			//createRigidDynamic��������
			PxRigidDynamic* body = gPhysics->createRigidDynamic(t.transform(localTm));
			//attachShape����״��������;
			body->attachShape(*shape);
			//���������͹��ԣ���ֵ��ʾ�ܶȣ�
			PxRigidBodyExt::updateMassAndInertia(*body, 1.0f);

			body->setName(PigName);

			body->userData = body;

			
			//���������
			ballPigList.push_back(body);


			//��Actor��ӵ�������,ע�͵���һ��֮�������������ò��ɼ���û����ײ���
			gScene->addActor(*body);
			//gScene->removeActor(*body);

		}
	}
	//�ͷ�
	shape->release();
}

//������·����
void createRoad(const PxTransform& t,PxReal halfExtent,std::string name) {
	Block* r = new Block(halfExtent,name);
	PxShape* shape = gPhysics->createShape(PxBoxGeometry(halfExtent, halfExtent, halfExtent), *gMaterial);
	shape->setQueryFilterData(collisionGroup);
	PxRigidDynamic* body = gPhysics->createRigidDynamic(t);
	body->attachShape(*shape);
	//���������͹��ԣ���ֵ��ʾ�ܶȣ�
	PxRigidBodyExt::updateMassAndInertia(*body, 1.0f);
	body->setName("Ground");
	body->userData = r;
	//��Actor��ӵ�������,ע�͵���һ��֮�������������ò��ɼ���û����ײ���
	gScene->addActor(*body);
}

//ʵ��������
void initPhysics(bool interactive)
{
	//PxFoundation(�汾��,�ڴ�ص�,����ص�)
	gFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, gAllocator, gErrorCallback);
	//PVD
	gPvd = PxCreatePvd(*gFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	gPvd->connect(*transport,PxPvdInstrumentationFlag::eALL);
	//��������PxPhysics����
	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(),true,gPvd);

	// ����Cooking����
	gCooking = PxCreateCooking(PX_PHYSICS_VERSION, *gFoundation, PxCookingParams(PxTolerancesScale()));

	//?����
	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	//����
	sceneDesc.gravity = PxVec3(0.0f, -9.8f, 0.0f);
	gDispatcher = PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher	= gDispatcher;
	sceneDesc.filterShader	= PxDefaultSimulationFilterShader;
	sceneDesc.filterShader = contactReportFilterShader;
	sceneDesc.simulationEventCallback = &gContactReportCallback;
	gScene = gPhysics->createScene(sceneDesc);



	PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
	if(pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}
	cManager = PxCreateControllerManager(*gScene);
	cManager->setOverlapRecoveryModule(true);
	//��Ħ������Ħ����restitution�ָ�ԭ״(����)
	gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.0f);
	PxRigidStatic* groundPlane = PxCreatePlane(*gPhysics, PxPlane(0,1,0,0), *gMaterial);
	
	groundPlane->setName("over");
	gScene->addActor(*groundPlane);

	// ���Դ���
	Model testModel("D:/sx/PhysX_3.4/SCUT2022_Portal/src/DemoTest/Mouse Server.obj");
	testModel.createMeshActor(PxVec3(30, 30, 30));
	// end

	createRoad(PxTransform(PxVec3(0, 5, -5)), 5, "0");
	createRoad(PxTransform(PxVec3(0, 5, 20)),5,"1");
	createRoad(PxTransform(PxVec3(0, 5, 30)), 5 ,"2");
	createRoad(PxTransform(PxVec3(0, 5, 40)), 5, "3");
	createRoad(PxTransform(PxVec3(10, 5, 40)), 5, "4");
	createRoad(PxTransform(PxVec3(20, 5, 40)), 5, "5");
	createRoad(PxTransform(PxVec3(20, 5, 30)), 5, "6");
	createRoad(PxTransform(PxVec3(20, 15, 30)), 5, "6");
	role = new Role();

	role->fall();
	//if (������)����render�аѽ������false����һ��Ĭ�ϵ������ȥײ���ѡ�
	if(!interactive)
		//PxSphereGeometry Transform,geometry,velocity���ٶȣ�
		createDynamic(10,PxTransform(PxVec3(0,40,100)), PxVec3(0,-50,-100));
}

/**
* @brief ��������
* @param origin �������ߵ����� unitDir �������ߵķ��� Road��Ϸ��·����
**/
bool RayCast(PxVec3 origin, PxVec3 unitDir, Block& block)
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
		block.setPosition(PxVec3(pose.x, pose.y, pose.z));
		Block* b = (Block*)hitInfo.actor->userData;
		block.setName(b->getName());
		block.setBlockType(b->getBlockType());
		//std::cout << "hit position x:" << pose.x << " y:" << pose.y << " z:" << pose.z << std::endl;
	}
	else {
	}
		//std::cout << "rayHit is not hit" << std::endl;
	return isRayHit;
}

/**
* @brief ��ɫ�ײ���������
**/
void RayCastByRole() {
	PxVec3 origin = role->getFootPosition();
	PxVec3 unitDir = PxVec3(0, -999, 0);
	if (RayCast(origin, unitDir, role->standingBlock)) {
		//��ײ������
		//std::cout << "��������" << std::endl;
		if (role->standingBlock.getBlockType() == BlockType::ground) {
			//std::cout << role->standingBlock.getName()<<std::endl;
		}
	}
	else {
		if (role->standingBlock.getBlockType() != BlockType::error) {
			role->setFootPosition(role->getFootPosition() + role->getSpeed()*5.0f);
		}
		role->standingBlock = Block();
		//std::cout << "δ��������" << std::endl;	
		//role->gameOver();
		role->fall();
	}
}

//����render�е��ã�
void stepPhysics(bool interactive)
{
	PX_UNUSED(interactive);
	//ʱ��
	gScene->simulate(1.0f/60.0f);
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
	switch(toupper(key))
	{
	case 'B':	createStack(PxTransform(PxVec3(0,0,stackZ-=10.0f)), 10, 2.0f);break;
	//PxSphereGeometry Transform,geometry,velocity���ٶȣ�
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
			if (role->getMovingStatus())return;
			//role->roleMoveByMouse(x, y);
			PxVec3 position = ScenetoWorld(x, y);
			Block road;
			if (RayCast(position,PxVec3(0,5,0), road))
			{			
				PxVec3 blockPosition = road.getPosition();
				role->roleMoveByMouse(PxVec3(blockPosition.x,role->getFootPosition().y, blockPosition.z));
				//role->setFootPosition(PxVec3(road.position.x,role->getFootPosition().y,road.position.z));
			}
			else {
				std::cout << "���ɵ��";
			}
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
int snippetMain(int, const char*const*)
{
#ifdef RENDER_SNIPPET
	extern void renderLoop();
	renderLoop();
#else
	static const PxU32 frameCount = 100;
	initPhysics(false);
	for(PxU32 i=0; i<frameCount; i++)
		stepPhysics(false);
	cleanupPhysics(false);
#endif

	return 0;
}
