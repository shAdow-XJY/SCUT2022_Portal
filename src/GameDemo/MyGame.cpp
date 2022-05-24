#include <ctype.h>

#include "PxPhysicsAPI.h"
#include "../Common/Print.h"
#include "../Common/PVD.h"
#include "../Utils/Utils.h"
#include "module.h"
#include "../Common/PhysXCallback.h"
#define PI 3.1415926

using namespace physx;

PxDefaultAllocator		gAllocator;
PxDefaultErrorCallback	gErrorCallback;
PxSimulationEventCallback *myCallBack;

PxFoundation*			gFoundation = NULL;
PxPhysics*				gPhysics	= NULL;

PxDefaultCpuDispatcher*	gDispatcher = NULL;
PxScene*				gScene		= NULL;

PxMaterial*				gMaterial	= NULL;

PxPvd*                  gPvd        = NULL;

PxReal stackZ = 10.0f;

PxRigidDynamic*			body_1		= NULL;
PxRigidDynamic* createDynamic(const PxTransform& t, const PxGeometry& geometry, const PxVec3& velocity=PxVec3(0))
{
	PxRigidDynamic* dynamic = PxCreateDynamic(*gPhysics, t, geometry, *gMaterial, 10.0f);

	//设置trigger的参数
	body_1 = dynamic;
	printf("createDynamic!\n");
	PxShape* treasureShape;
	body_1->getShapes(&treasureShape, 1);
	//treasureShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
	treasureShape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);

	dynamic->setAngularDamping(0.5f);
	dynamic->setLinearVelocity(velocity);
	gScene->addActor(*dynamic);
	return dynamic;
}

void createStack(const PxTransform& t, PxU32 size, PxReal halfExtent)
{
	PxShape* shape = gPhysics->createShape(PxBoxGeometry(halfExtent, halfExtent, halfExtent), *gMaterial);
	for(PxU32 i=0; i<size;i++)
	{
		for(PxU32 j=0;j<size-i;j++)
		{
			PxTransform localTm(PxVec3(PxReal(j*2) - PxReal(size-i), PxReal(i*2+1), 0) * halfExtent);
			PxRigidDynamic* body = gPhysics->createRigidDynamic(t.transform(localTm));
			body->attachShape(*shape);
			PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
			gScene->addActor(*body);
		}
	}
	shape->release();
}
void createWall() {
	PxTransform original(PxVec3(PxReal(0), PxReal(0), PxReal(0)));

	PxShape* shape1 = gPhysics->createShape(PxBoxGeometry(100, 30, 3), *gMaterial);
	PxShape* shape2 = gPhysics->createShape(PxBoxGeometry(3, 30, 100-3), *gMaterial);

	PxTransform local1(PxVec3(PxReal(0), PxReal(30), PxReal(-100)));
	PxTransform local2(PxVec3(PxReal(100-3), PxReal(30), PxReal(0)));
	PxTransform local3(PxVec3(PxReal(0), PxReal(30), PxReal(100)));

	PxRigidStatic* body1 = gPhysics->createRigidStatic(original.transform(local1));
	body1->attachShape(*shape1);
	gScene->addActor(*body1);
	PxRigidStatic* body2 = gPhysics->createRigidStatic(original.transform(local2));
	body2->attachShape(*shape2);
	gScene->addActor(*body2);
	PxRigidStatic* body3 = gPhysics->createRigidStatic(original.transform(local3));
	body3->attachShape(*shape1);
	gScene->addActor(*body3);
	
	shape1->release();
	shape2->release();
}

PxRigidStatic* bodyToRemove;
void remove() {
	gScene->removeActor(*bodyToRemove);
}

void createBowl() {
	PxTransform original(PxVec3(PxReal(150), PxReal(0), PxReal(0)));

	PxShape* bowlShape = gPhysics->createShape(PxCapsuleGeometry(2, 3.5), *gMaterial);
	PxTransform local1(PxVec3(PxReal(-5.5), PxReal(2), PxReal(0)));
	PxTransform local2(PxVec3(PxReal(5.5), PxReal(4), PxReal(0)));
	PxTransform local3(PxVec3(PxReal(16.5), PxReal(5.5), PxReal(0)));
	PxTransform local4(PxVec3(PxReal(27.5), PxReal(8), PxReal(0)));

	PxRigidStatic* body1 = gPhysics->createRigidStatic(original.transform(local1));
	body1->attachShape(*bowlShape);
	/*body1->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_LINEAR_X, true);
	body1->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_LINEAR_Y,true);
	body1->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_LINEAR_Z, true);*/
	gScene->addActor(*body1);
	PxRigidStatic* body2 = gPhysics->createRigidStatic(original.transform(local2));
	body2->attachShape(*bowlShape);
	gScene->addActor(*body2);
	PxRigidStatic* body3 = gPhysics->createRigidStatic(original.transform(local3));
	body3->attachShape(*bowlShape);
	gScene->addActor(*body3);
	PxRigidStatic* body4 = gPhysics->createRigidStatic(original.transform(local4));
	body4->attachShape(*bowlShape);
	gScene->addActor(*body4);

	bowlShape->release();
}

