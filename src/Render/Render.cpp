#include "Render.h"
#include<time.h>
#include <iostream>
#include "BMPLoader.h"
#include <string>
#include <map>
#include <Block/Block.h>
#include <Render/DynamicBall.h>
using namespace std;
#include "../LoadModel/Model.h"
#include "../Render/Camera.h"
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
extern Role* role;
//材质贴图ID数组
extern std::map<string, unsigned int> textureMap;
unsigned int textureID;
//动态渲染圈
extern DynamicBall dynamicBall;

bool helpMenu = false;

extern void reshape(int width, int height);
extern Snippets::Camera* sCamera;
extern ImFont* emojiFont;
extern ImFont* titleFont;

namespace Callbacks {
	extern void idleCallback();
	extern void renderCallback();
	extern void keyboardUpCallback(unsigned char key, int x, int y);
	extern void specialKeyDownCallback(GLint key, GLint x, GLint y);
	extern void specialKeyUpCallback(GLint key, GLint x, GLint y);
	extern void mouseCallback(int button, int state, int x, int y);
	extern void idleCallback();
	extern void animationRenderCallback();
	extern void renderCallback();
	extern void exitCallback(void);
	extern void keyboardDownCallback(unsigned char key, int x, int y);
	extern void motionCallback(int x, int y);
}


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
//关卡数
int* checkpoint = new int[1]{ 0 };

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
		if (name == "Particle") {
			//glColor4f(1.f, 1.f, 1.f, 0.5);
			float black[3] = { 0, 0, 0 };
			glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, black);
			glEnable(GL_COLOR_MATERIAL);
			glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
		}
		else
		{
			glColor4f(0.102f, 0.102f, 0.102f, 1);
		}
			
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

//extern void reshape(int width, int height);

bool defaultWindow = false;
bool fps = false;
bool welcome = true;

namespace Snippets
{
	// 点击开始游戏后再进行键鼠回调函数的注册
	void installPlayFunc() {
		glutKeyboardFunc(Callbacks::keyboardDownCallback);
		glutKeyboardUpFunc(Callbacks::keyboardUpCallback);
		glutSpecialFunc(Callbacks::specialKeyDownCallback);
		glutSpecialUpFunc(Callbacks::specialKeyUpCallback);
		//glutMouseFunc(Callbacks::mouseCallback);
		glutMotionFunc(Callbacks::motionCallback);
		glutReshapeFunc(reshape);

	}

	void textCentered(string text) {
		auto windowWidth = ImGui::GetWindowSize().x;
		auto textWidth = ImGui::CalcTextSize(text.c_str()).x;

		ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
		ImGui::Text(text.c_str());
	}

