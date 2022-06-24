#ifndef PHYSX_SNIPPET_CAMERA_H
#define PHYSX_SNIPPET_CAMERA_H

#include "foundation/PxTransform.h"
#include <glut.h>
#include<time.h>


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
		physx::PxVec3		targetDir; //旋转目标
		int					isMoving = 0; //旋转方向 1逆时针 -1顺时针 0不旋转
		void				updateDir(physx::PxVec3);
		void				calDirMoving(GLint);
		float				rotateSpeed = 200.0f; //旋转速度
		bool				isChangeImmediate = false; //是否立即改变视角
	private:
		physx::PxVec3	mEye;	
		physx::PxVec3	mDir;
	
		int				mMouseX;
		int				mMouseY;
		bool			free = false; //Ƿƶ
	};


}


#endif //PHYSX_SNIPPET_CAMERA_H