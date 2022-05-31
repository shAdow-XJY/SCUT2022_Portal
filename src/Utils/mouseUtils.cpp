#include "PxPhysicsAPI.h"
#include "PxPhysicsAPI.h"
#include "../Render/Render.h"
using namespace physx;


PxVec3 ScenetoWorld(int xCord,int yCord) {
	//三大变换矩阵
	GLint viewport[4];
	GLdouble modelview[16];
	GLdouble projection[16];
	//投影坐标
	GLfloat winX, winY, winZ;
	//世界坐标
	GLdouble posX, posY, posZ;

	glPushMatrix();  
	glGetIntegerv(GL_VIEWPORT, viewport);  
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	glPopMatrix();

	winX = xCord;
	winY = viewport[3] - (float)yCord; 
	glReadPixels((int)winX, (int)winY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);

	gluUnProject(winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);

	return PxVec3(posX, posY, posZ);
}