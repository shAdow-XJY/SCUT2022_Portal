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
	cout << "render model" << endl;
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

	//glScalef(2.0f, 2.0f, 2.0f);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glEnable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glColor4f(1, 1, 1, 1);
	for (auto mesh : role.getModel().m_meshes) {
		glColor4f(1, 1, 1, 1);
		glBindTexture(GL_TEXTURE_2D, mesh.m_texture.m_texID);
		glNormalPointer(GL_FLOAT, 0, &mesh.m_normals[0]);
		glVertexPointer(3, GL_FLOAT, 0, &mesh.m_vertices[0]);
		glTexCoordPointer(2, GL_FLOAT, 0, &mesh.m_texCoords[0]);
		
		glDrawElements(GL_TRIANGLES, mesh.m_vertices.size(), GL_UNSIGNED_INT, &mesh.m_indices[0]);
		//glDrawArrays(GL_TRIANGLES, 0, int(mesh.m_vertices.size()));

		// 指定顶点与纹理坐标的旧代码
		/*size_t numFaces = mesh.m_vertices.size() / 3;
		for (size_t i = 0; i < numFaces; ++i) {
			glBegin(GL_TRIANGLES);
			GLfloat t1[2] = { mesh.m_texCoords[3 * i].x, mesh.m_texCoords[3 * i].y };
			glTexCoord2fv(t1);
			GLfloat v1[3] = { mesh.m_vertices[3 * i].x,mesh.m_vertices[3 * i].y,mesh.m_vertices[3 * i].z };
			glVertex3fv(v1);

			GLfloat t2[2] = { mesh.m_texCoords[3 * i + 1].x, mesh.m_texCoords[3 * i + 1].y };
			glTexCoord2fv(t2);
			GLfloat v2[3] = { mesh.m_vertices[3 * i + 1].x,mesh.m_vertices[3 * i + 1].y,mesh.m_vertices[3 * i + 1].z };
			glVertex3fv(v2);

			GLfloat t3[2] = { mesh.m_texCoords[3 * i + 2].x, mesh.m_texCoords[3 * i + 2].y };
			glTexCoord2fv(t3);
			GLfloat v3[3] = { mesh.m_vertices[3 * i + 2].x,mesh.m_vertices[3 * i + 2].y,mesh.m_vertices[3 * i + 2].z };
			glVertex3fv(v3);
			glEnd();
		}*/
	}
	glEnable(GL_LIGHTING);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glPopMatrix();
}



