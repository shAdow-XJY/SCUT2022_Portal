#include"BMPLoader.h"              /**< ����ͷ�ļ� */
#include <cstdio>
#include <iostream>

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
bool CBMPLoader::LoadBitmap(char* file)
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
    image = new unsigned char[bitmapInfoHeader.biSizeImage];

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

/** �ͷ��ڴ� */
void CBMPLoader::FreeImage()
{
    /** �ͷŷ�����ڴ� */
    if (image)
    {
        delete[] image;
        image = 0;
    }
}
