#include "Block.h"
class Seesaw :public Block {

private:
	PxRigidDynamic* seesaw = NULL;
	PxRevoluteJoint* revorevolute = NULL; //����
	int tiltDir = 0; //��б���� 0��z�� 1��x��
	PxVec3 calTilt();

public:
	Seesaw(){

	}
	Seesaw(std::string name, PxVec3 position, PxReal halfX, PxReal halfY, PxReal halfZ)
		:Block(name, position, halfX, halfY, halfZ, OrganType::seesaw) {
	}
	Seesaw(std::string name, PxVec3 position, PxReal halfX, PxReal halfY, PxReal halfZ, PxRigidDynamic* seesaw, int tiltDir = 0)
		:Block(name, position, halfX, halfY, halfZ, OrganType::seesaw) {
		this->seesaw = seesaw;
		this->tiltDir = tiltDir;
	}
	void attachSeesawActor(PxRigidDynamic* road);
	void attachRevolute(PxRevoluteJoint* revorevolute);
	PxRigidDynamic* getSeesawActor();
	PxVec3 addGForce(PxVec3 position, PxVec3 force);
};