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

#include "Render.h"
#include<time.h>
#include <iostream>
#include "BMPLoader.h"
#include <string>
#include <map>
#include <Block/Block.h>
using namespace std;
using namespace physx;

static float gCylinderData[]={
	1.0f,0.0f,1.0f,1.0f,0.0f,1.0f,1.0f,0.0f,0.0f,1.0f,0.0f,0.0f,
	0.866025f,0.500000f,1.0f,0.866025f,0.500000f,1.0f,0.866025f,0.500000f,0.0f,0.866025f,0.500000f,0.0f,
	0.500000f,0.866025f,1.0f,0.500000f,0.866025f,1.0f,0.500000f,0.866025f,0.0f,0.500000f,0.866025f,0.0f,
	-0.0f,1.0f,1.0f,-0.0f,1.0f,1.0f,-0.0f,1.0f,0.0f,-0.0f,1.0f,0.0f,
	-0.500000f,0.866025f,1.0f,-0.500000f,0.866025f,1.0f,-0.500000f,0.866025f,0.0f,-0.500000f,0.866025f,0.0f,
	-0.866025f,0.500000f,1.0f,-0.866025f,0.500000f,1.0f,-0.866025f,0.500000f,0.0f,-0.866025f,0.500000f,0.0f,
	-1.0f,-0.0f,1.0f,-1.0f,-0.0f,1.0f,-1.0f,-0.0f,0.0f,-1.0f,-0.0f,0.0f,
	-0.866025f,-0.500000f,1.0f,-0.866025f,-0.500000f,1.0f,-0.866025f,-0.500000f,0.0f,-0.866025f,-0.500000f,0.0f,
	-0.500000f,-0.866025f,1.0f,-0.500000f,-0.866025f,1.0f,-0.500000f,-0.866025f,0.0f,-0.500000f,-0.866025f,0.0f,
	0.0f,-1.0f,1.0f,0.0f,-1.0f,1.0f,0.0f,-1.0f,0.0f,0.0f,-1.0f,0.0f,
	0.500000f,-0.866025f,1.0f,0.500000f,-0.866025f,1.0f,0.500000f,-0.866025f,0.0f,0.500000f,-0.866025f,0.0f,
	0.866026f,-0.500000f,1.0f,0.866026f,-0.500000f,1.0f,0.866026f,-0.500000f,0.0f,0.866026f,-0.500000f,0.0f,
	1.0f,0.0f,1.0f,1.0f,0.0f,1.0f,1.0f,0.0f,0.0f,1.0f,0.0f,0.0f
};

static bool show_demo_window = true;
static bool show_another_window = false;
static ImVec4 clear_color = ImVec4(.45f, .55f, .6f, 1.0f);
static clock_t lastTime, currTime;
double rasterTime;
static bool textShouldRaster = false;

// 此处原先定义为1024， 而TriangleMesh中一个三角形就要占用2个Vec3，复杂模型根本不够用
#define MAX_NUM_MESH_VEC3S  524288

static PxVec3 gVertexBuffer[MAX_NUM_MESH_VEC3S];
extern bool press;
extern int mouseX;
extern int mouseY;
extern int textX, textY;


extern std::map<string, unsigned int> textureMap;
CBMPLoader* TextureLoader;
unsigned int textureID;