	void menuButtonCentered2(string button1, string button2) {
		float windowWidth = ImGui::GetWindowSize().x;
		float button1Width = ImGui::CalcTextSize(button1.c_str()).x;
		float button2Width = ImGui::CalcTextSize(button2.c_str()).x;
		
		//float space = (windowWidth - button1Width - button2Width) / 3.f;
		ImGui::SetCursorPosX((windowWidth - button1Width) * 0.5f - 30);
		ImGui::PushFont(titleFont);
		if (ImGui::Button(button1.c_str(),ImVec2(90,59))) {
			welcome = false;
			defaultWindow = true;
			fps = true;
			installPlayFunc();
		}
		ImGui::NewLine();
		ImGui::SetCursorPosX((windowWidth - button1Width) * 0.5f - 30);
		
		if (ImGui::Button(button2.c_str(), ImVec2(90, 59))) {
			exit(0);
		}
		ImGui::PopFont();
	}

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
		// Setup lighting 光照设置
		glEnable(GL_LIGHTING);
		PxReal ambientColor1[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		PxReal diffuseColor1[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		PxReal specularColor1[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		PxReal position1[] = { -137.841f, 155.0f,228.381f, 1.0f };
		glLightfv(GL_LIGHT1, GL_AMBIENT, ambientColor1);
		glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuseColor1);
		glLightfv(GL_LIGHT1, GL_SPECULAR, specularColor1);
		glLightfv(GL_LIGHT1, GL_POSITION, position1);
		glEnable(GL_LIGHT1);
		//X:-137.841 Y : 149.557 Z : 228.381

		/** 启用纹理 */
		glEnable(GL_TEXTURE_2D);
		//模型纹理贴图内嵌，开启法向量
		glEnable(GL_NORMALIZE);
		glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
		glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	}
	
	

	void renderImGui() {
		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoBackground;
		window_flags |= ImGuiWindowFlags_NoTitleBar;
		window_flags |= ImGuiWindowFlags_NoResize;
	
		if (welcome) {
			ImGui::Begin(" ", &welcome, window_flags - 128);
			ImGui::NewLine();
			ImGui::NewLine();
			ImGui::PushFont(titleFont);
			textCentered("Welcome to Portal's game");
			ImGui::PopFont();
			ImGui::NewLine();
			ImGui::NewLine();
			ImGui::NewLine();
			ImGui::NewLine();

			menuButtonCentered2("Play", "Exit");
				
			ImGui::End();
		}


		if (defaultWindow) {
			//ImGui::ShowDemoWindow(&demo);

			ImGui::Begin("Monitoring", &defaultWindow, window_flags);

			//ImGui::Text("ImGui successfully deployed.");           

			//ImGui::SameLine();
			
			auto count = role->getHealth();
			while (count >= 0 && !role->isOver()) {
				ImGui::Text(u8"♥");
				ImGui::SameLine();
				--count;
			}
			ImGui::NewLine();
			ImGui::NewLine();
			for (int i = 0; i < role->getCheckpoint(); ++i) {
				ImGui::Text(u8"⏩");
				ImGui::SameLine();
			}
			ImGui::NewLine();
			ImGui::Text("Score: %d", role->getScore());
			ImGui::Text("Press H to open help menu");
			if (!sCamera->isFree()) {
				ImGui::PushFont(emojiFont);
				ImGui::Text("Camera is locked");
				ImGui::PopFont();
			}
			ImGui::End();
		}
		
		if(helpMenu)
		{
			ImGuiWindowFlags flags = 0;
			flags |= ImGuiWindowFlags_NoTitleBar;
			flags |= ImGuiWindowFlags_NoBackground;
			flags |= ImGuiWindowFlags_NoResize;

			ImGui::Begin("Play Guide", &helpMenu, flags - 128);
			ImGui::Text("Arraw keys: Move the player");
			ImGui::Text("Hold/Release space: Charge/Jump");
			ImGui::Text("T: Unlock/Lock camera");
			ImGui::Text("WASD: Free camera");
			ImGui::End();

			ImGui::Begin("Control the character and head to the end!", &helpMenu, flags-128);
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 0, 255, 255));
			ImGui::Text("Control the character and head to the end!");
			ImGui::PopStyleColor();
			ImGui::End();

			ImGui::Begin("help", &helpMenu,flags);
			ImGui::Text(u8"← Chances to go");
			ImGui::NewLine();
			ImGui::Text(u8"← Checkpoints you've achieved");
			ImGui::End();

		}
			
	
		if (fps) {
			ImGui::Begin("FPS check", &fps, window_flags);
			ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
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
		//gluLookAt(-0.5, 600.5, 1300.5, -0.5, 200, 1.5, 0, 1, 0);
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

		PxVec3 actorWorldPosition = actors[i]->getGlobalPose().p;
		if (nbShapes > 0 && !dynamicBall.isInCircle(actorWorldPosition.x, actorWorldPosition.z)) {
			continue;
		}
		

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
				glEnable(GL_CULL_FACE);
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

				if (shapes[j]->getFlags() & PxShapeFlag::eVISUALIZATION && shadows)/*阴影*/
				{
					const PxVec3 shadowDir(0.0f, -0.7071067f, -0.7071067f);
					const PxReal shadowMat[] = { 1,0,0,0, -shadowDir.x / shadowDir.y,0,-shadowDir.z / shadowDir.y,0, 0,0,1,0, 0,0,0,1 };
					glPushMatrix();
					glMultMatrixf(shadowMat);
					glMultMatrixf(reinterpret_cast<const float*>(&shapePose));
					//开启表⾯剔除(默认背⾯剔除)
					glEnable(GL_CULL_FACE);
					glDisable(GL_LIGHTING);
					glColor4f(0.1f, 0.2f, 0.3f, 1.0f);
					renderGeometry(h, "Block",true);
					glDisable(GL_CULL_FACE);
					glEnable(GL_LIGHTING);
					glPopMatrix();
				}	
			}

		}
	}

	void finishRender()
	{
		 //把ui渲染放在这里 是为了其能显示在最上层
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
	bool die = true;
	ImGui::PushFont(titleFont);
	ImGui::Begin("gameover", &die, 3);

	ImGui::Text("Game Over.");
	ImGui::End();
	ImGui::PopFont();
	//int len = sizeof(msg) / sizeof(char);
	//Snippets::renderText(45, 50, msg, 10);
}
