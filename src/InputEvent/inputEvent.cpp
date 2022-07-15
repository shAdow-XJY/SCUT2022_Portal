#include <ctype.h>
#include "PxPhysicsAPI.h"
#include "../Role/Role.h"
#include "../Render/Camera.h"
#include "../Sound/SoundTools.h"
#include "../Animation/Animation.h"
using namespace physx;

extern Role* role;
extern SoundTool soundtool;
extern Animation animation;

extern vector<PxVec3> checkpoints;

// �Ҽ���갴��
bool press = false;

extern bool helpMenu;

// �����ʱ������
int mouseX, mouseY;

// ��ʾ�ַ���λ�ã������ã�
int textX = 0, textY = 0;

int animationTick = 0;

//�������ڽ�ɫ��ת��Ҫ�ĸ�����ת�Ƕ�
void getAdditionalAngleRadians();

//��ɫ���ܲ����ƶ���״̬
bool roleCannotOperate();

//��������
void keyPress(unsigned char key, const PxTransform& camera)
{
	if (role->nowCheckpoint == 8) {
		return;
	}

	switch (toupper(key))
	{
		//��ɫ��Ծ�������ո�
	case ' ':
	{
		if (role->tryJump(false)) {
			soundtool.playSound("jumpLoading.wav");
		}
		break;
	}
	//��ɫ�¶װ�����Z
	case 'Z':
	{
		if (!role->getCrouch()) {
			animation.setAnimation("crouching");
			role->roleCrouch();
		}
		
		break;
	}
	//��ɫʰȡ/���õ��ߣ�E
	case 'E':
	{
		if (role->getEquiped()) {
			if (role->layDownObj()) {
				soundtool.playSound("pickProp.wav",true);
				animation.setAnimation("putDown");
			}
		}
		else {
			if (role->pickUpObj()) {
				soundtool.playSound("pickProp.wav", true);
				animation.setAnimation("pickUp");
			}
		}

		break;
	}
	case 'R':
	{
		
		if (role->useKeyObj()) {
			soundtool.playSound("pickProp.wav", true);
			animation.setAnimation("useKey");
		}
		else
		{
			animation.setAnimation("notUseKey");
		}
		break;
	}
	case 'H': {
		helpMenu = !helpMenu;
		break;
	}
	case '1': {
		//��ʼλ��
		role->setFootPosition(checkpoints[0]);
		break;
	}
	case '2': {
		//�ڴ�ǰλ��
		role->setFootPosition(checkpoints[1]);
		break;
	}
	case '3': {
		//Ħ����ǰλ��
		role->setFootPosition(checkpoints[2]);
		break;
	}
	case '4': {
		//��ת·�ؿ�ǰλ��
		role->setFootPosition(checkpoints[3]);
		break;
	}
	case '5': {
		//�Թ�ǰλ��
		role->setFootPosition(checkpoints[4]);
		break;
	}
	case '6': {
		//�Թ�����λ�� 
		role->setFootPosition(checkpoints[5]);
		break;
	}
	case '7': {
		//��ת�˹ؿ�����λ��
		role->setFootPosition(checkpoints[6]);
		break;
	}
	case '8': {
		//��Ӿ��λ��
		role->setFootPosition(checkpoints[7]);
		break;
	}
	case '9': {
		//�յ�λ��
		role->setFootPosition(checkpoints[8]);
		break;
	}
	default:
		break;
	}
}

void keyRelease(unsigned char key)
{
	if (role->nowCheckpoint == 8) {
		return;
	}
	switch (toupper(key))
	{
	case ' ':
	{
		//soundtool.pauseSound();
		if (role->tryJump(true)) {
			animation.setAnimation("jumping");
			soundtool.playSound("jump.wav");
		}
		break;
	}
	case 'Z':
	{
		animation.setAnimation("idle");
		role->roleNoCrouch();
		break;
	}
	default:
		break;
	}
}

