#include "Camera.h"
#include "cmath"
#include <ctype.h>
#include <iostream>
#include "foundation/PxMat33.h"

using namespace physx;

namespace Snippets
{

	Camera::Camera(const PxVec3& eye, const PxVec3& dir)/*定义眼睛位置、视线方向*/
	{
		mEye = eye;
		mDir = dir.getNormalized();
		targetDir = mDir;
		mMouseX = 0;
		mMouseY = 0;
	}

	void Camera::handleMouse(int button, int state, int x, int y)
	{
		PX_UNUSED(state);
		PX_UNUSED(button);
		mMouseX = x;
		mMouseY = y;
	}

void Camera::goFront(float speed)
{
	mEye.z -= speed;
}

bool Camera::handleKey(unsigned char key, int x, int y,float speed)
{
	PX_UNUSED(x);
	PX_UNUSED(y);

		PxVec3 viewY = mDir.cross(PxVec3(0, 1, 0)).getNormalized();
		switch (toupper(key))
		{
			if (this->free) {
		case 'W':	mEye += mDir * 20.0f * speed;		break;
		case 'S':	mEye -= mDir * 20.0f * speed;		break;
		case 'A':	mEye -= viewY * 20.0f * speed;		break;
		case 'D':	mEye += viewY * 20.0f * speed;		break;
			}
			//切换是否为自由视角
		case 'T': {
			this->free = !this->free; ;
			this->isChangeImmediate = true;
			//if (!this->free) this->mDir = dir.getNormalized(); //朝物体下方看
			break;
		}
		default:							
			return false;
	}
	return true;
}

	void Camera::handleAnalogMove(float x, float y)
	{
		PxVec3 viewY = mDir.cross(PxVec3(0, 1, 0)).getNormalized();
		mEye += mDir * y;
		mEye += viewY * x;
	}

	void Camera::handleMotion(int x, int y)
	{
		int dx = mMouseX - x;/*鼠标每次移动带来的视角变换*/
		int dy = mMouseY - y;
		float sensitivity = 0.3;/*鼠标灵敏度[0,1]即可*/

		PxVec3 viewY = mDir.cross(PxVec3(0, 1, 0)).getNormalized();

		PxQuat qx(PxPi * dx * sensitivity / 180.0f, PxVec3(0, 1, 0));
		mDir = qx.rotate(mDir);
		PxQuat qy(PxPi * dy * sensitivity / 180.0f, viewY);
		mDir = qy.rotate(mDir);

		mDir.normalize();

		mMouseX = x;
		mMouseY = y;
	}

	PxTransform Camera::getTransform() const
	{
		PxVec3 viewY = mDir.cross(PxVec3(0, 1, 0));

		if (viewY.normalize() < 1e-6f)
			return PxTransform(mEye);

		PxMat33 m(mDir.cross(viewY), viewY, -mDir);
		return PxTransform(mEye, PxQuat(m));
	}

	void Camera::setEye(PxVec3 position) {
		this->mEye = position;
	}

	void Camera::setDir(PxVec3 direction) {
		this->mDir = direction;
	}

	bool Camera::isFree() {
		return this->free;
	}

	PxVec3 Camera::getEye() const
	{
		return mEye;
	}

	PxVec3 Camera::getDir() const
	{
		return mDir;
	}
	/**
	* @brief 每帧更新摄像机
	* @Param position角色中心位置
	**/
	void Camera::updateDir(PxVec3 position) {
		if (this->isChangeImmediate) {
			this->targetDir = this->targetDir.getNormalized();
			this->mDir = this->targetDir;
			this->isChangeImmediate = false;
			return;
		}
		if (this->free || this->mDir == this->targetDir) {
			this->isMoving = 0;
			return;
		};
		float dis = std::abs(this->mDir.getNormalized().dot(this->targetDir.getNormalized()));
		if (dis > 0.99f) {
			this->mDir = this->targetDir;
			this->isMoving = 0;
			return;
		};
		const float delta = PxHalfPi / this->rotateSpeed * this->isMoving;
		PxTransform rotate = PxTransform(position,PxQuat(delta, PxVec3(0, 1, 0)));
		this->mDir = rotate.rotate(this->mDir);
		this->mEye = position - this->mDir;
	}
	
	/**
	* @brief 根据按键计算旋转的方向
	* @Param key 按键
	**/
	void Camera::calDirMoving(GLint key) {
		if (this->isMoving) return;
		switch (key) {
			case GLUT_KEY_UP: {
				break;
			}
			case GLUT_KEY_DOWN: {
				this->isMoving = 1;
				//this->rotateSpeed = 400.0f;
				//this->isChangeImmediate = true;
				break;

			}case GLUT_KEY_LEFT: {
				this->isMoving = 1;
				//this->rotateSpeed = 400.0f;
				break;

			}case GLUT_KEY_RIGHT: {
				this->isMoving = -1;
				//this->rotateSpeed = 400.0f;
				break;

			}
			default: {
				return;
			}
		}
	}

}