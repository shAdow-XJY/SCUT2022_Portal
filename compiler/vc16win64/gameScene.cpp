#include <ctype.h>
#include "PxPhysicsAPI.h"
#include<vector>

using namespace physx;

extern PxScene* gScene;
extern PxMaterial* gMaterial;
extern PxPhysics* gPhysics;
float maxJumpHeight = 1.0;
float stepHeight = 0.8 * maxJumpHeight;

void createBox(const PxTransform& t, const PxVec3& v, PxReal x, PxReal y, PxReal z) {
	PxTransform local(v);
	PxShape* shape = gPhysics->createShape(PxBoxGeometry(x, y, z), *gMaterial);
	//setupFiltering(shape, FilterGroup::ePIG, FilterGroup::eBIRD);
	PxRigidStatic* sceneBox = gPhysics->createRigidStatic(t.transform(local));
	sceneBox->attachShape(*shape);
	gScene->addActor(*sceneBox);
}

void createPlane(const PxVec3& point, const PxVec3& normal) {
	/*PxTransformFromPlaneEquation()将平面方程转换成等价的transform。
	PxPlaneEquationFromTransform()提供相反的转换。
	PxPlaneGeometry没有attribute，因为形状的位置完全定义了平面的碰撞体积。
	PxPlaneGeometry的shape只能为静态actor创建。*/
	PxPlane p(point, normal);
	PxShape* planeShape = gPhysics->createShape(PxPlaneGeometry(), *gMaterial);
	PxRigidStatic* plane = gPhysics->createRigidStatic(PxTransformFromPlaneEquation(p));
	plane->attachShape(*planeShape);
	gScene->addActor(*plane);
	
}

float center_y(float y) {
	return y + 2 * stepHeight;
}

void createGameScene(const PxTransform& t) {
	float r_1_l = 4.0;  //road_1_length
	float r_1_w = 8.0;  //road_1_width
	float c_1_y = stepHeight;  //the position of the center of road_1
	// create road_1
	createBox(t, PxVec3(0, c_1_y, 0), r_1_l, stepHeight, r_1_w);

	float r_2_w = 2.0;
	float r_2_l = 20.0;
	float c_2_x = r_2_l - r_1_l;
	float c_2_y = center_y(c_1_y);
	float c_2_z = r_1_l + r_2_w;
	createBox(t, PxVec3(c_2_x, c_2_y, c_2_z), r_2_l , stepHeight, r_2_w);

	//stairs
	float stairsWidth = 2.0;
	float stairsLength = 1.0;
	float center_x = stairsLength + r_2_l + c_2_x;
	float centerHeight = center_y(c_2_y);
	float center_z = c_2_z;
	
	for (int i = 0; i <= 5; i++) {
		createBox(t, PxVec3(center_x, centerHeight, center_z), stairsLength, stepHeight, stairsWidth);
		centerHeight = center_y(centerHeight);
		center_x += 2 * stairsLength;
	}

	createPlane(PxVec3(0, 0, 0), PxVec3(0, 1, 0));
	//createPlane(PxVec3(0, 100, 0), PxVec3(0, 1, 0));
}