#include "Camera.h"
#include <ctype.h>
#include "foundation/PxMat33.h"

using namespace physx;

namespace Snippets
{

	Camera::Camera(const PxVec3& eye, const PxVec3& dir)/*定义眼睛位置、视线方向*/
	{
		mEye = eye;
		mDir = dir.getNormalized();
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

	bool Camera::handleKey(unsigned char key, int x, int y, float speed)
	{
		PX_UNUSED(x);
		PX_UNUSED(y);

		PxVec3 viewY = mDir.cross(PxVec3(0, 1, 0)).getNormalized();
		switch (toupper(key))
		{
			if (this->free) {
		case 'W':	mEye += mDir * 2.0f * speed;		break;
		case 'S':	mEye -= mDir * 2.0f * speed;		break;
		case 'A':	mEye -= viewY * 2.0f * speed;		break;
		case 'D':	mEye += viewY * 2.0f * speed;		break;
			}
			//切换是否为自由视角
		case 'T': {
			this->free = !this->free; ;
			if (!this->free) this->mDir = PxVec3(0, -20, 50).getNormalized(); //朝物体下方看
			break;
		}
		default:							return false;
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


}