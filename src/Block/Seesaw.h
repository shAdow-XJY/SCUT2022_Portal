#include "Block.h"
class Seesaw :public Block {

private:
	PxRigidDynamic* seesaw = NULL;

public:
	Seesaw(){

	}
	Seesaw(std::string name, PxVec3 position, PxReal halfX, PxReal halfY, PxReal halfZ)
		:Block(name, position, halfX, halfY, halfZ, BlockType::seesaw) {
	}
	Seesaw(std::string name, PxVec3 position, PxReal halfX, PxReal halfY, PxReal halfZ, PxRigidDynamic* seesaw)
		:Block(name, position, halfX, halfY, halfZ, BlockType::seesaw) {
		this->seesaw = seesaw;
	}
	void attachSeesawActor(PxRigidDynamic* road);
	PxRigidDynamic* getSeesawActor();
};