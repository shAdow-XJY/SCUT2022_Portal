#include <ctype.h>
#include "PxPhysicsAPI.h"
#include "../Role/Role.h"
#include "../Render/Camera.h"
using namespace physx;

extern Role* role;

// 右键鼠标按下
bool press = false;


// 鼠标点击时的坐标
int mouseX, mouseY;

// 提示字符的位置（测试用）
int textX = 0, textY = 0;


//按键设置
void keyPress(unsigned char key, const PxTransform& camera)
{
	switch (toupper(key))
	{
	case ' ':
	{
		role->tryJump(false);
		break;
	}
	case 'Z':
	{
		role->roleCrouch();
		break;
	}
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
		role->tryJump(true);
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

//特殊键设置
void specialKeyPress(GLint key) {
	switch (key) {
	default: {
		return;
	}
	}
}

void specialKeyRelease(GLint key) {

}

//鼠标点击
void mousePress(int button, int state, int x, int y) {
	switch (button)
	{
		//点击右键
	case 0: {
		//右键抬起
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
			//	std::cout << "不可点击";
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