#include "SkyBox.h"
#include <glut.h>
#include <iostream>

CSkyBox::CSkyBox()
{
}

CSkyBox::~CSkyBox()
{
    /** 删除纹理对象及其占用的内存 */
    for (int i = 0; i < 6; i++)
    {
        m_texture[i].FreeImage();
        glDeleteTextures(1, &m_texture[i].ID);
    }

}

/** 天空盒初始化 */
bool CSkyBox::Init()
{
    char filename[128];                                         /**< 用来保存文件名 */
    const char* bmpName[] = { "back","front","bottom","top","left","right" };
    for (int i = 0; i < 6; i++)
    {
        sprintf(filename, "%s", bmpName[i]);
        strcat(filename, ".bmp");
        std::cout << filename << std::endl;
        if (!m_texture[i].LoadBitmap(filename))                     /**< 载入位图文件 */
        {
            //MessageBox(NULL, (LPCWSTR)"装载位图文件失败！", (LPCWSTR)"错误", MB_OK);    /**< 如果载入失败则弹出对话框 */
            std::cout << "error load image in Init" << std::endl;
        }
        glGenTextures(1, &m_texture[i].ID);                        /**< 生成一个纹理对象名称 */

        glBindTexture(GL_TEXTURE_2D, m_texture[i].ID);             /**< 创建纹理对象 */
        /** 控制滤波: */
        /*
            其中GL_TEXTURE_WRAP_S，GL_TEXTURE_WRAP_T通常可设置为GL_REPEAT或GL_CLAMP两种方式。
            当待填充的多边形大于纹理的时候，GL_REPEAT表示多余的部分用重复的方式填充；GL_CLAMP表示多余的部分用相连边缘的相邻像素填充。
            在实际绘制中，我们一般采用GL_CLAMP_EDGE来处理，这就消除了接缝处的细线，增强了天空盒的真实感。
        */
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        /** 创建纹理 */
        gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, m_texture[i].imageWidth,
            m_texture[i].imageHeight, GL_RGB, GL_UNSIGNED_BYTE,
            m_texture[i].image);
    }
    return true;

}

/** 构造天空盒 */
void  CSkyBox::CreateSkyBox(float x, float y, float z,
    float box_width, float box_height,
    float box_length)
{
    /** 获得场景中光照状态 */
    GLboolean lp;
    glGetBooleanv(GL_LIGHTING, &lp);

    /** 计算天空盒长 宽 高 */
    float width = MAP * box_width / 8;
    float height = MAP * box_height / 8;
    float length = MAP * box_length / 8;

    /** 计算天空盒中心位置 */
    x = x + MAP / 8 - width / 2;
    y = y + MAP / 24 - height / 2;
    z = z + MAP / 8 - length / 2;

    glDisable(GL_LIGHTING);            /**< 关闭光照 */

    /** 开始绘制 */
    glPushMatrix();
    //glTranslatef(-x, -y, -z);
    glColor4f(1, 1, 1, 1);
    /** 绘制背面 */
    glBindTexture(GL_TEXTURE_2D, m_texture[0].ID);

    glBegin(GL_QUADS);

    /** 指定纹理坐标和顶点坐标 */
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x + width, y, z);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x + width, y + height, z);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x, y + height, z);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x, y, z);

    glEnd();

    /** 绘制前面 */
    glBindTexture(GL_TEXTURE_2D, m_texture[1].ID);

    glBegin(GL_QUADS);

    /** 指定纹理坐标和顶点坐标 */
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x, y, z + length);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x, y + height, z + length);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x + width, y + height, z + length);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x + width, y, z + length);

    glEnd();

    /** 绘制底面 */
    glBindTexture(GL_TEXTURE_2D, m_texture[2].ID);

    glBegin(GL_QUADS);

    /** 指定纹理坐标和顶点坐标 */
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x, y, z);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x, y, z + length);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x + width, y, z + length);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x + width, y, z);

    glEnd();

    /** 绘制顶面 */
    glBindTexture(GL_TEXTURE_2D, m_texture[3].ID);

    glBegin(GL_QUADS);

    /** 指定纹理坐标和顶点坐标 */
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x + width, y + height, z);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x + width, y + height, z + length);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x, y + height, z + length);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x, y + height, z);

    glEnd();

    /** 绘制左面 */
    glBindTexture(GL_TEXTURE_2D, m_texture[4].ID);

    glBegin(GL_QUADS);

    /** 指定纹理坐标和顶点坐标 */
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x, y + height, z);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x, y + height, z + length);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x, y, z + length);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x, y, z);

    glEnd();

    /** 绘制右面 */
    glBindTexture(GL_TEXTURE_2D, m_texture[5].ID);

    glBegin(GL_QUADS);

    /** 指定纹理坐标和顶点坐标 */
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x + width, y, z);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x + width, y, z + length);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x + width, y + height, z + length);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x + width, y + height, z);
    glEnd();

    glPopMatrix();                 /** 绘制结束 */

    if (lp)                         /** 恢复光照状态 */
    {
        //std::cout << "enable lightning" << std::endl;
        glEnable(GL_LIGHTING);
    }
        

}
