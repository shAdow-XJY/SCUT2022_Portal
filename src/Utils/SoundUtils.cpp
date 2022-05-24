#include "SoundUtils.h"
#include<stdio.h>
using namespace std;


bool checkFileExists(const char* filePath)
{
	auto file = fopen(filePath, "rb");
	if (file != NULL)
	{
		fclose(file);
		return true;
	}
	return false;
}

int Sound::playSound()
{
	if (checkFileExists("D:/test.wav"))
	{
		PlaySound(TEXT("D:/test.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
	}
	return 0;
}
