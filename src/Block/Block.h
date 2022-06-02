#include <ctype.h>
#include <string>
#include "PxPhysicsAPI.h"
using namespace physx;
using namespace std;
//普通盒子的高度
const int commonBoxHeight = 10;

enum BlockType
{
	error = -1,
	ground = 1
};


//方块基类，后续可拓展实现
class Block {
private:
	PxReal halfExtent = 0;
	//高度层数
	PxReal height = commonBoxHeight;
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
