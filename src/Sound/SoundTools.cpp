#include "SoundTools.h"
#include <iostream>
using namespace std;

//���캯��������bmg����Ч����ʼ��
SoundTool::SoundTool() {
	result = FMOD::System_Create(&systemA);
	result = systemA->init(32, FMOD_INIT_NORMAL, NULL);
	result = systemA->createSound("../../sounds/bgm.wav", FMOD_LOOP_NORMAL, 0, &soundA);
	result = systemA->playSound(soundA, 0, 0, &channelA);
	SoundInit();
}

SoundTool::~SoundTool() {
	result = soundA->release();
	result = soundB->release();
	result = systemA->close();
	result = systemA->release();
}

void SoundTool::SoundInit() {
	addSound("jumpLoading.wav",true);
	addSound("jump.wav",false);
	addSound("rain.wav", false);
	addSound("openDoorSlowly.wav", false);
	addSound("openDoorQuickly.wav", false);
	addSound("gameOver.wav", false);
	addSound("passLevel.wav", false);
}

//�����Ч
void SoundTool::addSound(string filename,bool loop) {
	string tempPath = "../../sounds/" + filename;
	FMOD::Sound* newSound;

	if (loop) {
		result = systemA->createSound(tempPath.c_str(), FMOD_LOOP_NORMAL, 0, &newSound);
	}
	else{
		result = systemA->createSound(tempPath.c_str(), FMOD_LOOP_OFF, 0, &newSound);
	}

	if (result == FMOD_OK) {
		cout << "add successfully sound:" << filename << endl;
		soundMap.insert(pair<string, FMOD::Sound*>(filename, newSound));
	}
}

//����result����
FMOD_RESULT SoundTool::getSoundResult() {
	return result;
}

//���º���update��ֻ����loop�е��ò��ܲ���Sound
void SoundTool::SoundUpdate()
{
	result = systemA->update();
}

/**
* @brief ����/�л���Ƶ
* @param interrupt �Ƿ������ж�ԭ����Ч��������Ч
**/
FMOD_RESULT SoundTool::playSound(string filename, bool interrupt) 
{
	FMOD::Sound* currentsound = 0;
	channelB->getCurrentSound(&currentsound);

	if (currentsound != soundMap[filename] || interrupt) 
	{
		pauseSound();
		result = systemA->playSound(soundMap[filename], 0, 0, &channelB);
	}

	return result;
}

//ֹͣ��Ƶ����
FMOD_RESULT SoundTool::pauseSound()
{
	return result = channelB->stop();
}
