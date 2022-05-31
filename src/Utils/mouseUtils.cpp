<<<<<<< HEAD
ï»¿#include "PxPhysicsAPI.h"
=======
#include "PxPhysicsAPI.h"
>>>>>>> 03a4d8441bd50d8f97e38590362942f4dfb0fc4e
#include "../Render/Render.h"
using namespace physx;

/**
<<<<<<< HEAD
* @brief Ä»Î¬×ªÎªÎ¬
* @param xCord Ä»xÖµ yCord Ä»yÖµ
* @return PxVec3
**/
PxVec3 ScenetoWorld(int xCord, int yCord) {
	//ä»»
	GLint viewport[4];
	GLdouble modelview[16];
	GLdouble projection[16];
	//Í¶Ó°
	GLfloat winX, winY, winZ;
	//
	GLdouble posX, posY, posZ;

	glPushMatrix();
	glGetIntegerv(GL_VIEWPORT, viewport);
=======
* @brief ÊäÈëÆÁÄ»¶þÎ¬×ø±ê×ª»»ÎªÈýÎ¬×ø±ê
* @param xCord ÆÁÄ»×ø±êxÖµ yCord ÆÁÄ»×ø±êyÖµ
* @return PxVec3 ÊÀ½ç×ø±ê
**/
PxVec3 ScenetoWorld(int xCord,int yCord) {
	//Èý´ó±ä»»¾ØÕó
	GLint viewport[4];
	GLdouble modelview[16];
	GLdouble projection[16];
	//Í¶Ó°×ø±ê
	GLfloat winX, winY, winZ;
	//ÊÀ½ç×ø±ê
	GLdouble posX, posY, posZ;

	glPushMatrix();  
	glGetIntegerv(GL_VIEWPORT, viewport);  
>>>>>>> 03a4d8441bd50d8f97e38590362942f4dfb0fc4e
	glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	glPopMatrix();

	winX = xCord;
<<<<<<< HEAD
	winY = viewport[3] - (float)yCord; //Â½Î»
	glReadPixels((int)winX, (int)winY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);

	gluUnProject(winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);

=======
	winY = viewport[3] - (float)yCord; //×óÏÂ½ÇÎ»ÖÃÐÞÕý
	glReadPixels((int)winX, (int)winY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);

	gluUnProject(winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);
	
>>>>>>> 03a4d8441bd50d8f97e38590362942f4dfb0fc4e
	return PxVec3(posX, posY, posZ);
}