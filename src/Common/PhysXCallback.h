#pragma once

#include "PxQueryReport.h"
#include "PxQueryFiltering.h"
#include "PxShape.h"
#include "PxRigidActor.h"
#include "PxFiltering.h"

//----------------------------------------------------------------------------------------------------------------------
// 这个保证与Morpheme系统保证兼容，只能做增量修改
//-----------------------------------------------------------------------------------------------------------------------
static physx::PxFilterFlags defaultPhysX3FilterShader(
	physx::PxFilterObjectAttributes attributes0,
	physx::PxFilterData filterData0,
	physx::PxFilterObjectAttributes attributes1,
	physx::PxFilterData filterData1,
	physx::PxPairFlags &pairFlags, 
	const void * constantBlock ,
	physx::PxU32 constantBlockSize)
{
// morphe me used filter callback
	bool kinematic0 = physx::PxFilterObjectIsKinematic(attributes0);
	bool kinematic1 = physx::PxFilterObjectIsKinematic(attributes1);
	if (kinematic0 && kinematic1)
	{
		pairFlags = physx::PxPairFlags(); 
		return physx::PxFilterFlag::eSUPPRESS;
	}

	// Support the idea of this being called from a user's filter shader - i.e. don't trample on existing flags
	//pairFlags = physx::PxPairFlag::eSOLVE_CONTACT
	//	| physx::PxPairFlag::eNOTIFY_TOUCH_FOUND
	//	| physx::PxPairFlag::eNOTIFY_CONTACT_POINTS;
	pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT;
	pairFlags = pairFlags | physx::PxPairFlag::eSOLVE_CONTACT
		| physx::PxPairFlag::eNOTIFY_TOUCH_FOUND
		| physx::PxPairFlag::eNOTIFY_CONTACT_POINTS;

	{
		// now test groups (word0) against ignore groups (word1)
		if ((filterData0.word0 & filterData1.word1) || (filterData1.word0 & filterData0.word1)) 
		{
			pairFlags = physx::PxPairFlags(); // disable all dynamic collisions
			return physx::PxFilterFlag::eSUPPRESS; // this stops the pair being actually passed into ray probe onObjectQuery
		}
		else
		{
			return physx::PxFilterFlag::eDEFAULT;
		}
	}
}
//class CollisionNotifyHandler;
//class PhysXCollisionEventCallback : public PxSimulationEventCallback
//{
//
//public:
//	PhysXCollisionEventCallback(ICollisionNotifyHandler* piCollisionEventsHandler);
//public:
//	virtual void							onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs);
//	virtual void							onTrigger(PxTriggerPair* pairs, PxU32 count);
//	virtual void							onConstraintBreak(PxConstraintInfo*, PxU32) {}
//	virtual void							onWake(PxActor** , PxU32 ) {}
//	virtual void							onSleep(PxActor** , PxU32 ){}
//private:
//	ICollisionNotifyHandler* m_piCollisionEventsHandler;
//};
//class PhysXRayCastCallback: public physx::PxRaycastBuffer
//{
//
//public:
//	PhysXRayCastCallback(physx::PxRaycastHit* aTouches = NULL, physx::PxU32 aMaxNbTouches = 0);
//public:
//	virtual physx::PxAgain processTouches(const physx::PxRaycastHit* buffer, physx::PxU32 nbHits);
//};
//
//class PhysXOverlapCallback: public physx::PxOverlapBuffer
//{
//public:
//	PhysXOverlapCallback(physx::PxOverlapHit* aTouches = NULL, physx::PxU32 aMaxNbTouches = 0);
//public:
//	virtual physx::PxAgain processTouches(const physx::PxOverlapHit* buffer, physx::PxU32 nbHits);
//};
//
//class PhysXSweepCallback: public physx::PxSweepBuffer
//{
//public:
//	PhysXSweepCallback(physx::PxSweepHit* aTouches = NULL, physx::PxU32 aMaxNbTouches = 0);
//public:
//	virtual physx::PxAgain processTouches(const physx::PxSweepHit* buffer, physx::PxU32 nbHits);
//	
//	float time;
//};
//
//class PhysXQueryFilterCallback : public physx::PxQueryFilterCallback
//{
//public:
//	PhysXQueryFilterCallback();
//	PhysXQueryFilterCallback(const physx::PxFilterData& queryFilterData, int32_t ignoreID = -1);
//	virtual ~PhysXQueryFilterCallback();
//
//protected:
//	physx::PxQueryHitType::Enum preFilter(
//		const physx::PxFilterData& filterData, 
//		const physx::PxShape* shape, 
//		const physx::PxRigidActor* actor, 
//		physx::PxHitFlags& queryFlags);
//	physx::PxQueryHitType::Enum postFilter(
//		const physx::PxFilterData& filterData, 
//		const physx::PxQueryHit& hit);
//protected:
//	physx::PxFilterData m_queryFilterData;
//	int32_t m_ignoreID;
//};