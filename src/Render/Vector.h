﻿#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <math.h>                            /**< 包含math头文件 */

/** 向量类 */
class Vector3
{
public:
    /** 构造函数 */
    Vector3() { x = 0.0; y = 0.0; z = 0.0; }
    Vector3(float xx, float yy, float zz)
    {
        x = xx;
        y = yy;
        z = zz;
    }
    Vector3(const Vector3& vec)
    {
        x = vec.x;
        y = vec.y;
        z = vec.z;
    }

    /** 成员函数 */
    inline float length();                       /**< 计算向量长度 */
    Vector3 normalize();                         /**< 单位化向量 */
    float dotProduct(const Vector3& v);          /**< 计算点积 */
    Vector3 crossProduct(const Vector3& v);      /**< 计算叉积 */

    /** 重载操作符 */
    Vector3 operator + (const Vector3& v);
    Vector3 operator - (const Vector3& v);
    Vector3 operator * (float scale);
    Vector3 operator / (float scale);
    Vector3 operator - ();

public:
    float x, y, z;

};

#endif //__VECTOR_H__