void module::onTrigger(PxTriggerPair* pairs, PxU32 count)
{
	printf("Enter onTrigger!\n");
	for (PxU32 i = 0; i < count; i++)
	{
		// ignore pairs when shapes have been deleted
		if (pairs[i].flags & (PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER | PxTriggerPairFlag::eREMOVED_SHAPE_OTHER)) {
			printf("Enter first 'if' !\n");
			continue;
		}


		if ((pairs[i].otherActor == body0) && (pairs[i].triggerActor == body_1))
		{
			printf("module::onTrigger!\n");
		}
	}
}
void testTrigger() {
	PxTransform original(PxVec3(PxReal(0), PxReal(0), PxReal(0)));//坐标原点
	PxShape* shape1 = gPhysics->createShape(PxBoxGeometry(100, 30, 3), *gMaterial);//长宽高为200 60 6的墙
	PxTransform local4(PxVec3(PxReal(-200), PxReal(30), PxReal(0)), PxQuat(0, 1, 0, 0));
	PxRigidStatic* body4 = gPhysics->createRigidStatic(original.transform(local4));

	module wall(original, shape1, local4, body4);
	wall.body0->attachShape(*shape1);
	gScene->addActor(*wall.body0);

	/*body4->attachShape(*shape1);
	
	gScene->addActor(*body4);*/
	bodyToRemove = body4;

	shape1->release();
}

void initPhysics(bool interactive)
{
	gFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, gAllocator, gErrorCallback);

	gPvd = PxCreatePvd(*gFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	gPvd->connect(*transport,PxPvdInstrumentationFlag::eALL);

	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(),true,gPvd);

	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	gDispatcher = PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher	= gDispatcher;
	sceneDesc.filterShader = defaultPhysX3FilterShader;// PxDefaultSimulationFilterShader;
	//注册onTrigger
	myCallBack = new module();
	sceneDesc.simulationEventCallback = myCallBack;
	gScene = gPhysics->createScene(sceneDesc);

	PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
	if(pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}
	gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);

	PxRigidStatic* groundPlane = PxCreatePlane(*gPhysics, PxPlane(0,1,0,0), *gMaterial);
	gScene->addActor(*groundPlane);

	for(PxU32 i=0;i<5;i++)
		createStack(PxTransform(PxVec3(0,0,stackZ-=10.0f)), 10, 2.0f);

	createWall();
	createBowl();
	testTrigger();

	if(!interactive)
		createDynamic(PxTransform(PxVec3(0,40,100)), PxSphereGeometry(10), PxVec3(0,-50,-100));
}

void stepPhysics(bool interactive)
{
	PX_UNUSED(interactive);
	gScene->simulate(1.0f/60.0f);
	gScene->fetchResults(true);
}
	
void cleanupPhysics(bool interactive)
{
	PX_UNUSED(interactive);
	gScene->release();
	gDispatcher->release();
	gPhysics->release();	
	PxPvdTransport* transport = gPvd->getTransport();
	gPvd->release();
	transport->release();
	
	gFoundation->release();
	
	printf("SnippetHelloWorld done.\n");
}

void keyPress(unsigned char key, const PxTransform& camera)
{
	switch(toupper(key))
	{
	case 'B':	createStack(PxTransform(PxVec3(0,0,stackZ-=10.0f)), 10, 2.0f);						break;
	case ' ':	createDynamic(camera, PxSphereGeometry(3.0f), camera.rotate(PxVec3(0,0,-1))*200);	break;
	case 'R':	remove();	break;
	}
}

#define RENDER_SNIPPET 1
int snippetMain(int, const char*const*)
{
#ifdef RENDER_SNIPPET
	extern void myRenderLoop();
	myRenderLoop();
#else
	static const PxU32 frameCount = 100;
	initPhysics(false);
	for(PxU32 i=0; i<frameCount; i++)
		stepPhysics(false);
	cleanupPhysics(false);
#endif

	return 0;
}