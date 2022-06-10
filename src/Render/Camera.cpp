#include "Camera.h"
#include "cmath"
#include <ctype.h>
#include <iostream>
#include "foundation/PxMat33.h"

using namespace physx;

namespace Snippets
{

	Camera::Camera(const PxVec3& eye, const PxVec3& dir)/*�����۾�λ�á����߷���*/
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
		case 'W':	mEye += mDir * 2.0f * speed;		break;
		case 'S':	mEye -= mDir * 2.0f * speed;		break;
		case 'A':	mEye -= viewY * 2.0f * speed;		break;
		case 'D':	mEye += viewY * 2.0f * speed;		break;
			}
			//�л��Ƿ�Ϊ�����ӽ�
		case 'T': {
			this->free = !this->free; ;
			this->isChangeImmediate = true;
			this->mDir = this->mDir.getNormalized();
			//if (!this->free) this->mDir = dir.getNormalized(); //�������·���
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
		int dx = mMouseX - x;/*���ÿ���ƶ��������ӽǱ任*/
		int dy = mMouseY - y;
		float sensitivity = 0.3;/*���������[0,1]����*/

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
	* @brief ÿ֡���������
	* @Param position��ɫ����λ��
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
	* @brief ���ݰ���������ת�ķ���
	* @Param key ����
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