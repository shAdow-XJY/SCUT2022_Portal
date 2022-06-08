#ifndef __SKYBOX_H__
#define __SKYBOX_H__

#include "BMPLoader.h"
#include "Vector.h"
#include "Camera.h"

#define GL_CLAMP_TO_EDGE    0x812F

const unsigned int MAP_WIDTH = 2048;
const unsigned int CELL_WIDTH = 16;
const unsigned int MAP = MAP_WIDTH * CELL_WIDTH / 2;

/** 天空盒类 */
class CSkyBox
{
public:
    /** 构造函数 */
    CSkyBox();
    ~CSkyBox();

    /** 初始化 */
    bool Init();

    /** 渲染天空 */
    void  CreateSkyBox(float x, float y, float z,
        float width, float height,
        float length);

private:

    CBMPLoader  m_texture[6];   /**< 天空盒纹理 */

};


#endif ///__SKYBOX_H__