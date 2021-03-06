#ifndef __ROLE_HIT_CALLBACK__H__
#define __ROLE_HIT_CALLBACK__H__

#include <ctype.h>
#include <iostream>
#include "PxPhysicsAPI.h"
#include "../Sound/SoundTools.h"
#include "../Animation/Animation.h"
#include "../Role/Role.h"
using namespace physx;



extern PxScene* gScene;
extern PxMaterial* gMaterial;
extern PxPhysics* gPhysics;
extern PxControllerManager* cManager;
extern void renderGameOver();

extern SoundTool soundtool;
extern Animation animation;

class RoleHitCallback :public PxUserControllerHitReport {
public:
	void onShapeHit(const PxControllerShapeHit& hit) {
	}
	void onControllerHit(const PxControllersHit& hit) {
	}
	void  onObstacleHit(const PxControllerObstacleHit& hit) {
	}
};

/**
* @brief 角色碰撞判定，用于角色撞物体和推物体
**/
class RoleHitBehaviorCallback :public PxControllerBehaviorCallback {
private:
	Role* role = NULL;
public:
	RoleHitBehaviorCallback(Role* role) :role(role) {};
	PxControllerBehaviorFlags getBehaviorFlags(const PxShape& shape, const PxActor& actor) {
		////是否接触到地面
		//if (actor.getName() != "Ground") {
		//	this->role->stopMoving();
		//}
		if (actor.getName()) {
			string name(actor.getName());
			if (name == "Door") {
				Door* door = (Door*)actor.userData;
				float scale = 9000.0f;

				if (!door->getNeedKey()) {

					door->addPForce(role->getFaceDir() * scale);

					if (door->canOpen()) {
						if (!door->getDoorStauts()) {
							animation.setAnimation("openDoor");
							soundtool.playSound("openDoorSlowly.wav", true);
							door->setDoorStatus(true);
						}
					}
					else {
						animation.setAnimation("openDoor");
					}
				}
			}
			else if (name == "Particle") {
				//cout << "PoolWall" << endl;
				role->nowCheckpoint = 8;
			}
			else if (name == "FinalLine") {
				//cout << "PoolWall" << endl;
				//role->nowCheckpoint = 9;
				animation.setAnimation("dancing");
			}
			else if (name == "Pendulum") {
				cout << "Pendulum" << endl;
				animation.setAnimation("roll");
			}
			else if (name == "Over" || name == "Ground0") {
				//cout << "Over" << endl;
				animation.setAnimation("dying");
			}
			else if (animation.getCurrentAnimation() == "swimming" && name == "Road") {
				role->nowCheckpoint = 9;
			}
		}

		return PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT;
	}
	PxControllerBehaviorFlags getBehaviorFlags(const PxController& controller) {
		return PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT;
	}
	PxControllerBehaviorFlags getBehaviorFlags(const PxObstacle& obstacle) {
		return PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT;
	}
};
#endif
