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

// 右键鼠标按下
bool press = false;

extern bool helpMenu;

// 鼠标点击时的坐标
int mouseX, mouseY;

// 提示字符的位置（测试用）
int textX = 0, textY = 0;

int animationTick = 0;

//计算由于角色旋转需要的附加旋转角度
void getAdditionalAngleRadians();

//角色不能操作移动的状态
bool roleCannotOperate();

//按键设置
void keyPress(unsigned char key, const PxTransform& camera)
{
	if (role->nowCheckpoint == 8) {
		return;
	}

	switch (toupper(key))
	{
		//角色跳跃按键：空格
	case ' ':
	{
		if (role->tryJump(false)) {
			soundtool.playSound("jumpLoading.wav");
		}
		break;
	}
	//角色下蹲按键：Z
	case 'Z':
	{
		if (!role->getCrouch()) {
			animation.setAnimation("crouching");
			role->roleCrouch();
		}
		
		break;
	}
	//角色拾取/放置道具：E
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
		//初始位置
		role->setFootPosition(checkpoints[0]);
		break;
	}
	case '2': {
		//摆锤前位置
		role->setFootPosition(checkpoints[1]);
		break;
	}
	case '3': {
		//摩天轮前位置
		role->setFootPosition(checkpoints[2]);
		break;
	}
	case '4': {
		//旋转路关卡前位置
		role->setFootPosition(checkpoints[3]);
		break;
	}
	case '5': {
		//迷宫前位置
		role->setFootPosition(checkpoints[4]);
		break;
	}
	case '6': {
		//迷宫出口位置 
		role->setFootPosition(checkpoints[5]);
		break;
	}
	case '7': {
		//旋转杆关卡角落位置
		role->setFootPosition(checkpoints[6]);
		break;
	}
	case '8': {
		//游泳池位置
		role->setFootPosition(checkpoints[7]);
		break;
	}
	case '9': {
		//终点位置
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

//特殊键设置
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

//鼠标点击
void mousePress(int button, int state, int x, int y) {
	switch (button)
	{
		//点击右键
	case 0: {
		//右键抬起
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

//角色转动时，模型也跟随角色face Dir 向量方向旋转
void getAdditionalAngleRadians() {
	//cout << role->getFaceDir().x << "？？" << role->getFaceDir().y << "!!" << role->getFaceDir().z << endl;
	
	if (role->getFaceDir().z == 1) //前方
	{
		animation.changeOrientation(PxQuat(0, PxVec3(0, 1, 0)));
	}
	else if (role->getFaceDir().z == -1) //后方
	{
		animation.changeOrientation(PxQuat(PxPi, PxVec3(0, 1, 0)));
	}
	else if (role->getFaceDir().x == 1) //左方
	{
		PxTransform rotate = PxTransform(PxQuat(PxHalfPi, PxVec3(0, 1, 0)));
		animation.changeOrientation(PxQuat(PxHalfPi, PxVec3(0, 1, 0)));
	}
	else if (role->getFaceDir().x == -1) //右方
	{
		PxTransform rotate = PxTransform(PxQuat(PxHalfPi, PxVec3(0, 1, 0)));
		animation.changeOrientation(PxQuat(-PxHalfPi, PxVec3(0, 1, 0)));
	}
}

//角色不能操作移动的状态
bool roleCannotOperate() {
	if (animation.getCurrentAnimation() == "dying" || animation.getCurrentAnimation() == "sleeping"
		|| animation.getCurrentAnimation() == "useKey" || animation.getCurrentAnimation() == "notUseKey"
		|| animation.getCurrentAnimation() == "dancing") 
	{
		return false;
	}
	return true;
}