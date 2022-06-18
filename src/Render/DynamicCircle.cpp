#include "DynamicCircle.h"

void DynamicCircle::setRadius(float radius)
{
	this->radius = radius;
}

void DynamicCircle::setCenterPosition(float center_X, float center_Z)
{
	this->center_X = center_X;
	this->center_Z = center_Z;
}

bool DynamicCircle::isInCircle(float worldLocation_X, float worldLocation_Z)
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
