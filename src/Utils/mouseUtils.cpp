#include "PxPhysicsAPI.h"
#include "../Render/Render.h"
#include <time.h>
using namespace physx;

/**
* @brief 屏幕二维坐标转换到三维屏幕坐标
* @param int xCord 屏幕x坐标 int yCord 屏幕y坐标
* @return PxVec3
**/
PxVec3 ScenetoWorld(int xCord, int yCord) {

	GLint viewport[4];
	GLdouble modelview[16];
	GLdouble projection[16];
	//投影坐标
	GLfloat winX, winY, winZ;
	//三维坐标点
	GLdouble posX, posY, posZ;

	glPushMatrix();
	glGetIntegerv(GL_VIEWPORT, viewport);
	glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	glPopMatrix();

	winX = xCord;
	winY = viewport[3] - (float)yCord; //修正为左下角为起点
	glReadPixels((int)winX, (int)winY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);

	gluUnProject(winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);

	return PxVec3(posX, posY, posZ);
}

clock_t currClock = 0, lastClock = 0;
clock_t deltaClock;

void calculateElapsedClocksFromLastFrame() {
	//更新deltaTime给需要速度的地方使用
	currClock = clock();
	//deltaTime = static_cast<double>((currClock - lastClock) / CLOCKS_PER_SEC);
	deltaClock = currClock - lastClock;
	lastClock = currClock;
}