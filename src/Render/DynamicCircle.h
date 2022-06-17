#pragma once
#ifndef DYNAMICCIRCLE_H
#define DYNAMICCIRCLE_H

#include <math.h>

//动态渲染圈，以中心的圈，圈内的返回true要渲染，圈外的返回false不渲染
class DynamicCircle
{
private:
	float radius = 0.0;
	float center_X = 0.0;
	float center_Z = 0.0;
	//是否开启范围检测
	bool dynamic = true;

public:
	DynamicCircle(bool dynamic) {
		this->radius = 30.0;
		this->dynamic = dynamic;
	};

	void setRadius(float radius);
	void setCenterPosition(float center_X, float center_Z);
	bool isInCircle(float worldLocation_X, float worldLocation_Z);
};

#endif