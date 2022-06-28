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
#include <time.h>
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
extern clock_t deltaClock;

extern void renderVisible(const Role& role);
extern void printPxVecFun(const PxVec3& vec);
//��ɫ���������Ĭ��λ��
PxVec3 roleBackPosition = PxVec3(0, 0, 0);
//���Ŀ��λ��
PxVec3 targetPosition = PxVec3(0, 0, 0);

//զ��³��
bool timeStop = false;

//�����ɫ������
PxVec3 dir = PxVec3(0, 0, 0);


bool beginGame = true;
//���ͼ
RenderBox skyBox;
//��Ƶ��
extern SoundTool soundtool;
//��̬��ȾȦ
PxVec3 roleWorldPosition = PxVec3(0);
//�Ƿ�����̬��ȾȦ���������ؾݴ����ò�ͬ�ٶȣ�
bool openDynamicBall = false;
DynamicBall dynamicBall = DynamicBall(openDynamicBall);


extern Animation animation;
extern void renderGameOver();

bool cameraMove = false;

enum keyBoardInputFlags {
	KEYBOARD_NONE = 0,
	KEYBOARD_W = 1 << 0,
	KEYBOARD_A = 1 << 1,
	KEYBOARD_S = 1 << 2,
	KEYBOARD_D = 1 << 3,
	KEYBOARD_UP = 1 << 5,
	KEYBOARD_DOWN = 1 << 6,
	KEYBOARD_LEFT = 1 << 7,
	KEYBOARD_RIGHT = 1 << 8,
	KEYBOARD_T = 1 << 9
};

int keyBoardInputFlag = KEYBOARD_NONE;


Snippets::Camera* sCamera;
namespace Callbacks
{
	

	void cameraInputUpdate() {

		if (keyBoardInputFlag & KEYBOARD_W) {
			sCamera->handleKey('W');
		}
		else if (keyBoardInputFlag & KEYBOARD_A) {
			sCamera->handleKey('A');
		}
		else if (keyBoardInputFlag & KEYBOARD_S) {
			sCamera->handleKey('S');
		}
		else if (keyBoardInputFlag & KEYBOARD_D) {
			sCamera->handleKey('D');
		}
		keyBoardInputFlag = 0;
	}

	void motionCallback(int x, int y)
	{
		sCamera->handleMotion(x, y);
	}

	void keyboardDownCallback(unsigned char key, int x, int y)
	{
		if(key==27)
			exit(0);
	
		switch (toupper(key)) {
		case 'W': {
			keyBoardInputFlag |= KEYBOARD_W;
			cameraMove = true;
			break;
		}
		case 'A': {
			keyBoardInputFlag |= KEYBOARD_A;
			cameraMove = true;
			break;
		}
		case 'S': {
			keyBoardInputFlag |= KEYBOARD_S;
			cameraMove = true;
			break;
		}
		case 'D': {
			keyBoardInputFlag |= KEYBOARD_D;
			cameraMove = true;
			break;
		}
		case 'T': {
			sCamera->handleKey('T');
			break;
		}
		default: {
			keyPress(key, sCamera->getTransform());
		}
		}

	}
	void keyboardUpCallback(unsigned char key, int x, int y)
	{
		key = toupper(key);
		if (key == 'W' || key == 'S'||key=='A'||key=='D') {
			cameraMove = false;
		}
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
		if (role->getRotateOrNot()) {
			sCamera->calDirMoving(key);
		}
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

	if (currentAnimation == "idle") {
		animation.update(0.5);
	}
	else if (currentAnimation == "openDoor")
	{
		if (animation.update(0.5, true)) {
			animation.setAnimation("idle");
		}
	}
	else if (currentAnimation == "pickUp" || currentAnimation == "putDown" || currentAnimation == "notUseKey")
	{
		if (animation.update(1.2, true)) {
			animation.setAnimation("idle");
		}
	}
	else if (currentAnimation == "useKey")
	{
		PxVec3 tempPosition = role->keyDoorActor->getGlobalPose().p;
		PxQuat tempQuat = role->keyDoorActor->getGlobalPose().q;
		role->keyDoorActor->setGlobalPose(PxTransform(PxVec3(tempPosition.x, tempPosition.y + 0.25f, tempPosition.z), tempQuat),false);
		if (animation.update(1.5, true)) {
			animation.setAnimation("idle");
		}
	}
	else if(currentAnimation == "jumping")
	{
		if (animation.update(1.8,true)) {
			animation.setAnimation("idle");
		}
	}
	else if (currentAnimation == "dying")
	{
		if (animation.update(0.5, true)) {
			if (!role->roleOver()) {
				animation.setAnimation("idle");
			}
			else {
				soundtool.playSound("gameOver.wav");
				animation.setAnimation("sleeping");
			}
		}
	}
	else {
		animation.update(1.0);
	}
	
	}

