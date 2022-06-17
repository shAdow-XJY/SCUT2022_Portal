#include "fmod.hpp"
#include <string>
#include <map>
using namespace std;
/**
* @brief ��Ƶ���Ź�����
* @param soundA ����������Ƶ
* @param soundA ��������ͨ��
* @param soundA �ɸ��ĵĵڶ�ͨ����Ƶ
* @param soundA �ɸ��ĵĵڶ�ͨ��
**/
class SoundTool
{
private:
	FMOD::System* systemA;
	FMOD::Sound* soundA, * soundB;
	FMOD::Channel* channelA = 0, * channelB = 0;
	map<string, FMOD::Sound*> soundMap;
	//ÿ����Ƶ�����Ľ������ֵ
	FMOD_RESULT       result;

public:
	SoundTool();
	~SoundTool();

	void SoundInit();
	void addSound(string filename,bool loop);

	//����result����
	FMOD_RESULT getSoundResult();

	//���º���update��ֻ����loop�е��ò��ܲ���Sound
	void SoundUpdate();

	//����/�л���Ƶ
	FMOD_RESULT playSound(string filename);

	//ֹͣ��Ƶ����
	FMOD_RESULT pauseSound();

};


