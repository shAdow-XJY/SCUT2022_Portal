#ifndef SOUND_H
#define SOUND_H

#include<iostream>
#include<windows.h>
#include<Mmsystem.h>
#pragma comment(lib,"winmm.lib")
using namespace std;

class Sound {

public:
	int playSound();

};
#endif
