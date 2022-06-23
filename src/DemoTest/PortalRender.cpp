#define RENDER_SNIPPET 1
#ifdef RENDER_SNIPPET
#include <vector>

#include "PxPhysicsAPI.h"
#include "../Render/Render.h"
#include "../Render/Camera.h"
#include "../Role/Role.h"
#include <Render/RenderBox.h>
#include <Render/DynamicBall.h>
#include <Sound/SoundTools.h>
#include <Animation/Animation.h>
using namespace physx;
extern void initPhysics(bool interactive);
extern void stepPhysics(bool interactive);	
extern void cleanupPhysics(bool interactive);
extern void keyPress(unsigned char key, const PxTransform& camera);
extern void keyRelease(unsigned char key);
extern void mousePress(int button, int state, int x, int y);
extern void specialKeyPress(GLint key);
extern void specialKeyRelease(GLint key);
extern void calculateElapsedClocksFromLastFrame();
extern void loadTexture();
extern void initGame();

extern Role* role;

extern void renderVisible(const Role& role);
//角色背后照相机默认位置
PxVec3 roleBackPosition = PxVec3(0, 0, 0);

//看向角色的视线
PxVec3 dir = PxVec3(0, 0, 0);


bool beginGame = true;
//天空图
RenderBox skyBox;
//音频类
extern SoundTool soundtool;
//动态渲染圈
PxVec3 roleWorldPosition = PxVec3(0);
DynamicBall dynamicBall = DynamicBall(true);

extern Animation animation;

namespace
{
	Snippets::Camera*	sCamera;

	void motionCallback(int x, int y)
	{
		sCamera->handleMotion(x, y);
	}
void keyboardDownCallback(unsigned char key, int x, int y)
{
	if(key==27)
		exit(0);
	if(!sCamera->handleKey(key, x, y))
		keyPress(key, sCamera->getTransform());
}
void keyboardUpCallback(unsigned char key, int x, int y)
{
	keyRelease(key);
}

void specialKeyDownCallback(GLint key, GLint x, GLint y)
{
	role->move(key,true,sCamera->isFree());
	
	specialKeyPress(key);
}

void specialKeyUpCallback(GLint key, GLint x, GLint y)
{
	role->move(key, false, sCamera->isFree());

	sCamera->calDirMoving(key);
	specialKeyRelease(key);
}

void mouseCallback(int button, int state, int x, int y)
{
	sCamera->handleMouse(button, state, x, y);
	mousePress(button, state, x, y);
}

void idleCallback()
{
	glutPostRedisplay();
}

void animationRenderCallback() {
	animation.display();

	string currentAnimation = animation.getCurrentAnimation();

	if ((currentAnimation == "idle")) 
	{
		animation.update(0.5);
	}
	/*else if (currentAnimation == "walk")
	{
		animation.update(1000 * timeAlways);
		timeAlways++;
	}*/
	else if(currentAnimation == "jump")
	{
		if (animation.update(2.0,true)) {
			animation.setAnimation("idle");
		}
	}
	else if(currentAnimation == "walk"){
		animation.update(1.0);
	}
	
}

void renderCallback()
{
	if (soundtool.getSoundResult()!= FMOD_OK) {
		soundtool.SoundUpdate();
	}
	
	stepPhysics(true);

		
		if (!sCamera->isFree() || beginGame) {
			if (beginGame) {
				sCamera->isChangeImmediate = true;
				beginGame = false;
			}
			roleBackPosition = role->getFootPosition() + PxVec3(0, 50, 0) + (role->getFaceDir() * -50);
			if (!sCamera->isMoving) {
				sCamera->setEye(roleBackPosition);
				role->changeCanMove(true);
			}
			else {
				role->changeCanMove(false);
			}
			dir = role->getPosition() - roleBackPosition;
			sCamera->targetDir = dir;
			sCamera->updateDir(role->getPosition());
			//非自由视角动态渲染圈跟人
			dynamicBall.setCircleCenterPosition_XZ(role->getRoleWorldPosition().x, role->getRoleWorldPosition().z);
		}
		else
		{
			dir = role->getPosition() - roleBackPosition;
			roleBackPosition = role->getFootPosition() + PxVec3(0, 50, 0) + (role->getDir() * -50);
			//自由视角动态渲染圈跟摄像机
			dynamicBall.setCircleCenterPosition_XZ(sCamera->getEye().x, sCamera->getEye().z);
		}
		Snippets::startRender(sCamera->getEye(), sCamera->getDir());
		
		if (role) {
			//是否重生传送
			role->protal();
			//是否跳跃
			//是否检测底部是否接触物体
			role->simulationGravity();
			role->roleJump();
			role->roleFall();
			//是否发生滑动
			role->roleSlide();
			role->rayAround();
			//是否进行物理刚体模拟
			role->stimulate();
			//动态刚体渲染
			roleWorldPosition = role->getRoleWorldPosition();
			dynamicBall.setCircleCenterPosition_XZ(roleWorldPosition.x, roleWorldPosition.z);
			role->move();
			//更新得分
			role->updateScore();
		}
		PxScene* scene;
		PxGetPhysics().getScenes(&scene,1);
		PxU32 nbActors = scene->getNbActors(PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC);
		if(nbActors)
		{
			std::vector<PxRigidActor*> actors(nbActors);
			scene->getActors(PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC, reinterpret_cast<PxActor**>(&actors[0]), nbActors);
			Snippets::renderActors(&actors[0], static_cast<PxU32>(actors.size()), true);
		}
		
		// 渲染模型
		if (role->isStaticAttached()) {
			renderVisible(*role);
		}
		

		/** 绘制天空 */
		skyBox.CreateSkyBox(-2000, -200, -2000, 1.0, 0.5, 1.0);
		
		animationRenderCallback();

		Snippets::finishRender();
		
		calculateElapsedClocksFromLastFrame();
	}

void exitCallback(void)
	{
		delete sCamera;
		cleanupPhysics(true);
	}
}

/**
* @brief 窗口大小重置函数
**/
void reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, GLUT_WINDOW_WIDTH, 0, GLUT_WINDOW_HEIGHT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

}


void renderLoop()
{
	sCamera = new Snippets::Camera(PxVec3(50.0f, 50.0f, 50.0f), PxVec3(-0.6f, -0.2f, -0.7f));

	Snippets::setupDefaultWindow("PhysX Demo");

	Snippets::setupDefaultRenderState();

	/** 初始化天空 */
	skyBox.Init(true);
	animation.init();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();

	ImGui_ImplGLUT_Init();
	ImGui_ImplGLUT_InstallFuncs();
	ImGui_ImplOpenGL2_Init();

	glutIdleFunc(idleCallback);
	glutDisplayFunc(renderCallback);
	glutKeyboardFunc(keyboardDownCallback);
	glutKeyboardUpFunc(keyboardUpCallback);
	glutSpecialFunc(specialKeyDownCallback);
	glutSpecialUpFunc(specialKeyUpCallback);
	glutMouseFunc(mouseCallback);
	glutMotionFunc(motionCallback);
	glutReshapeFunc(reshape);
	motionCallback(0,0);
	atexit(exitCallback);
	

	initPhysics(true);
	loadTexture();
	initGame();
	glutMainLoop();

	ImGui_ImplOpenGL2_Shutdown();
	ImGui_ImplGLUT_Shutdown();
	ImGui::DestroyContext();
}


#endif

LRESULT CALLBACK HostWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_KEYDOWN:
		if (wParam == VK_LEFT)
		{
			std::cout << "...." << std::endl;
		}
		break;

		if (wParam == VK_F1)
		{
			std::cout << "...." << std::endl;
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}



