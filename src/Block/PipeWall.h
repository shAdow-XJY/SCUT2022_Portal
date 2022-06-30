#include "../Block/Block.h"
class PipeWall :public Block {

private:
	PxRigidStatic* pipeWall = NULL;

public:
	PipeWall() {

	}
	PipeWall(std::string name, PxVec3 position, PxReal halfX, PxReal halfY, PxReal halfZ)
		:Block(name, position, halfX, halfY, halfZ, OrganType::pipeWall) {
	}
	PipeWall(std::string name, PxVec3 position, PxReal halfX, PxReal halfY, PxReal halfZ, PxRigidStatic* pipeWall)
		:Block(name, position, halfX, halfY, halfZ, OrganType::pipeWall) {
		this->pipeWall = pipeWall;
	}
	PipeWall(std::string name, PxVec3 position, PxReal halfX, PxReal halfY, PxReal halfZ, PxRigidStatic* pipeWall, OrganType type)
		:Block(name, position, halfX, halfY, halfZ, OrganType::pipeWall) {
		this->pipeWall = pipeWall;
		this->type = type;
	}
	void attachPipeWallActor(PxRigidStatic* pipeWall);
	PxRigidStatic* getPipeWallActor();
};