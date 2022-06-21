#pragma once
#include "fmod.hpp"
#include <string>
#include <map>
using namespace std;
/**
* @brief 音频播放工具类
* @param soundA 背景音乐音频
* @param soundA 背景音乐通道
* @param soundA 可更改的第二通道音频
* @param soundA 可更改的第二通道
**/
class SoundTool
{
private:
	FMOD::System* systemA;
	FMOD::Sound* soundA, * soundB;
	FMOD::Channel* channelA = 0, * channelB = 0;
	map<string, FMOD::Sound*> soundMap;
	//每次音频操作的结果返回值
	FMOD_RESULT       result;

public:
	SoundTool();
	~SoundTool();

	void SoundInit();
	void addSound(string filename,bool loop);

	//返回result属性
	FMOD_RESULT getSoundResult();

	//更新函数update，只有在loop中调用才能播放Sound
	void SoundUpdate();

	//播放/切换音频
	FMOD_RESULT playSound(string filename,bool interrupt = false);

	//停止音频播放
	FMOD_RESULT pauseSound();

};


