#pragma once
#ifndef DYNAMICCIRCLE_H
#define DYNAMICCIRCLE_H
#include <iostream>
#include <math.h>

//动态渲染圈，以中心的圈，圈内的返回true要渲染，圈外的返回false不渲染
class DynamicBall
{
private:
	//是否开启范围检测
	bool dynamic = true;
	//切换圆圈范围/圆球范围
	bool ballMode = false;
	//范围半径
	float radius = 0.0;
	//坐标属性
	float center_X = 0.0;
	float center_Z = 0.0;
	float center_Y = 0.0;

public:
	DynamicBall(bool dynamic) {
		this->radius = 80.0;
		this->dynamic = dynamic;
	};
	void printDynamicXYZ() {
		std::cout << "dynamic XYZ:" <<  center_X << " " << center_Y << " " << center_Z << std::endl;
	}
	//开启或关闭
	void setDynamic(bool dynamic);
	//切换圆圈范围/圆球范围
	void setBallMode(bool mode);
	//设置半径范围
	void setRadius(float radius);
	//设置Circle中心坐标
	void setCircleCenterPosition_XZ(float center_X, float center_Z);
	//设置Ball中心坐标
	void setBallCenterPosition_XYZ(float center_X, float center_Y, float center_Z);
	//返回是否在Circle范围内的判断结果
	bool isInCircle(float worldLocation_X, float worldLocation_Z);
	//返回是否在Ball范围内的判断结果
	bool isInBall(float worldLocation_X, float worldLocation_Y, float worldLocation_Z);
};

#endif