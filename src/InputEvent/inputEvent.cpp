#include <ctype.h>
#include "PxPhysicsAPI.h"
#include "../Role/Role.h"
#include "../Render/Camera.h"
#include "../Sound/SoundTools.h"
#include <Animation/Animation.h>
using namespace physx;

extern Role* role;
extern SoundTool soundtool;
extern Animation animation;
// �Ҽ���갴��
bool press = false;


// �����ʱ������
int mouseX, mouseY;

// ��ʾ�ַ���λ�ã������ã�
int textX = 0, textY = 0;

int timeAnim = 0;
//��������
void keyPress(unsigned char key, const PxTransform& camera)
{
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
		role->roleCrouch();
		break;
	}
	//��ɫʰȡ/���õ��ߣ�E
	case 'E':
	{
		if (role->getEquiped()) {
			role->layDownObj();

		}
		else {
			role->pickUpObj();

		}

		break;
	}
	//��ɫ�л���������ʱ
	case 'Q':
	{
		//animation.
		animation.update(1000*timeAnim);
		timeAnim++;
		break;
	}
	default:
		break;
	}
}

void keyRelease(unsigned char key)
{
	switch (toupper(key))
	{
	case ' ':
	{
		//soundtool.pauseSound();
		if (role->tryJump(true)) {
			soundtool.playSound("jump.wav");
			animation.setAnimation("jump");
		}
		break;
	}
	case 'Z':
	{
		role->roleNoCrouch();
		break;
	}
	default:
		break;
	}
}

//���������
void specialKeyPress(GLint key) {
	switch (key) {
	default: {
		return;
	}
	}
}

void specialKeyRelease(GLint key) {

}

//�����
void mousePress(int button, int state, int x, int y) {
	switch (button)
	{
		//����Ҽ�
	case 0: {
		//�Ҽ�̧��
		if (state == 1) {
			//if (role->getMovingStatus())return;
			////role->roleMoveByMouse(x, y);
			//PxVec3 position = ScenetoWorld(x, y);
			//Block road;
			//if (RayCast(position, PxVec3(0, 5, 0), road))
			//{
			//	PxVec3 blockPosition = road.getPosition();
			//	//role->roleMoveByMouse(PxVec3(blockPosition.x, role->getFootPosition().y, blockPosition.z));
			//	role->roleMoveByMouse(position);
			//}
			//else {
			//	std::cout << "���ɵ��";
			//}
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