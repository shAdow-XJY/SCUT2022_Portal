#include "RenderBox.h"
#include <glut.h>
#include <iostream>

RenderBox::RenderBox()
{
}

RenderBox::~RenderBox()
{
    /** ɾ�����������ռ�õ��ڴ� */
    for (int i = 0; i < 6; i++)
    {
        m_texture[i].FreeImage();
        glDeleteTextures(1, &m_texture[i].ID);
    }

}

/** ������ͼ��ʼ�� */
bool RenderBox::Init(bool skybox)
{
    char filename[128];                                         /**< ���������ļ��� */
    const char* bmpName[6];
    if (skybox) {
        bmpName[0] = this->skyBmpName[0];
        bmpName[1] = this->skyBmpName[1];
        bmpName[2] = this->skyBmpName[2];
        bmpName[3] = this->skyBmpName[3];
        bmpName[4] = this->skyBmpName[4];
        bmpName[5] = this->skyBmpName[5];
    }
    else
    {
        bmpName[0] = this->boxBmpName[0];
        bmpName[1] = this->boxBmpName[1];
        bmpName[2] = this->boxBmpName[2];
        bmpName[3] = this->boxBmpName[3];
        bmpName[4] = this->boxBmpName[4];
        bmpName[5] = this->boxBmpName[5];
    }
    //const char* bmpName[] = { "back","front","bottom","top","left","right" };
    for (int i = 0; i < 6; i++)
    {
        sprintf(filename, "%s", bmpName[i]);
        strcat(filename, ".bmp");
        std::cout << filename << std::endl;
        if (!m_texture[i].LoadBitmap(filename))                     /**< ����λͼ�ļ� */
        {
            //MessageBox(NULL, (LPCWSTR)"װ��λͼ�ļ�ʧ�ܣ�", (LPCWSTR)"����", MB_OK);    /**< �������ʧ���򵯳��Ի��� */
            std::cout << "error load image in Init" << std::endl;
        }
        glGenTextures(1, &m_texture[i].ID);                        /**< ����һ������������� */

        glBindTexture(GL_TEXTURE_2D, m_texture[i].ID);             /**< ����������� */
        /** �����˲�: */
        /*
            ����GL_TEXTURE_WRAP_S��GL_TEXTURE_WRAP_Tͨ��������ΪGL_REPEAT��GL_CLAMP���ַ�ʽ��
            �������Ķ���δ��������ʱ��GL_REPEAT��ʾ����Ĳ������ظ��ķ�ʽ��䣻GL_CLAMP��ʾ����Ĳ�����������Ե������������䡣
            ��ʵ�ʻ����У�����һ�����GL_CLAMP_EDGE��������������˽ӷ촦��ϸ�ߣ���ǿ����պе���ʵ�С�
        */
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        /** �������� */
        gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, m_texture[i].imageWidth,
            m_texture[i].imageHeight, GL_RGB, GL_UNSIGNED_BYTE,
            m_texture[i].image);
    }
    return true;

}

