#include "PxPhysicsAPI.h"
#include "../Render/Render.h"
using namespace physx;

/**
* @brief ĻάתΪά
* @param xCord Ļxֵ yCord Ļyֵ
* @return PxVec3
**/
PxVec3 ScenetoWorld(int xCord, int yCord) {
	//任
	GLint viewport[4];
	GLdouble modelview[16];
	GLdouble projection[16];
	//ͶӰ
	GLfloat winX, winY, winZ;
	//
	GLdouble posX, posY, posZ;

	glPushMatrix();
	glGetIntegerv(GL_VIEWPORT, viewport);
	glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	glPopMatrix();

	winX = xCord;
	winY = viewport[3] - (float)yCord; //½λ
	glReadPixels((int)winX, (int)winY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);

	gluUnProject(winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);

	return PxVec3(posX, posY, posZ);
}