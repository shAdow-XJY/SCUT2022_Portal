#include "SoundTools.h"
#include <iostream>
using namespace std;

//构造函数，播放bmg和音效包初始化
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
	addSound("jumpLoading.wav", true);
	addSound("jump.wav", false);
	addSound("rain.wav", false);
	addSound("openDoorSlowly.wav", false);
	addSound("openDoorQuickly.wav", false);
	addSound("gameOver.wav", false);
	addSound("passLevel.wav", false);
}

//添加音效
void SoundTool::addSound(string filename, bool loop) {
	string tempPath = "../../sounds/" + filename;
	FMOD::Sound* newSound;

	if (loop) {
		result = systemA->createSound(tempPath.c_str(), FMOD_LOOP_NORMAL, 0, &newSound);
	}
	else {
		result = systemA->createSound(tempPath.c_str(), FMOD_LOOP_OFF, 0, &newSound);
	}

	if (result == FMOD_OK) {
		cout << "add successfully sound:" << filename << endl;
		soundMap.insert(pair<string, FMOD::Sound*>(filename, newSound));
	}
}

//返回result属性
FMOD_RESULT SoundTool::getSoundResult() {
	return result;
}

//更新函数update，只有在loop中调用才能播放Sound
void SoundTool::SoundUpdate()
{
	result = systemA->update();
}

/**
* @brief 播放/切换音频
* @param interrupt 是否立刻中断原有音效并播放音效
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

//停止音频播放
FMOD_RESULT SoundTool::pauseSound()
{
	return result = channelB->stop();
}
