#include <ctype.h>
#include "PxPhysicsAPI.h"
using namespace physx;

extern PxScene* gScene;
extern PxMaterial* gMaterial;
extern PxPhysics* gPhysics;
extern PxControllerManager* cManager;
extern PxVec3 ScenetoWorld(int xCord, int yCord);

class Role {
private:
	PxRigidBody* role;
	PxController* roleController;
	bool isMoving = false;
	PxVec3 nowPostion;
	PxVec3 lastPostion;
public:
	/*Role() {
		PxShape* shape = gPhysics->createShape(PxCapsuleGeometry(2, 4), *gMaterial);		
		PxTransform relativePose(PxQuat(PxHalfPi, PxVec3(0, 0, 1)));
		shape->setLocalPose(relativePose);
		role = gPhysics->createRigidDynamic(PxTransform(PxVec3(0, 0, 0)));
		role->setName("role");
		role->attachShape(*shape);
		gScene->addActor(*role);
	};*/
	Role() {
		PxCapsuleControllerDesc desc;
		desc.radius = 1.0f;
		desc.height = 2.0f;
		desc.material = gMaterial;
		desc.climbingMode = PxCapsuleClimbingMode::eCONSTRAINED;
		roleController = cManager->createController(desc);
		roleController->setFootPosition(PxExtendedVec3(0, desc.contactOffset, 0));
		roleController->resize(desc.height + desc.radius);	
		this->role = roleController->getActor();
		this->nowPostion = this->role->getGlobalPose().p;
	}
	~Role() {
		this->role->release();
		this->roleController->release();
	};
	//void setRolePosition(PxVec3 position);
	void roleMoveByMouse(int x, int y);
	void move();
};

