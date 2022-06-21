#pragma once
#include "../Scene/SceneBasic.h"

//���ӵĸ߶�
extern float boxHeight;


//������࣬��������չʵ��
class Block:public GameSceneBasic {
protected:
	PxReal halfX = 0;
	PxReal halfY = 0;
	PxReal halfZ = 0;
public:
	Block() {};
	Block(string name, PxVec3 position, PxReal halfX, PxReal halfY, PxReal halfZ, OrganType type = OrganType::block)
	:GameSceneBasic(name,position,type){
		this->halfX = halfX;
		this->halfY = halfY;
		this->halfZ = halfZ;
	}
	PxReal getHalfX(){
		return this->halfX;
	}
	PxReal getHalfY() {
		return this->halfY;
	}
	PxReal getHalfZ() {
		return this->halfZ;
	}


	void setHalfX(PxReal halfX) {
		this->halfX = halfX;
	}

	void setHalfY(PxReal halfY) {
		this->halfY = halfY;
	}

	void setHalfZ(PxReal halfZ) {
		this->halfZ = halfZ;
	}
};
