#ifndef PHYSX_SNIPPET_CAMERA_H
#define PHYSX_SNIPPET_CAMERA_H

#include "foundation/PxTransform.h"

namespace Snippets
{
	class Camera
	{
	public:
		Camera(const physx::PxVec3& eye, const physx::PxVec3& dir);

		void				handleMouse(int button, int state, int x, int y);
		bool				handleKey(unsigned char key, int x, int y,float speed = 1.0f);
		void				handleMotion(int x, int y);
		void				handleAnalogMove(float x, float y);
		void				goFront(float speed);
		void				setEye(physx::PxVec3 position);
		void				setDir(physx::PxVec3 direction);
		physx::PxVec3		getEye()	const;
		physx::PxVec3		getDir()	const;
		physx::PxTransform	getTransform() const;
		bool				isFree();
	private:
		physx::PxVec3	mEye;	
		physx::PxVec3	mDir;
		int				mMouseX;
		int				mMouseY;
		bool			free = true; //Ƿƶ
	};


}


#endif //PHYSX_SNIPPET_CAMERA_H