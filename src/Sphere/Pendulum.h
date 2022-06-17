#include "Sphere.h"

class Pendulum :public Sphere {

private:
	PxRigidDynamic* pendulum = NULL;

public:
	Pendulum() {

	}
	Pendulum(string name, PxVec3 position, PxReal halfExtend)
		:Sphere(name, position, halfExtend, SphereType::pendulum) {
	}
	Pendulum(std::string name, PxVec3 position, PxReal halfExtend, PxRigidDynamic* pendulum)
		:Sphere(name, position, halfExtend, SphereType::pendulum) {
		this->pendulum = pendulum;
	}

	void attachPendulumActor(PxRigidDynamic* pendulum);
	PxRigidDynamic* getPendulumActor();
};