static void drawBox(GLfloat x, GLfloat y, GLfloat z,bool shadow)
{
	static GLfloat n[6][3] =
	{
	  {-1.0, 0.0, 0.0},
	  {0.0, 1.0, 0.0},
	  {1.0, 0.0, 0.0},
	  {0.0, -1.0, 0.0},
	  {0.0, 0.0, 1.0},
	  {0.0, 0.0, -1.0}
	};
	static GLint faces[6][4] =
	{
	  {0, 1, 2, 3},
	  {3, 2, 6, 7},
	  {7, 6, 5, 4},
	  {4, 5, 1, 0},
	  {5, 6, 2, 1},
	  {7, 4, 0, 3}
	};
	GLfloat v[8][3];
	GLint i;

	v[0][0] = v[1][0] = v[2][0] = v[3][0] = -x / 2;
	v[4][0] = v[5][0] = v[6][0] = v[7][0] = x / 2;
	v[0][1] = v[1][1] = v[4][1] = v[5][1] = -y / 2;
	v[2][1] = v[3][1] = v[6][1] = v[7][1] = y / 2;
	v[0][2] = v[3][2] = v[4][2] = v[7][2] = -z / 2;
	v[1][2] = v[2][2] = v[5][2] = v[6][2] = z / 2;
	/** 启用纹理 */
	glEnable(GL_TEXTURE_2D);


	for (i = 5; i >= 0; i--) {
		/** 开始绘制 */
		glPushMatrix();
		if (shadow) {
			glColor4f(0.1f, 0.2f, 0.3f, 1.0f);
		}
		else
		{
			glColor4f(1, 1, 1, 1);
		}

		/** 绘制背面 */
		glBindTexture(GL_TEXTURE_2D, textureID);
		//std::cout << TextureLoader->ID << std::endl;
		glBegin(GL_QUADS);
		glNormal3fv(&n[i][0]);
		/** 指定纹理坐标和顶点坐标 */
		glTexCoord2f(1.0f, 0.0f); glVertex3fv(&v[faces[i][0]][0]);
		glTexCoord2f(1.0f, 1.0f); glVertex3fv(&v[faces[i][1]][0]);
		glTexCoord2f(0.0f, 1.0f); glVertex3fv(&v[faces[i][2]][0]);
		glTexCoord2f(0.0f, 0.0f); glVertex3fv(&v[faces[i][3]][0]);
		
		
		glEnd();
	}
	glPopMatrix();                 /** 绘制结束 */
	glFlush();
}


