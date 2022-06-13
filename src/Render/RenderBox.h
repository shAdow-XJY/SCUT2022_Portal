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
class RenderBox
{
public:
    /** 构造函数 */
    RenderBox();
    ~RenderBox();

    /** 材质初始化 
    *   bool skybox 决定使用哪组材质贴图
    */
    bool Init(bool skybox);
    
    /** 渲染天空 */
    void  CreateSkyBox(float x, float y, float z,
        float width, float height,
        float length);

    /** 渲染盒子 */
    void  CreateRenderBox(float x, float y, float z,
        float width, float height,
        float length);

    int getTexture() {
        return m_texture[0].ID;
    }
private:

    CBMPLoader  m_texture[6];   /**< 天空盒/渲染盒 纹理 */
    const char* skyBmpName[6] = { "back","front","bottom","top","left","right" };
    const char* boxBmpName[6] = { "back","front","bottom","top","left","right" };
};


#endif ///__SKYBOX_H__