//���������
void specialKeyPress(GLint key) {
	
	if (!roleCannotOperate()) {
		return;
	}
	else if (animation.getCurrentAnimation() == "openDoor") {
		role->setSpeed(role->getSpeed() * 0.08);
		return;
	}

	int mod;
	switch (key) {
	case GLUT_KEY_UP: 
	case GLUT_KEY_DOWN:
	case GLUT_KEY_LEFT:
	case GLUT_KEY_RIGHT:
	{
		mod = glutGetModifiers();
		if (role->nowCheckpoint == 8) {
			animation.setAnimation("swimming");
		}
		else if (mod == (GLUT_ACTIVE_SHIFT)) {
			cout << "shift" << endl;
			role->setSpeed(role->getSpeed() * 1.5);
			animation.setAnimation("run");
		}else if (role->getCrouch()) {
			role->setSpeed(role->getSpeed() * 0.8);
			animation.setAnimation("crouchedWalking");
		}
		else if(animation.getCurrentAnimation() == "jumping")
		{
		}
		else {
			animation.setAnimation("walk");
		}
		getAdditionalAngleRadians();
		break;
	}
	default: {
		return;
	}
	}
	}

void specialKeyRelease(GLint key) {

	if (animation.getCurrentAnimation() == "jumping" || !roleCannotOperate()) {
		return;
	}

	switch (key) {
	case GLUT_KEY_UP: 
	case GLUT_KEY_DOWN:
	case GLUT_KEY_LEFT:
	case GLUT_KEY_RIGHT: 
	{	
		if (role->nowCheckpoint == 8) {
			animation.setAnimation("swimIdle");
		}
		else if (role->getCrouch()) {
			animation.setAnimation("crouching");
		}
		else {
			animation.setAnimation("idle");
		}
		
		break;
	}
	default: {
		return;
	}
	}
}

//�����
void mousePress(int button, int state, int x, int y) {
	switch (button)
	{
		//����Ҽ�
	case 0: {
		//�Ҽ�̧��
		if (state == 1) {

		}
		break;
	}
	case 2: {
		if (state == 1) {
			press = true;
			mouseX = x;
			mouseY = y;
			std::cout << "mouseX: " << x << std::endl;
			std::cout << "mouseY: " << y << std::endl;
		}
		break;
	}
	default:
		break;
	}
}

//��ɫת��ʱ��ģ��Ҳ�����ɫface Dir ����������ת
void getAdditionalAngleRadians() {
	//cout << role->getFaceDir().x << "����" << role->getFaceDir().y << "!!" << role->getFaceDir().z << endl;
	
	if (role->getFaceDir().z == 1) //ǰ��
	{
		animation.changeOrientation(PxQuat(0, PxVec3(0, 1, 0)));
	}
	else if (role->getFaceDir().z == -1) //��
	{
		animation.changeOrientation(PxQuat(PxPi, PxVec3(0, 1, 0)));
	}
	else if (role->getFaceDir().x == 1) //��
	{
		PxTransform rotate = PxTransform(PxQuat(PxHalfPi, PxVec3(0, 1, 0)));
		animation.changeOrientation(PxQuat(PxHalfPi, PxVec3(0, 1, 0)));
	}
	else if (role->getFaceDir().x == -1) //�ҷ�
	{
		PxTransform rotate = PxTransform(PxQuat(PxHalfPi, PxVec3(0, 1, 0)));
		animation.changeOrientation(PxQuat(-PxHalfPi, PxVec3(0, 1, 0)));
	}
}

//��ɫ���ܲ����ƶ���״̬
bool roleCannotOperate() {
	if (animation.getCurrentAnimation() == "dying" || animation.getCurrentAnimation() == "sleeping"
		|| animation.getCurrentAnimation() == "useKey" || animation.getCurrentAnimation() == "notUseKey"
		|| animation.getCurrentAnimation() == "dancing") 
	{
		return false;
	}
	return true;
}