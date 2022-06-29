#include"BMPLoader.h"              /**< ����ͷ�ļ� */
#include <cstdio>
#include <iostream>
#include <glut.h>

#define STB_IMAGE_IMPLEMENTATION
#include<../src/LoadModel/stb_image.h>

#define GL_CLAMP_TO_EDGE    0x812F
/** ���캯�� */
CBMPLoader::CBMPLoader()
{
    /** ��ʼ����ԱֵΪ0 */
    image = 0;
    imageWidth = 0;
    imageHeight = 0;
}

/** �������� */
CBMPLoader::~CBMPLoader()
{
    FreeImage(); /**< �ͷ�ͼ������ռ�ݵ��ڴ� */
}

/** װ��һ��λͼ�ļ� */
bool CBMPLoader::LoadBitmap(const char* file)
{
    FILE* pFile = 0; /**< �ļ�ָ�� */

    /** ����λͼ�ļ���Ϣ��λͼ�ļ�ͷ�ṹ */
    BITMAPINFOHEADER bitmapInfoHeader;
    BITMAPFILEHEADER header;

    unsigned char textureColors = 0;/**< ���ڽ�ͼ����ɫ��BGR�任��RGB */

   /** ���ļ�,�������� */
    pFile = fopen(file, "rb");
    if (pFile == 0) { 
        perror("���ļ��ļ�,��������,ʧ����");
        std::cout << "���ļ�,��������" << std::endl;
        return false; 
    }

    /** ����λͼ�ļ�ͷ��Ϣ */
    fread(&header, sizeof(BITMAPFILEHEADER), 1, pFile);

    /** �����ļ��Ƿ�Ϊλͼ�ļ� */
    if (header.bfType != BITMAP_ID)
    {
        perror("������λͼ�ļ�,��ر��ļ�������");
        std::cout << "������λͼ�ļ�,��ر��ļ�������" << std::endl;
        fclose(pFile);             /**< ������λͼ�ļ�,��ر��ļ������� */
        return false;
    }

    /** ����λͼ�ļ���Ϣ */
    fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, pFile);

    /** ����ͼ��Ŀ�Ⱥ͸߶� */
    imageWidth = bitmapInfoHeader.biWidth;
    imageHeight = bitmapInfoHeader.biHeight;

    /** ȷ����ȡ���ݵĴ�С */
    if (bitmapInfoHeader.biSizeImage == 0)
        bitmapInfoHeader.biSizeImage = bitmapInfoHeader.biWidth *
        bitmapInfoHeader.biHeight * 3;

    /** ��ָ���Ƶ����ݿ�ʼλ�� */
    fseek(pFile, header.bfOffBits, SEEK_SET);

    /** �����ڴ� */
    image = new unsigned char[bitmapInfoHeader.biSizeImage * 2];

    /** ����ڴ�����Ƿ�ɹ� */
    if (!image)                        /**< �������ڴ�ʧ���򷵻� */
    {
        std::cout << "�������ڴ�ʧ���򷵻�" << std::endl;
        delete[] image;
        fclose(pFile);
        return false;
    }

    /** ��ȡͼ������ */
    fread(image, 1, bitmapInfoHeader.biSizeImage, pFile);

    /** ��ͼ����ɫ���ݸ�ʽ���н���,��BGRת��ΪRGB */
    for (int index = 0; index < (int)bitmapInfoHeader.biSizeImage; index += 3)
    {
        textureColors = image[index];
        image[index] = image[index + 2];
        image[index + 2] = textureColors;
    }

    fclose(pFile);       /**< �ر��ļ� */
    return true;         /**< �ɹ����� */
}

unsigned int CBMPLoader::generateID(const char* file) {

    if (!LoadBitmap(file))                     /**< ����λͼ�ļ� */
    {
        std::cout << "error load image in Init" << std::endl;
    }
    std::cout << file << std::endl;

    glGenTextures(1, &ID);                        /**< ����һ������������� */

    glBindTexture(GL_TEXTURE_2D, ID);             /**< ����������� */
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
    //glTexImage2D(GL_TEXTURE_2D, 0, 3, imageWidth, imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, imageWidth,
        imageHeight, GL_RGB, GL_UNSIGNED_BYTE,
        image);

    //std::cout << "sucess" << std::endl;
    return ID;
}

/** �ͷ��ڴ� */
void CBMPLoader::FreeImage()
{
    /** �ͷŷ�����ڴ� */
    if (image)
    {
        delete[] image;
        //stbi_image_free(image);
        image = 0;
    }
}


bool CBMPLoader::LoadOtherPic(const char* filename)
{
    image = stbi_load(filename, &imageWidth, &imageHeight, &nbChannels, 0);
    if (image != nullptr) return true;
    return false;
}


unsigned int CBMPLoader::generateModelID(const char* filename)
{
    if (!LoadOtherPic(filename)) {
        std::cout << "error load image in Init" << std::endl;
    }
    std::cout << "Imported: " << filename << std::endl;

    unsigned int ID;
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID); 

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (this->nbChannels == 3) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    }
    else {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    }
    
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, imageWidth,
        imageHeight, GL_RGB, GL_UNSIGNED_BYTE,
        image);
    return ID;
}