void renderGeometry(const PxGeometryHolder& h, string name,bool shadow)
{
	switch(h.getType())
	{
	case PxGeometryType::eBOX:			
		{
	
			textureID = textureMap[name];
			drawBox(h.box().halfExtents.x*2, h.box().halfExtents.y*2, h.box().halfExtents.z*2,shadow);
		}
		break;
	case PxGeometryType::eSPHERE:		
		{
			glutSolidSphere(GLdouble(h.sphere().radius), 10, 10);
		}
		break;
	case PxGeometryType::eCAPSULE:
		{

			const PxF32 radius = h.capsule().radius;
			const PxF32 halfHeight = h.capsule().halfHeight;
			glColor4f(0.0f, 1.0f, 0.0f, 0.0f);
			//Sphere
			glPushMatrix();
			glTranslatef(halfHeight, 0.0f, 0.0f);
			glScalef(radius,radius,radius);
			glutSolidSphere(1, 10, 10);		
			glPopMatrix();

			//Sphere
			glPushMatrix();
			glTranslatef(-halfHeight, 0.0f, 0.0f);
			glScalef(radius,radius,radius);
			glutSolidSphere(1, 10, 10);		
			glPopMatrix();

			//Cylinder
			glPushMatrix();
			glTranslatef(-halfHeight, 0.0f, 0.0f);
			glScalef(2.0f*halfHeight, radius,radius);
			glRotatef(90.0f,0.0f,1.0f,0.0f);
			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_NORMAL_ARRAY);
			glVertexPointer(3, GL_FLOAT, 2*3*sizeof(float), gCylinderData);
			glNormalPointer(GL_FLOAT, 2*3*sizeof(float), gCylinderData+3);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 13*2);
			glDisableClientState(GL_VERTEX_ARRAY);
			glDisableClientState(GL_NORMAL_ARRAY);
			glPopMatrix();
		}
		break;
	case PxGeometryType::eCONVEXMESH:
		{

			//Compute triangles for each polygon.
			const PxVec3 scale = h.convexMesh().scale.scale;
			PxConvexMesh* mesh = h.convexMesh().convexMesh;
			const PxU32 nbPolys = mesh->getNbPolygons();
			const PxU8* polygons = mesh->getIndexBuffer();
			const PxVec3* verts = mesh->getVertices();
			PxU32 nbVerts = mesh->getNbVertices();
			PX_UNUSED(nbVerts);

			PxU32 numTotalTriangles = 0;
			for(PxU32 i = 0; i < nbPolys; i++)
			{
				PxHullPolygon data;
				mesh->getPolygonData(i, data);

				const PxU32 nbTris = PxU32(data.mNbVerts - 2);
				const PxU8 vref0 = polygons[data.mIndexBase + 0];
				PX_ASSERT(vref0 < nbVerts);
				for(PxU32 j=0;j<nbTris;j++)
				{
					const PxU32 vref1 = polygons[data.mIndexBase + 0 + j + 1];
					const PxU32 vref2 = polygons[data.mIndexBase + 0 + j + 2];

					//generate face normal:
					PxVec3 e0 = verts[vref1] - verts[vref0];
					PxVec3 e1 = verts[vref2] - verts[vref0];

					PX_ASSERT(vref1 < nbVerts);
					PX_ASSERT(vref2 < nbVerts);

					PxVec3 fnormal = e0.cross(e1);
					fnormal.normalize();
			
					if(numTotalTriangles*6 < MAX_NUM_MESH_VEC3S)
					{
						gVertexBuffer[numTotalTriangles*6 + 0] = fnormal;
						gVertexBuffer[numTotalTriangles*6 + 1] = verts[vref0];
						gVertexBuffer[numTotalTriangles*6 + 2] = fnormal;
						gVertexBuffer[numTotalTriangles*6 + 3] = verts[vref1];
						gVertexBuffer[numTotalTriangles*6 + 4] = fnormal;
						gVertexBuffer[numTotalTriangles*6 + 5] = verts[vref2];
						numTotalTriangles++;
					}
				}
			}
			glPushMatrix();
			glScalef(scale.x, scale.y, scale.z);
			glEnableClientState(GL_NORMAL_ARRAY);
			glEnableClientState(GL_VERTEX_ARRAY);
			glNormalPointer(GL_FLOAT, 2*3*sizeof(float), gVertexBuffer);
			glVertexPointer(3, GL_FLOAT, 2*3*sizeof(float), gVertexBuffer+1);
			glDrawArrays(GL_TRIANGLES, 0, int(numTotalTriangles * 3));
			glPopMatrix();
		}
		break;
	case PxGeometryType::eTRIANGLEMESH:
		{
			const PxTriangleMeshGeometry& triGeom = h.triangleMesh();
			const PxTriangleMesh& mesh = *triGeom.triangleMesh;
			const PxVec3 scale = triGeom.scale.scale;


			const PxU32 triangleCount = mesh.getNbTriangles();
			const PxU32 has16BitIndices = mesh.getTriangleMeshFlags() & PxTriangleMeshFlag::e16_BIT_INDICES;
			const void* indexBuffer = mesh.getTriangles();

			const PxVec3* vertexBuffer = mesh.getVertices();

			const PxU32* intIndices = reinterpret_cast<const PxU32*>(indexBuffer);
			const PxU16* shortIndices = reinterpret_cast<const PxU16*>(indexBuffer);
			PxU32 numTotalTriangles = 0;
			for(PxU32 i=0; i < triangleCount; ++i)
			{
				PxVec3 triVert[3];

				if(has16BitIndices)
				{
					triVert[0] = vertexBuffer[*shortIndices++];
					triVert[1] = vertexBuffer[*shortIndices++];
					triVert[2] = vertexBuffer[*shortIndices++];
				}
				else
				{
					triVert[0] = vertexBuffer[*intIndices++];
					triVert[1] = vertexBuffer[*intIndices++];
					triVert[2] = vertexBuffer[*intIndices++];
				}

				PxVec3 fnormal = (triVert[1] - triVert[0]).cross(triVert[2] - triVert[0]);
				fnormal.normalize();

				if(numTotalTriangles*6 < MAX_NUM_MESH_VEC3S)
				{
					gVertexBuffer[numTotalTriangles*6 + 0] = fnormal;
					gVertexBuffer[numTotalTriangles*6 + 1] = triVert[0];
					gVertexBuffer[numTotalTriangles*6 + 2] = fnormal;
					gVertexBuffer[numTotalTriangles*6 + 3] = triVert[1];
					gVertexBuffer[numTotalTriangles*6 + 4] = fnormal;
					gVertexBuffer[numTotalTriangles*6 + 5] = triVert[2];
					numTotalTriangles++;
				}
			}
			glPushMatrix();
			glScalef(scale.x, scale.y, scale.z);
			glEnableClientState(GL_NORMAL_ARRAY);
			glEnableClientState(GL_VERTEX_ARRAY);
			glNormalPointer(GL_FLOAT, 2*3*sizeof(float), gVertexBuffer);
			glVertexPointer(3, GL_FLOAT, 2*3*sizeof(float), gVertexBuffer+1);
			glDrawArrays(GL_TRIANGLES, 0, int(numTotalTriangles * 3));
			glDisableClientState(GL_VERTEX_ARRAY);
			glDisableClientState(GL_NORMAL_ARRAY);
			glPopMatrix();
		}
		break;
	case PxGeometryType::eINVALID:
	case PxGeometryType::eHEIGHTFIELD:
	case PxGeometryType::eGEOMETRY_COUNT:	
	case PxGeometryType::ePLANE:/*这里设置了平面，给了几个点让glut画了个正方形，颜色设置为120的绿色（满值是255）*/
		glBegin(GL_QUADS);
		glColor4ub(0, 159, 149, 200); glVertex3f(-1.0f, -300.0f, -300.0f);/*这个坐标有点奇怪 后面应该是变换过，不用太在意，反正就当第一个是y轴就可以了*/
		glColor4ub(0, 159, 149, 200); glVertex3f(-1.0f, -300.0f, 300.0f);
		glColor4ub(0, 159, 149, 200); glVertex3f(-1.0f, 300.0f, 300.0f);
		glColor4ub(0, 159, 149, 200); glVertex3f(-1.0f, 300.0f, -300.0f);
		glEnd();
		break;
	}
}

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