/** ������պ� */
void  RenderBox::CreateSkyBox(float x, float y, float z,
    float box_width, float box_height,
    float box_length)
{
    /** ��ó����й���״̬ */
    GLboolean lp;
    glGetBooleanv(GL_LIGHTING, &lp);

    /** ������պг� �� �� */
    float width = MAP * box_width / 8;
    float height = MAP * box_height / 8;
    float length = MAP * box_length / 8;

    /** ������պ�����λ�� */
    x = x + MAP / 8 - width / 2;
    y = y + MAP / 24 - height / 2;
    z = z + MAP / 8 - length / 2;

    glDisable(GL_LIGHTING);            /**< �رչ��� */

    /** ��ʼ���� */
    glPushMatrix();
    //glTranslatef(-x, -y, -z);
    glColor4f(1, 1, 1, 1);
    /** ���Ʊ��� */
    glBindTexture(GL_TEXTURE_2D, m_texture[0].ID);

    glBegin(GL_QUADS);

    /** ָ����������Ͷ������� */
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x + width, y, z);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x + width, y + height, z);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x, y + height, z);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x, y, z);

    glEnd();

    /** ����ǰ�� */
    glBindTexture(GL_TEXTURE_2D, m_texture[1].ID);

    glBegin(GL_QUADS);

    /** ָ����������Ͷ������� */
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x, y, z + length);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x, y + height, z + length);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x + width, y + height, z + length);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x + width, y, z + length);

    glEnd();

    /** ���Ƶ��� */
    glBindTexture(GL_TEXTURE_2D, m_texture[2].ID);

    glBegin(GL_QUADS);

    /** ָ����������Ͷ������� */
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x, y, z);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x, y, z + length);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x + width, y, z + length);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x + width, y, z);

    glEnd();

    /** ���ƶ��� */
    glBindTexture(GL_TEXTURE_2D, m_texture[3].ID);

    glBegin(GL_QUADS);

    /** ָ����������Ͷ������� */
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x + width, y + height, z);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x + width, y + height, z + length);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x, y + height, z + length);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x, y + height, z);

    glEnd();

    /** �������� */
    glBindTexture(GL_TEXTURE_2D, m_texture[4].ID);

    glBegin(GL_QUADS);

    /** ָ����������Ͷ������� */
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x, y + height, z);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x, y + height, z + length);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x, y, z + length);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x, y, z);

    glEnd();

    /** �������� */
    glBindTexture(GL_TEXTURE_2D, m_texture[5].ID);

    glBegin(GL_QUADS);

    /** ָ����������Ͷ������� */
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x + width, y, z);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x + width, y, z + length);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x + width, y + height, z + length);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x + width, y + height, z);
    glEnd();

    glPopMatrix();                 /** ���ƽ��� */

    if (lp)                         /** �ָ�����״̬ */
    {
        //std::cout << "enable lightning" << std::endl;
        glEnable(GL_LIGHTING);
    }
        

}

/** ������Ⱦ�� 
*  x y z : ������λ��
*  box_width box_height box_length :�еĴ�С����
*/
void  RenderBox::CreateRenderBox(float x, float y, float z,
    float box_width, float box_height,
    float box_length) {

    /** ���㳤 �� �� */
    float width = box_width;
    float height = box_height;
    float length = box_length;

    /** ������պ�����λ�� */
    x = x - width / 2;
    y = y - height / 2;
    z = z - length / 2;
    std::cout << "4654654564" << x << " " << y << " " << z << std::endl;
    /** ��ʼ���� */
    glPushMatrix();
    //glTranslatef(-x, -y, -z);
    glColor4f(1, 1, 1, 1);
    /** ���Ʊ��� */
    glBindTexture(GL_TEXTURE_2D, m_texture[0].ID);

    glBegin(GL_QUADS);

    /** ָ����������Ͷ������� */
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x + width, y, z);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x + width, y + height, z);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x, y + height, z);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x, y, z);

    glEnd();

    /** ����ǰ�� */
    glBindTexture(GL_TEXTURE_2D, m_texture[1].ID);

    glBegin(GL_QUADS);

    /** ָ����������Ͷ������� */
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x, y, z + length);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x, y + height, z + length);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x + width, y + height, z + length);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x + width, y, z + length);

    glEnd();

    /** ���Ƶ��� */
    glBindTexture(GL_TEXTURE_2D, m_texture[2].ID);

    glBegin(GL_QUADS);

    /** ָ����������Ͷ������� */
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x, y, z);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x, y, z + length);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x + width, y, z + length);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x + width, y, z);

    glEnd();

    /** ���ƶ��� */
    glBindTexture(GL_TEXTURE_2D, m_texture[3].ID);

    glBegin(GL_QUADS);

    /** ָ����������Ͷ������� */
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x + width, y + height, z);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x + width, y + height, z + length);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x, y + height, z + length);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x, y + height, z);

    glEnd();

    /** �������� */
    glBindTexture(GL_TEXTURE_2D, m_texture[4].ID);

    glBegin(GL_QUADS);

    /** ָ����������Ͷ������� */
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x, y + height, z);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x, y + height, z + length);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x, y, z + length);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x, y, z);

    glEnd();

    /** �������� */
    glBindTexture(GL_TEXTURE_2D, m_texture[5].ID);

    glBegin(GL_QUADS);

    /** ָ����������Ͷ������� */
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x + width, y, z);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x + width, y, z + length);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x + width, y + height, z + length);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x + width, y + height, z);
    glEnd();

    glPopMatrix();                 /** ���ƽ��� */
}
