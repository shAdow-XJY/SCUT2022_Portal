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

// 左键鼠标按下
bool press = false;
//用于过滤的属性
const PxFilterData collisionGroupIgnore(0, 0, 0, 1);  // 碰撞会被忽略的组
const PxFilterData collisionGroup(1, 0, 0, 0); // 需要碰撞的组



extern PxVec3 ScenetoWorld(int xCord, int yCord);



// 鼠标点击时的坐标
int mouseX, mouseY;

// 提示字符的位置（测试用）
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
//创造动态刚体
PxRigidDynamic* createDynamic( PxReal radius, const PxTransform& t, const PxVec3& velocity=PxVec3(0))
{


	PxShape* shape = gPhysics->createShape(PxSphereGeometry(radius), *gMaterial);
	setupFiltering(shape, FilterGroup::eBIRD, FilterGroup::ePIG);


	//PxPhysics object，transform of the new object ，shape of the new object ，the density of the new object(>0)
	PxRigidDynamic* dynamic = gPhysics->createRigidDynamic(t);
	dynamic->attachShape(*shape);

	
	

	//设置角阻尼系数，还有线性阻尼linearDamping；线性阻尼控制物理形体或约束抵抗平移的量,而角阻尼控制其抵抗旋转的量。如果设置为0，物体会一直旋转/平移
	dynamic->setAngularDamping(10.0f);


	//设置线性速度 
	dynamic->setLinearVelocity(velocity);

	PxRigidBodyExt::updateMassAndInertia(*dynamic, 1.0f);
	//加入鸟队列
	ballBirdList.push_back(dynamic);

	dynamic->setName(BirdName);

	gScene->addActor(*dynamic);
	return dynamic;
}

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
	
	setupFiltering(shape, FilterGroup::ePIG, FilterGroup::eBIRD);


	for(PxU32 i=0; i<size;i++)
	{
		for(PxU32 j=0;j<size;j++)
		{
			//指定位置(-10/-7..9,1,0)(-7..,3,0)(-4..,5,0)...
			PxTransform localTm(PxVec3(PxReal(j*2) - PxReal(size), PxReal(i*2+1), 0) * halfExtent);
			//createRigidDynamic构建刚体
			PxRigidDynamic* body = gPhysics->createRigidDynamic(t.transform(localTm));
			//attachShape绑定形状到刚体上;
			body->attachShape(*shape);
			//更新质量和惯性（数值表示密度）
			PxRigidBodyExt::updateMassAndInertia(*body, 1.0f);

			body->setName(PigName);

			body->userData = body;

			
			//加入猪队列
			ballPigList.push_back(body);


			//把Actor添加到场景中,注释掉这一句之后所有立方体变得不可见且没有碰撞体积
			gScene->addActor(*body);
			//gScene->removeActor(*body);

		}
	}
	//释放
	shape->release();
}

//创建道路方块
void createRoad(const PxTransform& t,PxReal halfExtent,std::string name) {
	Block* r = new Block(halfExtent,name);
	PxShape* shape = gPhysics->createShape(PxBoxGeometry(halfExtent, halfExtent, halfExtent), *gMaterial);
	shape->setQueryFilterData(collisionGroup);
	PxRigidDynamic* body = gPhysics->createRigidDynamic(t);
	body->attachShape(*shape);
	//更新质量和惯性（数值表示密度）
	PxRigidBodyExt::updateMassAndInertia(*body, 1.0f);
	body->setName("Ground");
	body->userData = r;
	//把Actor添加到场景中,注释掉这一句之后所有立方体变得不可见且没有碰撞体积
	gScene->addActor(*body);
}

//实例化物理
void initPhysics(bool interactive)
{
	//PxFoundation(版本号,内存回调,错误回调)
	gFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, gAllocator, gErrorCallback);
	//PVD
	gPvd = PxCreatePvd(*gFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	gPvd->connect(*transport,PxPvdInstrumentationFlag::eALL);
	//创建顶级PxPhysics对象
	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(),true,gPvd);

	// 创建Cooking对象
	gCooking = PxCreateCooking(PX_PHYSICS_VERSION, *gFoundation, PxCookingParams(PxTolerancesScale()));

	//?缩放
	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	//重力
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
	//静摩擦，动摩擦，restitution恢复原状(弹性)
	gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.0f);
	PxRigidStatic* groundPlane = PxCreatePlane(*gPhysics, PxPlane(0,1,0,0), *gMaterial);
	
	groundPlane->setName("over");
	gScene->addActor(*groundPlane);

	// 测试代码
	Model testModel("C:/Users/bwz12/Downloads/keqing-fbx-with-textures/source/Keqing/Keqing/Keqing model/Keqing.fbx");
	//Model testModel("C:/Users/bwz12/Downloads/hololive-gawr-gura/source/Sketchfab_2020_12_03_09_08_48.obj");
	//Model testModel("C:/Users/bwz12/Downloads/ball.glb");
	testModel.createMeshActor((PxTransform(PxVec3(20, 30, 30))).transform(PxTransform(PxQuat(-PxHalfPi,PxVec3(1.0f,0.0f,0.0f)))));
	//testModel.createMeshActor(PxTransform(20, 30, 30));
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
	//if (不交互)，在render中把交互设成false就有一个默认的球滚过去撞击堆。
	if(!interactive)
		//PxSphereGeometry Transform,geometry,velocity（速度）
		createDynamic(10,PxTransform(PxVec3(0,40,100)), PxVec3(0,-50,-100));
}

/**
* @brief 发送射线
* @param origin 发送射线的坐标 unitDir 发送射线的方向 Road游戏道路基类
**/
bool RayCast(PxVec3 origin, PxVec3 unitDir, Block& block)
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
* @brief 角色底部发送射线
**/
void RayCastByRole() {
	PxVec3 origin = role->getFootPosition();
	PxVec3 unitDir = PxVec3(0, -999, 0);
	if (RayCast(origin, unitDir, role->standingBlock)) {
		//碰撞到物体
		//std::cout << "碰到地面" << std::endl;
		if (role->standingBlock.getBlockType() == BlockType::ground) {
			//std::cout << role->standingBlock.getName()<<std::endl;
		}
	}
	else {
		if (role->standingBlock.getBlockType() != BlockType::error) {
			role->setFootPosition(role->getFootPosition() + role->getSpeed()*5.0f);
		}
		role->standingBlock = Block();
		//std::cout << "未碰到地面" << std::endl;	
		//role->gameOver();
		role->fall();
	}
}

//（在render中调用）
void stepPhysics(bool interactive)
{
	PX_UNUSED(interactive);
	//时间
	gScene->simulate(1.0f/60.0f);
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
	switch(toupper(key))
	{
	case 'B':	createStack(PxTransform(PxVec3(0,0,stackZ-=10.0f)), 10, 2.0f);break;
	//PxSphereGeometry Transform,geometry,velocity（速度）
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
				std::cout << "不可点击";
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