extern void reshape(int width, int height);

namespace Snippets
{


void setupDefaultWindow(const char *name)
{
	char* namestr = new char[strlen(name)+1];
	strcpy(namestr, name);
	int argc = 1;
	char* argv[1] = { namestr };

	glutInit(&argc, argv);
	
	glutInitWindowSize(1024, 768);
	glutInitDisplayMode(GLUT_RGB|GLUT_DOUBLE|GLUT_DEPTH);
	int mainHandle = glutCreateWindow(name);
	glutSetWindow(mainHandle);
	glutReshapeFunc(reshape);
	
	delete[] namestr;
}

void setupDefaultRenderState()
{
	// Setup default render states 初始化设置
	glClearColor(0.7f, 0.9f, 0.86f, 1.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
	// Setup lighting 光照设置
	glEnable(GL_LIGHTING);
	PxReal ambientColor[]	= { 0.0f, 0.1f, 0.2f, 0.0f };
	PxReal diffuseColor[]	= { 1.0f, 1.0f, 1.0f, 0.0f };		
	PxReal specularColor[]	= { 0.0f, 0.0f, 0.0f, 0.0f };		
	PxReal position[]		= { 50.0f, 50.0f, 200.0f, 1.0f };		
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientColor);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseColor);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specularColor);
	glLightfv(GL_LIGHT0, GL_POSITION, position);
	glEnable(GL_LIGHT0);
	/** 启用纹理 */
	glEnable(GL_TEXTURE_2D);
	
}

void renderImGui() {

	{
		static float f = 0.0f;
		static int counter = 0;

		ImGui::Begin("Monitoring");                         

		//ImGui::Text("ImGui successfully deployed.");           

		//ImGui::SameLine();
		ImGui::Text("clickX = %d", textX);
		ImGui::Text("clickY = %d", textY);

		ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
		ImGui::End();
	}

	// 3. Show another simple window.
	if (show_another_window)
	{
		ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
		ImGui::Text("Hello from another window!");
		if (ImGui::Button("Close Me"))
			show_another_window = false;
		ImGui::End();
	}

	ImGui::Render();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
}

void startRender(const PxVec3& cameraEye, const PxVec3& cameraDir, PxReal clipNear, PxReal clipFar)
{
	ImGui_ImplOpenGL2_NewFrame();
	ImGui_ImplGLUT_NewFrame();
	lastTime = currTime;
	currTime = clock();
	 
	if (press == true) {
		textShouldRaster = true;
		lastTime = currTime;
		rasterTime = (double)0.;
		press = false;
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Setup camera
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, GLdouble(glutGet(GLUT_WINDOW_WIDTH)) / GLdouble(glutGet(GLUT_WINDOW_HEIGHT)), GLdouble(clipNear), GLdouble(clipFar));

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(GLdouble(cameraEye.x), GLdouble(cameraEye.y), GLdouble(cameraEye.z), GLdouble(cameraEye.x + cameraDir.x), GLdouble(cameraEye.y + cameraDir.y), GLdouble(cameraEye.z + cameraDir.z), 0.0, 1.0, 0.0);

	glColor4f(0.4f, 0.4f, 0.4f, 1.0f);
}