	void renderCallback()
	{
		if (soundtool.getSoundResult()!= FMOD_OK) {
			soundtool.SoundUpdate();
		}
	
		stepPhysics(true);

		if (cameraMove) {
			cameraInputUpdate();
		}
	
		if (!sCamera->isFree() || beginGame) {
			if (beginGame) {
				sCamera->isChangeImmediate = true;
				roleBackPosition = role->getFootPosition() + PxVec3(0, 32, 0) + (role->getFaceDir() * -30);
				beginGame = false;
			}
			//�Թ�
			if (role->nowCheckpoint == 5) {
				
				if (role->getRotateOrNot() && role->getSpeed().isZero()) {
					role->setRotateOrNot(true);
					sCamera->isMoving = 1;
					role->setRotateOrNot(false);
					role->setDir(PxVec3(0, 0, 1));
					
				}
				roleBackPosition = role->getFootPosition() + PxVec3(0, 32, 0) + (PxVec3(0, 0, -30));
			}
			else  if(role->nowCheckpoint != 5){
				
				if (!role->getRotateOrNot() && role->getSpeed().isZero()) {
					role->setRotateOrNot(true);
					sCamera->isMoving = 1;
				}
				roleBackPosition = role->getFootPosition() + PxVec3(0, 32, 0) + (role->getFaceDir() * -30);
			}
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
			//�������ӽǶ�̬��ȾȦ����
			//dynamicBall.printDynamicXYZ();
			dynamicBall.setCircleCenterPosition_XZ(role->getRoleWorldPosition().x, role->getRoleWorldPosition().z);
		}
		else
		{
			/*dir = role->getPosition() - roleBackPosition;
			roleBackPosition = role->getFootPosition() + PxVec3(0, 80, 0) + (role->getDir() * -50);*/
			//cout << "camera" << sCamera->getEye().x << " " << sCamera->getEye().z <<  endl;
			//dynamicBall.printDynamicXYZ();
			//�����ӽǶ�̬��ȾȦ�������
			dynamicBall.setCircleCenterPosition_XZ(sCamera->getEye().x, sCamera->getEye().z);
		}
		Snippets::startRender(sCamera->getEye(), sCamera->getDir());
		
		if (role && !timeStop) {
			//�Ƿ���������
			role->protal();
			//�Ƿ���Ծ
			//�Ƿ���ײ��Ƿ�Ӵ�����
			role->simulationGravity();
			role->roleJump();
			role->roleFall();
			//�Ƿ�������
			role->roleSlide();
			role->rayAround();
			//�Ƿ�����������ģ��
			role->stimulate();
			//��̬������Ⱦ
			roleWorldPosition = role->getRoleWorldPosition();
			role->move();
			//���µ÷�
			role->updateScore();
			if (!role->getAliveStatus()) {
				renderGameOver();
			}
		}
		PxScene* scene;
		PxGetPhysics().getScenes(&scene,1);
		PxU32 nbActors = scene->getNbActors(PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC);
		if(nbActors)
		{
			std::vector<PxRigidActor*> actors(nbActors);
			scene->getActors(PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC, reinterpret_cast<PxActor**>(&actors[0]), nbActors);
			for (int i = 0; i < nbActors; i++) {
				//�Թ���ƽ��·��
				if (actors[i]->getName() == "PrismaticRoad0") {
					PxRigidDynamic* actor = actors[i]->is<PxRigidDynamic>();
					GameSceneBasic* gsb = (GameSceneBasic*)actor->userData;
					PrismaticRoad* pr = (PrismaticRoad*)gsb;
					if (actor->getGlobalPose().p.x <= pr->getEndPosition().x) {
						if (openDynamicBall) {
							actor->setLinearVelocity(PxVec3(15, 0, 0));
						}
						else {
							actor->setLinearVelocity(PxVec3(40, 0, 0));
						}
					}
					else if (actor->getGlobalPose().p.x >= pr->getStartPosition().x) {
						if (openDynamicBall) {
							actor->setLinearVelocity(PxVec3(-15, 0, 0));
						}
						else {
							actor->setLinearVelocity(PxVec3(-40, 0, 0));
						}
					}
				}
				//�ڴ���ƽ��·��
				else if (actors[i]->getName() == "PrismaticRoad1") {
					PxRigidDynamic* actor = actors[i]->is<PxRigidDynamic>();
					GameSceneBasic* gsb = (GameSceneBasic*)actor->userData;
					PrismaticRoad* pr = (PrismaticRoad*)gsb;
					if (actor->getGlobalPose().p.x <= pr->getStartPosition().x) {
						if (openDynamicBall) {
							actor->setLinearVelocity(PxVec3(15, 0, 0));
						}
						else {
							actor->setLinearVelocity(PxVec3(30, 0, 0));
						}
					}
					else if (actor->getGlobalPose().p.x >= pr->getEndPosition().x) {
						if (openDynamicBall) {
							actor->setLinearVelocity(PxVec3(-15, 0, 0));
						}
						else {
							actor->setLinearVelocity(PxVec3(-30, 0, 0));
						}
					}
				}
				
			}
			Snippets::renderActors(&actors[0], static_cast<PxU32>(actors.size()), true);
		}
		
		// ��Ⱦģ��
		if (role->isStaticAttached()) {
			renderVisible(*role);
		}
		

		/** ������� */
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
* @brief ���ڴ�С���ú���
**/
void reshape(int width, int height)
{
	cout << "New width: " << width << " " << "New height: " << height << endl;
	ImGui_ImplGLUT_ReshapeFunc(width, height);
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

	/** ��ʼ����� */
	skyBox.Init(true);
	animation.init();

	
	glutIdleFunc(Callbacks::idleCallback);
	glutDisplayFunc(Callbacks::renderCallback);
	Callbacks::motionCallback(0,0);
	atexit(Callbacks::exitCallback);
	
	initPhysics(true);
	loadTexture();
	initGame();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.Fonts->AddFontFromFileTTF("../../src/ImGui/segoeui.ttf",23.0f);
	ImGui::StyleColorsDark();

	ImGui_ImplGLUT_Init();
	ImGui_ImplGLUT_InstallFuncs();
	ImGui_ImplOpenGL2_Init();


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



