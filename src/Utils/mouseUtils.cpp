<<<<<<< HEAD
﻿#include "PxPhysicsAPI.h"
=======
#include "PxPhysicsAPI.h"
>>>>>>> 03a4d8441bd50d8f97e38590362942f4dfb0fc4e
#include "../Render/Render.h"
using namespace physx;

/**
<<<<<<< HEAD
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
=======
* @brief ������Ļ��ά����ת��Ϊ��ά����
* @param xCord ��Ļ����xֵ yCord ��Ļ����yֵ
* @return PxVec3 ��������
**/
PxVec3 ScenetoWorld(int xCord,int yCord) {
	//����任����
	GLint viewport[4];
	GLdouble modelview[16];
	GLdouble projection[16];
	//ͶӰ����
	GLfloat winX, winY, winZ;
	//��������
	GLdouble posX, posY, posZ;

	glPushMatrix();  
	glGetIntegerv(GL_VIEWPORT, viewport);  
>>>>>>> 03a4d8441bd50d8f97e38590362942f4dfb0fc4e
	glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	glPopMatrix();

	winX = xCord;
<<<<<<< HEAD
	winY = viewport[3] - (float)yCord; //½λ
	glReadPixels((int)winX, (int)winY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);

	gluUnProject(winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);

=======
	winY = viewport[3] - (float)yCord; //���½�λ������
	glReadPixels((int)winX, (int)winY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);

	gluUnProject(winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);
	
>>>>>>> 03a4d8441bd50d8f97e38590362942f4dfb0fc4e
	return PxVec3(posX, posY, posZ);
}