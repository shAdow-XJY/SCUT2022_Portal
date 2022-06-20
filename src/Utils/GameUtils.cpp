#include "PxPhysicsAPI.h"
#include "../Render/Render.h"
#include "../Role/Role.h"
#include <iostream>
#include <time.h>
using namespace physx;
using namespace std;

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
clock_t deltaClock = 0;

void calculateElapsedClocksFromLastFrame() {
	//更新deltaTime给需要速度的地方使用
	currClock = clock();
	//deltaTime = static_cast<double>((currClock - lastClock) / CLOCKS_PER_SEC);
	deltaClock = currClock - lastClock;
	lastClock = currClock;
}

/**
* @brief 通用打印PxVec函数
**/
void printPxVecFun(const PxVec3& vec) {
	cout << "X:" << vec.x << " " << "Y:" << vec.y << " " << "Z:" << vec.z << endl;
}

void renderVisible(const Role& role) {
	size_t indices = 0;
	glPushMatrix();

	float identity[] = {
		1.0f,0.0f,0.0f,0.0f,
		0.0f,1.0f,0.0f,0.0f,
		0.0f,0.0f,1.0f,0.0f,
		0.0f,0.0f,0.0f,1.0f
	};
	PxMat44 modelMatrix(PxShapeExt::getGlobalPose(*role.getShape(), *role.getActor()));
	PxMat44 rotate(PxQuat(-PxHalfPi, PxVec3(0.0f, 0.0f, 1.0f)));
	glMultMatrixf(reinterpret_cast<const float*>(&modelMatrix));
	glMultMatrixf(reinterpret_cast<const float*>(&rotate));
	glGetFloatv(GL_MODELVIEW_MATRIX, identity);

	glGetFloatv(GL_MODELVIEW_MATRIX, identity);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	//glScalef(2.0f, 2.0f, 2.0f);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glNormalPointer(GL_FLOAT, 0, &role.getModel().m_normals[0]);
	glVertexPointer(3, GL_FLOAT, 0, &role.getModel().m_vertices[0]);
	glTexCoordPointer(2, GL_FLOAT, 0, &role.getModel().m_texCoords[0]);
	//glDrawElements(GL_TRIANGLES, role.getModel().m_indices.size(), GL_UNSIGNED_INT, &role.getModel().m_indices[0]);
	glDrawArrays(GL_TRIANGLES, 0, int(role.getModel().m_vertices.size()));
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glPopMatrix();
}