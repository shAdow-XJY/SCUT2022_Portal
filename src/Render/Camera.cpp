#include "Camera.h"
#include "cmath"
#include <ctype.h>
#include <iostream>
#include "foundation/PxMat33.h"
#include <Render/DynamicBall.h>

extern DynamicBall dynamicBall;
using namespace physx;
extern clock_t deltaClock;
extern PxVec3 roleBackPosition;
extern PxVec3 dir;

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
		mMouseX = x;
		mMouseY = y;
		switch (button)
		{
		case 0: {
			break;
		}
		case 2: {
			if (state == 1) {
				if (this->free) {
					this->mEye = roleBackPosition;
					this->targetDir = dir.getNormalized();
					this->mDir = dir.getNormalized();
				}
			}
			break;
		}
		default:
			break;
		}
	}

void Camera::goFront(float speed)
{
	mEye.z -= speed;
}

bool Camera::handleKey(unsigned char key, /*int x, int y,*/float speed)
{
	/*PX_UNUSED(x);
	PX_UNUSED(y);*/

	PxVec3 viewY = mDir.cross(PxVec3(0, 1, 0)).getNormalized();
	switch (toupper(key))
	{
		if (this->free) {
	case 'W':	mEye += mDir * 0.25f * speed * deltaClock;		break;
	case 'S':	mEye -= mDir * 0.25f * speed * deltaClock;		break;
	case 'A':	mEye -= viewY * 0.25f * speed * deltaClock;		break;
	case 'D':	mEye += viewY * 0.25f * speed * deltaClock;		break;
		}
	
		//切换是否为自由视角
	case 'T': {
		this->free = !this->free; ;
		this->isChangeImmediate = true;
		this->mDir = this->mDir.getNormalized();
		if (this->free) {
			dynamicBall.setRadius(200);
		}
		else
		{
			dynamicBall.setRadius(100);
		}
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
		if (!this->free) {
			return;
		}
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
	* @param position: 角色中心位置
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
		//通过点乘判断两个向量的接近程度
		float dis = std::abs(this->mDir.getNormalized().dot(this->targetDir.getNormalized()));
		if (dis > 0.95f) {
			this->mDir = this->targetDir;
			this->isMoving = 0;
			return;
		};
		const float delta = (this->isMoving * PxHalfPi / this->rotateSpeed ) * deltaClock;
		PxTransform rotate = PxTransform(position,PxQuat(delta, PxVec3(0, 1, 0)));
		this->mDir = rotate.rotate(this->mDir);
		this->mEye = position - this->mDir;
	}
	
	/**
	* @brief 根据按键计算旋转的方向
	* @param key: 按键
	**/
	void Camera::calDirMoving(GLint key) {
		if (this->isMoving) return;
		switch (key) {
			case GLUT_KEY_UP: {
				break;
			}
			case GLUT_KEY_DOWN: {
				this->isMoving = 1;
				//this->isChangeImmediate = true;
				break;

			}case GLUT_KEY_LEFT: {
				this->isMoving = 1;
				break;

			}case GLUT_KEY_RIGHT: {
				this->isMoving = -1;
				break;

			}
			default: {
				return;
			}
		}
	}

}