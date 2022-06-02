#include "../Block/Block.h"

PxReal Block::getHalfExtent() {
	return this->halfExtent;
}
PxReal Block::getHeight() {
	return this->height;
}
PxVec3 Block::getPosition() {
	return this->position;
}
BlockType Block::getBlockType() {
	return this->type;
}
string Block::getName() {
	return this->name;
}

void Block::setHalfExtent(PxReal halfExtent) {
	this->halfExtent = halfExtent;
}
void Block::setHeight(PxReal height) {
	this->height = height;
}
void Block::setPosition(PxVec3 position) {
	this->position = position;

}
void Block::setBlockType(BlockType type) {
	this->type = type;
}
void Block::setName(string name) {
	this->name = name;
}