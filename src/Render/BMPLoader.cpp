#include"BMPLoader.h"              /**< 包含头文件 */
#include <cstdio>
#include <iostream>
#include <glut.h>

#define STB_IMAGE_IMPLEMENTATION
#include<../src/LoadModel/stb_image.h>

#define GL_CLAMP_TO_EDGE    0x812F
/** 构造函数 */
CBMPLoader::CBMPLoader()
{
    /** 初始化成员值为0 */
    image = 0;
    imageWidth = 0;
    imageHeight = 0;
}

/** 析构函数 */
CBMPLoader::~CBMPLoader()
{
    FreeImage(); /**< 释放图像数据占据的内存 */
}

/** 装载一个位图文件 */
bool CBMPLoader::LoadBitmap(const char* file)
{
    FILE* pFile = 0; /**< 文件指针 */

    /** 创建位图文件信息和位图文件头结构 */
    BITMAPINFOHEADER bitmapInfoHeader;
    BITMAPFILEHEADER header;

    unsigned char textureColors = 0;/**< 用于将图像颜色从BGR变换到RGB */

   /** 打开文件,并检查错误 */
    pFile = fopen(file, "rb");
    if (pFile == 0) { 
        perror("打开文件文件,并检查错误,失败啦");
        std::cout << "打开文件,并检查错误" << std::endl;
        return false; 
    }

    /** 读入位图文件头信息 */
    fread(&header, sizeof(BITMAPFILEHEADER), 1, pFile);

    /** 检查该文件是否为位图文件 */
    if (header.bfType != BITMAP_ID)
    {
        perror("若不是位图文件,则关闭文件并返回");
        std::cout << "若不是位图文件,则关闭文件并返回" << std::endl;
        fclose(pFile);             /**< 若不是位图文件,则关闭文件并返回 */
        return false;
    }

    /** 读入位图文件信息 */
    fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, pFile);

    /** 保存图像的宽度和高度 */
    imageWidth = bitmapInfoHeader.biWidth;
    imageHeight = bitmapInfoHeader.biHeight;

    /** 确保读取数据的大小 */
    if (bitmapInfoHeader.biSizeImage == 0)
        bitmapInfoHeader.biSizeImage = bitmapInfoHeader.biWidth *
        bitmapInfoHeader.biHeight * 3;

    /** 将指针移到数据开始位置 */
    fseek(pFile, header.bfOffBits, SEEK_SET);

    /** 分配内存 */
    image = new unsigned char[bitmapInfoHeader.biSizeImage * 2];

    /** 检查内存分配是否成功 */
    if (!image)                        /**< 若分配内存失败则返回 */
    {
        std::cout << "若分配内存失败则返回" << std::endl;
        delete[] image;
        fclose(pFile);
        return false;
    }

    /** 读取图像数据 */
    fread(image, 1, bitmapInfoHeader.biSizeImage, pFile);

    /** 将图像颜色数据格式进行交换,由BGR转换为RGB */
    for (int index = 0; index < (int)bitmapInfoHeader.biSizeImage; index += 3)
    {
        textureColors = image[index];
        image[index] = image[index + 2];
        image[index + 2] = textureColors;
    }

    fclose(pFile);       /**< 关闭文件 */
    return true;         /**< 成功返回 */
}

unsigned int CBMPLoader::generateID(const char* file) {

    if (!LoadBitmap(file))                     /**< 载入位图文件 */
    {
        std::cout << "error load image in Init" << std::endl;
    }
    std::cout << file << std::endl;

    glGenTextures(1, &ID);                        /**< 生成一个纹理对象名称 */

    glBindTexture(GL_TEXTURE_2D, ID);             /**< 创建纹理对象 */
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
    //glTexImage2D(GL_TEXTURE_2D, 0, 3, imageWidth, imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, imageWidth,
        imageHeight, GL_RGB, GL_UNSIGNED_BYTE,
        image);

    //std::cout << "sucess" << std::endl;
    return ID;
}

/** 释放内存 */
void CBMPLoader::FreeImage()
{
    /** 释放分配的内存 */
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
