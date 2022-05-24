#include "module.h"
#include <PsString.h>
module::module():shape(NULL),body0(NULL) {}

module::module(PxTransform o, PxShape* s, PxTransform l, PxRigidActor* b0):
	ori(o), 
	shape(s), 
	local(l), 
	body0(b0) {}

module::~module() {

}

void module::onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs)
{
	printf("onContact.......\n");
}

//void module::onTrigger(PxTriggerPair* pairs, PxU32 count)
//{
//	printf("Enter onTrigger!\n");
//	for (PxU32 i = 0; i < count; i++)
//	{
//		// ignore pairs when shapes have been deleted
//		if (pairs[i].flags & (PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER | PxTriggerPairFlag::eREMOVED_SHAPE_OTHER)) {
//			printf("Enter first 'if' !\n");
//			continue;
//		}
//			
//
//		if ((pairs[i].otherActor == body0) && (pairs[i].triggerActor == body1))
//		{
//			printf("module::onTrigger!\n");
//		}
//	}
//}
