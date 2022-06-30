
#include <ctype.h>
#include "PxPhysicsAPI.h"

#include "../Common/Print.h"
#include "../Common/PVD.h"
#include "../Utils/Utils.h"
#include "../Role/Role.h"
#include "../LoadModel/Model.h"
#include "../Render/BMPLoader.h"
#include "../Sound/SoundTools.h"
#include "../Animation/Animation.h"
#include <vector>
#include <string>
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
PxMaterial*				pMaterial	= NULL;

PxPvd*                  gPvd        = NULL;

PxReal stackZ = 10.0f;


Role* role = NULL;
PxControllerManager* cManager = NULL;


extern PxVec3 ScenetoWorld(int xCord, int yCord);

const PxTransform t = PxTransform(PxVec3(-50, 0, -250));


//������ͼID����
std::map<string, unsigned int> textureMap;
std::string texture[] = { "Door","Wall","Road",
							"SeesawBox","Seesaw","Ice",
							"KeyDoor","Prop", "RotateRod",
							"PrismaticRoad0","PrismaticRoad1","PrismaticRoad2","Prismatic","Gear",
							"FerrisWheel","PoolWall","Ground","FinalLine","PipeWall"};

//��Ƶ��
SoundTool soundtool = SoundTool();


// ������������ģ��
Animation animation;


//�ɶ�ȡ�Ĵ浵��
vector<PxVec3> checkpoints = {
	//t.transform(PxVec3(0,7.0,0)),
	//t.transform(PxVec3(88, 36, 26)), //�ڴ�ǰ
	//t.transform(PxVec3(270, 36, 71)), //�Թ�ǰ
	//t.transform(PxVec3(122, 36, 392.8)), //�Թ�����
	//t.transform(PxVec3(46, 69.6, 403.8)) //��ת�˹ؿ�����
};


//��������
void loadTexture() {	
	for (auto name : texture) {
		string baseUrl = "../../texture/";
		CBMPLoader* BMPLoader = new CBMPLoader();
		unsigned int id = BMPLoader->generateModelID((baseUrl + name + ".bmp").c_str());
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
	//role->setFootPosition(checkpoints[0]);
	//�ڴ�ǰλ��
    //role->setFootPosition(checkpoints[1]);
	//Ħ����ǰλ��
	role->setFootPosition(checkpoints[2]);
	//��ת·�ؿ�ǰλ��
	//role->setFootPosition(checkpoints[3]);
	//�Թ�ǰλ��
	//role->setFootPosition(checkpoints[4]);
	//�Թ�����λ�� 
	//role->setFootPosition(checkpoints[5]);
	//��ת�˹ؿ�����λ��
	//role->setFootPosition(checkpoints[6]);
	//��Ӿ��λ��
	//role->setFootPosition(checkpoints[7]);
	//�յ�λ��
	//role->setFootPosition(checkpoints[8]);
	role->fall();

	animation.attachRole(*role);
	animation.setAnimation("idle");
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
	//��Ħ������Ħ����restitution�ָ�ԭ״(����)
	gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.0f);
	pMaterial = gPhysics->createMaterial(0.3f, 0.3f, 0.2f);

	PxRigidStatic* groundPlane = PxCreatePlane(*gPhysics, PxPlane(0, 1, 0, 0), *gMaterial);
	groundPlane->setName("Over");
	gScene->addActor(*groundPlane);
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