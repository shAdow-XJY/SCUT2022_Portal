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
#define RENDER_SNIPPET 1
#ifdef RENDER_SNIPPET
#include <vector>

#include "PxPhysicsAPI.h"
#include "../Render/Render.h"
#include "../Render/Camera.h"
#include "../Role/Role.h"
#include <Render/RenderBox.h>
#include <Render/DynamicCircle.h>
#include <Sound/SoundTools.h>
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
DynamicCircle dynamicCircle = DynamicCircle(false);
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
	role->move(key,false,sCamera->isFree());
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
			
		}
		else
		{
			dir = role->getPosition() - roleBackPosition;
			roleBackPosition = role->getFootPosition() + PxVec3(0, 50, 0) + (role->getDir() * -50);
		}
		Snippets::startRender(sCamera->getEye(), sCamera->getDir());

		/*if (sCamera->isFree())
		{
			role->move();
		}*/
		if (role) {
			role->roleJump();
			role->roleFall();
			role->roleSlide();	
			role->rayAround();
			role->simulationGravity();
			role->stimulate();
			
			roleWorldPosition = role->getRoleWorldPosition();
			dynamicCircle.setCenterPosition(roleWorldPosition.x, roleWorldPosition.z);
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
		/** 绘制天空 */
		skyBox.CreateSkyBox(-2000, -200, -2000, 1.0, 0.5, 1.0);

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



