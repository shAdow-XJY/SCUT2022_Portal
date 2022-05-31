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
<<<<<<< HEAD
#include "../Role/CCT.h"
=======
#include "../Role/Role.h"


>>>>>>> 03a4d8441bd50d8f97e38590362942f4dfb0fc4e
using namespace physx;

extern void initPhysics(bool interactive);
extern void stepPhysics(bool interactive);	
extern void cleanupPhysics(bool interactive);
extern void keyPress(unsigned char key, const PxTransform& camera);
<<<<<<< HEAD
extern void specialKeyPress(GLint key);
extern CCTRole* cctRole;
=======
extern void mousePress(int button, int state, int x, int y);
extern Role* role;
>>>>>>> 03a4d8441bd50d8f97e38590362942f4dfb0fc4e

namespace
{
Snippets::Camera*	sCamera;

void motionCallback(int x, int y)
{
	sCamera->handleMotion(x, y);
}

void keyboardCallback(unsigned char key, int x, int y)
{
	if(key==27)
		exit(0);

	if(!sCamera->handleKey(key, x, y))
		keyPress(key, sCamera->getTransform());

	role->move(key);
}

void SpecialKeyCallback(GLint key, GLint x, GLint y)
{
	specialKeyPress(key);
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
	stepPhysics(true);
<<<<<<< HEAD

	if (!sCamera->isFree()) {
		sCamera->setEye(cctRole->getFootPosition() + PxVec3(0, 50, -50));
	}
	Snippets::startRender(sCamera->getEye(), sCamera->getDir());

	if (cctRole) {
		cctRole->roleJump();
		cctRole->roleFall();
=======
	
	if (!sCamera->isFree()) {
		sCamera->setEye(role->getFootPosition() + PxVec3(0,50,-50));
	}
	Snippets::startRender(sCamera->getEye(), sCamera->getDir());

	if (role) {
		role->move();
>>>>>>> 03a4d8441bd50d8f97e38590362942f4dfb0fc4e
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

	Snippets::finishRender();
}

void exitCallback(void)
{
	delete sCamera;
	cleanupPhysics(true);
}
}


void renderLoop()
{
	sCamera = new Snippets::Camera(PxVec3(0.0f, 250.0f, 0.0f), PxVec3(-0.1f,-0.8f,-0.1f));

	Snippets::setupDefaultWindow("PhysX Demo");
	Snippets::setupDefaultRenderState();

	glutIdleFunc(idleCallback);
	glutDisplayFunc(renderCallback);
	glutKeyboardFunc(keyboardCallback);
	glutSpecialFunc(SpecialKeyCallback);
	glutMouseFunc(mouseCallback);
	glutMotionFunc(motionCallback);
	motionCallback(0,0);

	atexit(exitCallback);

	initPhysics(true);
	glutMainLoop();
}
#endif
