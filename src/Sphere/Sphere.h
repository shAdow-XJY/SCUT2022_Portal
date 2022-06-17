#include <ctype.h>
#include <string>
#include "PxPhysicsAPI.h"

using namespace physx;
using namespace std;

enum SphereType
{
	sphere = 0, //普通球体
	pendulum = 1, //摆锤
};


//球体基类
class Sphere {
protected:
	PxVec3 position;
	PxReal halfExtend = 0;
	SphereType type = SphereType::sphere;
	string name = "";
public:
	Sphere() {};
	Sphere(string name, PxVec3 position, PxReal halfExtend, SphereType type = SphereType::sphere)
		:name(name), position(position), halfExtend(halfExtend), type(type) {}


	PxVec3 getPosition() {
		return this->position;
	}

	PxReal getHalfExtend() {
		return this->halfExtend;
	}


	SphereType getType() {
		return this->type;
	}

	string getName() {
		return this->name;
	}

	void setPosition(PxVec3 position) {
		this->position = position;
	}

	void setHalfExtend(PxReal halfExtend) {
		this->halfExtend = halfExtend;
	}

	void setType(SphereType type) {
		this->type = type;
	}

	void setName(string name) {
		this->name = name;
	}
};