void renderGameOver(const char text[], int len) 
{
	renderText(190,250,text,len);
}

void renderActors(PxRigidActor** actors, const PxU32 numActors, bool shadows, const PxVec3 & color)/*渲染actor*/
{
	PxShape* shapes[MAX_NUM_ACTOR_SHAPES];
	for(PxU32 i=0;i<numActors;i++)
	{
		const PxU32 nbShapes = actors[i]->getNbShapes();
		PX_ASSERT(nbShapes <= MAX_NUM_ACTOR_SHAPES);
		actors[i]->getShapes(shapes, nbShapes);
		bool sleeping = actors[i]->is<PxRigidDynamic>() ? actors[i]->is<PxRigidDynamic>()->isSleeping() : false; 
		/*这里以后会用到来判断每一杆的行动机会，sleeping则可以行动，不是sleeping状态说明还在运动*/

		for(PxU32 j=0;j<nbShapes;j++)
		{
			const PxMat44 shapePose(PxShapeExt::getGlobalPose(*shapes[j], *actors[i]));
			PxGeometryHolder h = shapes[j]->getGeometry();

			if (shapes[j]->getFlags() & PxShapeFlag::eTRIGGER_SHAPE)
				glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
			if (shapes[j]->getFlags() & PxShapeFlag::eVISUALIZATION) {
				// render object
				glPushMatrix();
				glMultMatrixf(reinterpret_cast<const float*>(&shapePose));
				if (sleeping)/*是否处于sleeping状态*/
				{
					PxVec3 darkColor = color * 0.8f;/*sleeping的情况下颜色*0.8*/
					glColor4f(darkColor.x, darkColor.y, darkColor.z, 1.0f);
				}
				else {
					glColor4f(color.x, color.y, color.z, 1.0f);
				}
				if (actors[i]->getName()) {
					string name = string(actors[i]->getName());
					renderGeometry(h, name, false);
				}
				else
				{
					renderGeometry(h, "Block", false);
				}			

				glPopMatrix();
			}
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			if (shadows)/*阴影，，，效果表现上有瑕疵但不知道怎么优化*/
			{
				const PxVec3 shadowDir(0.0f, -0.7071067f, -0.7071067f);
				const PxReal shadowMat[] = { 1,0,0,0, -shadowDir.x / shadowDir.y,0,-shadowDir.z / shadowDir.y,0, 0,0,1,0, 0,0,0,1 };
				glPushMatrix();
				glMultMatrixf(shadowMat);
				glMultMatrixf(reinterpret_cast<const float*>(&shapePose));
				glDisable(GL_LIGHTING);
				glColor4f(0.1f, 0.2f, 0.3f, 1.0f);
				renderGeometry(h, "Block",true);
				glEnable(GL_LIGHTING);
				glPopMatrix();
			}	
		}
	}
}

void finishRender()
{
	// 把ui渲染放在这里 是为了其能显示在最上层
	renderImGui();
	if (textShouldRaster == true) {
		rasterTime += double(currTime - lastTime)/CLOCKS_PER_SEC;
		if (rasterTime >= 3) {
			textShouldRaster = false;
		}
		else {
			// TextX的范围[0,90], textY的范围[0,100]
			// 这里的1024和768是窗口尺寸，后续应考虑换成变量表示
			textX = ((float)mouseX / 1024) * 90;
			textY= ((float)mouseY / 768) * 100;
			renderText(textX, 100-textY, "Test Text.", 10);
		}
	}
	
	glutSwapBuffers();
}

void renderText(int x, int y, const char text[], int len)
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, GLUT_WINDOW_WIDTH, 0, GLUT_WINDOW_HEIGHT);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glLoadIdentity();
	glRasterPos2i(x, y);
	for (int i = 0; i < len; i++) {
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, text[i]);

	}
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}
} //namespace Snippets


void renderGameOver() {
	const char* msg = "Game Over!";
	//int len = sizeof(msg) / sizeof(char);
	Snippets::renderText(45, 50, msg, 10);
}
