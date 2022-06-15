#include "Block.h"
class Seesaw :public Block {

private:
	PxRigidDynamic* seesaw = NULL;
	PxRevoluteJoint* revorevolute = NULL; //½ÂÁ´
	int tiltDir = 0; //ÇãÐ±·½Ïò 0£ºzÖá 1£ºxÖá
	PxVec3 calTilt();

public:
	Seesaw(){

	}
	Seesaw(std::string name, PxVec3 position, PxReal halfX, PxReal halfY, PxReal halfZ)
		:Block(name, position, halfX, halfY, halfZ, BlockType::seesaw) {
	}
	Seesaw(std::string name, PxVec3 position, PxReal halfX, PxReal halfY, PxReal halfZ, PxRigidDynamic* seesaw, int tiltDir = 0)
		:Block(name, position, halfX, halfY, halfZ, BlockType::seesaw) {
		this->seesaw = seesaw;
		this->tiltDir = tiltDir;
	}
	void attachSeesawActor(PxRigidDynamic* road);
	void attachRevolute(PxRevoluteJoint* revorevolute);
	PxRigidDynamic* getSeesawActor();
	PxVec3 addGForce(PxVec3 position, PxVec3 force);
};