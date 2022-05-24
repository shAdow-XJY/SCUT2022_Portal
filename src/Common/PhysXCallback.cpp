
#include "PxQueryReport.h"
#include "PxQueryFiltering.h"
#include "PxShape.h"
#include "PxSimulationEventCallback.h"
#include "PxRigidActor.h"
using namespace physx;
#include "PhysXCallback.h"

//void PhysXCollisionEventCallback::onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs)
//{
//	//const GPxActorUserData* pObject0 = (GPxActorUserData*)pairHeader.actors[0]->userData;
//	//const GPxActorUserData* pObject1 = (GPxActorUserData*)pairHeader.actors[1]->userData;	
//	//if(pObject0 && pObject1)
//	//{
//	//	for(unsigned i = 0; i != nbPairs; i++)
//	//	{
//	//		PxContactPairPoint contactPoints[16] = {};
//	//		int nContactPoints = pairs[i].extractContacts(contactPoints, 16);	
//	//		for( unsigned j = 0; j != nContactPoints; j++)
//	//		{
//	//			const PxContactPairPoint& Point = contactPoints[j];
//	//			const PxVec3 NormalImpulse = Point.impulse.dot(Point.normal) * Point.normal; // project impulse along normal
//	//			if(NormalImpulse.magnitude() > Phy_Global_Setting::fImpactThresholdWithContactCallback)
//	//			{
//	//				m_piCollisionEventsHandler->Push(CONTACT_INFO((void*)pObject0, 
//	//					(void*)pObject1, 
//	//					NormalImpulse.magnitude(),
//	//					ToGameData(Point.position)));
//	//			}
//	//			
//	//		}		
//	//	}	
//	//}	
//}
//
//void PhysXCollisionEventCallback::onTrigger(PxTriggerPair* pairs, PxU32 count)
//{
//
//}
//
//physx::PxAgain PhysXRayCastCallback::processTouches(const physx::PxRaycastHit* buffer, physx::PxU32 nbHits)
//{
//	return false;
//}
//
//PhysXRayCastCallback::PhysXRayCastCallback(physx::PxRaycastHit* aTouches, physx::PxU32 aMaxNbTouches):
//	physx::PxRaycastBuffer(aTouches, aMaxNbTouches)
//{
//
//}
//
//physx::PxAgain PhysXOverlapCallback::processTouches(const physx::PxOverlapHit* buffer, physx::PxU32 nbHits)
//{
//	
//	return false;
//}
//
//PhysXOverlapCallback::PhysXOverlapCallback(physx::PxOverlapHit* aTouches, physx::PxU32 aMaxNbTouches):
//	physx::PxOverlapBuffer(aTouches, aMaxNbTouches)
//{
//
//}
//
//physx::PxAgain PhysXSweepCallback::processTouches(const physx::PxSweepHit* buffer, physx::PxU32 nbHits)
//{
//	return false;
//}
//
//PhysXSweepCallback::PhysXSweepCallback(physx::PxSweepHit* aTouches, physx::PxU32 aMaxNbTouches):
//	physx::PxSweepBuffer(aTouches, aMaxNbTouches)
//	, time(0.f)
//{
//
//}
//
//PhysXQueryFilterCallback::PhysXQueryFilterCallback():
//m_ignoreID(-1)
//{
//
//}
//
//PhysXQueryFilterCallback::PhysXQueryFilterCallback(const physx::PxFilterData& queryFilterData, int32_t ignoreID /*= -1*/) :
//	m_queryFilterData(queryFilterData),
//	m_ignoreID(ignoreID)
//{
//
//}
//
//physx::PxQueryHitType::Enum PhysXQueryFilterCallback::preFilter(const physx::PxFilterData& filterData, 
//																const physx::PxShape* shape, 
//																const physx::PxRigidActor* actor, 
//																physx::PxHitFlags& queryFlags)
//{
//	physx::PxFilterData shapeFilterData = shape->getQueryFilterData();
//
//	if (m_ignoreID == (int)shapeFilterData.word2)
//	{
//		return physx::PxSceneQueryHitType::eNONE;
//	}
//
//	if ((shapeFilterData.word0 & m_queryFilterData.word1) || (m_queryFilterData.word0 & shapeFilterData.word1)) 
//	{
//		return physx::PxSceneQueryHitType::eNONE;
//	}
//	else
//	{
//		return physx::PxSceneQueryHitType::eBLOCK;
//	}
//}
//
//physx::PxQueryHitType::Enum PhysXQueryFilterCallback::postFilter(const physx::PxFilterData& filterData, const physx::PxQueryHit& hit)
//{
//	return physx::PxSceneQueryHitType::eNONE;
//}
//
//PhysXQueryFilterCallback::~PhysXQueryFilterCallback()
//{
//}


