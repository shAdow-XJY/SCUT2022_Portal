#include "../Scene/SceneBasic.h"


//ÇòÌå»ùÀà
class Sphere:public GameSceneBasic{
protected:
	PxReal halfExtend = 0;
public:
	Sphere() {};
	Sphere(string name, PxVec3 position, PxReal halfExtend, OrganType type = OrganType::sphere)
	:GameSceneBasic(name,position,type) {
		this->halfExtend = halfExtend;
	}

	PxReal getHalfExtend() {
		return this->halfExtend;
	}

	void setHalfExtend(PxReal halfExtend) {
		this->halfExtend = halfExtend;
	}


};

