
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
std::string texture[] = { "Door","Wall","Road","SeesawBox","Seesaw","Ice","man"};

//音频类
SoundTool soundtool = SoundTool();


// 带动画的人物模型
Animation animation;


//可读取的存档点
vector<PxVec3> checkpoints = {
	//t.transform(PxVec3(0,7.0,0)),
	//t.transform(PxVec3(88, 36, 26)), //摆锤前
	//t.transform(PxVec3(270, 36, 71)), //迷宫前
	//t.transform(PxVec3(122, 36, 392.8)), //迷宫出口
	//t.transform(PxVec3(46, 69.6, 403.8)) //旋转杆关卡角落
};


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
	//role->setFootPosition(checkpoints[0]);
	//摆锤前位置
	//role->setFootPosition(checkpoints[1]);
	//摩天轮前位置
	//role->setFootPosition(checkpoints[2]);
	//旋转路关卡前位置
	//role->setFootPosition(checkpoints[3]);
	//迷宫前位置
	role->setFootPosition(checkpoints[4]);
	//迷宫出口位置 
	//role->setFootPosition(checkpoints[5]);
	//平移路段前位置 
	//role->setFootPosition(t.transform(PxVec3(66, 69.6, 421.8)));
	//旋转杆关卡角落位置
	//role->setFootPosition(checkpoints[6]);
	//role->attachModel("../../models/paimon/paimon.obj");
	role->fall();

	animation.attachRole(*role);
	animation.setAnimation("idle");
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