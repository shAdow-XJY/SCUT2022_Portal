#include <ctype.h>
#include <string>
#include "PxPhysicsAPI.h"
using namespace physx;
using namespace std;

//���ڹ��˵�����
const PxFilterData collisionGroupIgnore(0, 0, 0, 1);  //������ײ��
const PxFilterData collisionGroup(1, 0, 0, 0); // �����ײ��

//���ӵĸ߶�
extern float boxHeight;

enum BlockType
{
	error = -1,
	ground = 1
};


//������࣬��������չʵ��
class Block {
private:
	PxReal halfExtent = 0;
	//�߶Ȳ���
	PxReal height = 2 * boxHeight;
	PxVec3 position = PxVec3(0, 0, 0);
	BlockType type = BlockType::error;
	string name = "";
public:

	Block() {};
	Block(PxReal halfExtent, string name) :halfExtent(halfExtent), name(name), type(BlockType::ground) {};

	PxReal getHalfExtent();
	PxReal getHeight();
	PxVec3 getPosition();
	BlockType getBlockType();
	string getName();

	void setHalfExtent(PxReal);
	void setHeight(PxReal);
	void setPosition(PxVec3);
	void setBlockType(BlockType);
	void setName(string);

};
