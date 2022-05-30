#include <ctype.h>
#include "PxPhysicsAPI.h"
using namespace physx;

extern PxScene* gScene;
extern PxMaterial* gMaterial;
extern PxPhysics* gPhysics;

class Role {
private:
	PxRigidBody* role;

public:
	Role() {
		PxShape* shape = gPhysics->createShape(PxCapsuleGeometry(2, 4), *gMaterial);		
		PxTransform relativePose(PxQuat(PxHalfPi, PxVec3(0, 0, 1)));
		shape->setLocalPose(relativePose);
		role = gPhysics->createRigidDynamic(PxTransform(PxVec3(0, 0, 0)));
		role->setName("role");
		role->attachShape(*shape);
		gScene->addActor(*role);
	};
	~Role();
	void setRolePosition(PxVec3 position);

};
