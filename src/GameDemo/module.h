#pragma once
#include <ctype.h>

#include "PxPhysicsAPI.h"
#include "../Common/Print.h"
#include "../Common/PVD.h"
#include "../Utils/Utils.h"
#include "PxSimulationEventCallback.h"
#define PI 3.1415926

using namespace physx;

class module :public PxSimulationEventCallback {
public:
	module();
	module(PxTransform o, PxShape* s, PxTransform l, PxRigidActor* b0);
	virtual ~module();

	// Implements PxSimulationEventCallback
	virtual void							onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs);
	virtual void							onTrigger(PxTriggerPair* pairs, PxU32 count);
	virtual void							onConstraintBreak(PxConstraintInfo*, PxU32) {}
	virtual void							onWake(PxActor**, PxU32) {}
	virtual void							onSleep(PxActor**, PxU32) {}
	virtual void							onAdvance(const PxRigidBody*const*, const PxTransform*, const PxU32) {}


	PxTransform ori;//����ϵԭ��
	PxShape* shape;//��״
	PxTransform local;//����������ϵ�е����λ��
	PxRigidActor* body0;//����trigger���ĸ�������0��1
	//PxRigidActor* body1;
};