#include "DynamicBall.h"

void DynamicBall::setDynamic(bool dynamic)
{
	this->dynamic = dynamic;
}

void DynamicBall::setBallMode(bool mode)
{
	this->ballMode = mode;
}

void DynamicBall::setRadius(float radius)
{
	this->radius = radius;
}

void DynamicBall::setCircleCenterPosition_XZ(float center_X, float center_Z)
{
	this->center_X = center_X;
	this->center_Z = center_Z;
}

void DynamicBall::setBallCenterPosition_XYZ(float center_X, float center_Y, float center_Z)
{
	this->center_X = center_X;
	this->center_Y = center_Y;
	this->center_Z = center_Z;
}

/**
* @brief 返回是否在圈内
* @return true在范围内/false不在范围内
**/
bool DynamicBall::isInCircle(float worldLocation_X, float worldLocation_Z)
{
	if (!this->dynamic) {
		return true;
	}

	float length_X = fabsf(worldLocation_X - this->center_X);
	float length_Z = fabsf(worldLocation_Z - this->center_Z);

	if (length_X > this->radius || length_Z > this->radius)
	{
		return false;
	}

	float length = sqrtf(powf(length_X, 2) + powf(length_Z, 2));

	if (length <= this->radius)
	{
		return true;
	}

	return false;
}

/**
* @brief 返回是否在球内
* @return true在范围内/false不在范围内
**/
bool DynamicBall::isInBall(float worldLocation_X, float worldLocation_Y, float worldLocation_Z)
{
	if (!this->dynamic || !this->ballMode) {
		return true;
	}

	float length_X = fabsf(worldLocation_X - this->center_X);
	float length_Y = fabsf(worldLocation_Y - this->center_Y);
	float length_Z = fabsf(worldLocation_Z - this->center_Z);

	if (length_X > this->radius || length_Y > this->radius || length_Z > this->radius)
	{
		return false;
	}

	float length = sqrtf(powf(length_X, 2) + powf(length_Y, 2) + powf(length_Z, 2));

	if (length <= this->radius)
	{
		return true;
	}

	return false;

}
