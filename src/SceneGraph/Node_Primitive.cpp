#include "SceneGraph/Node_Primitive.h"

SSS_BEGIN
glm::mat4 TextPlane::_getTranslationMat4() const {
	glm::vec3 offset = _offset;
	auto texture = getTexture();
	if (texture) {
		auto const [w, h] = getTexture()->getCurrentDimensions();
		float const x = static_cast<float>(w) / 2.f;
		float const y = static_cast<float>(-h) / 2.f;
		offset += glm::vec3(x, y, 0);
	}
	return glm::translate(ModelBase::_getTranslationMat4(), offset);
}

Node_Block::Node_Block(SceneGraph* p_Sg): Node(p_Sg)
{
	_pos = glm::vec3{ 0, 0, 0 };
	_size = glm::vec3{ 0, 0, 0 };
}

glm::mat4 Node_Block::getLocalTransform() const
{
	//Rotation along the z axis with _pos translation
	glm::mat4 mat = glm::mat4(1.0);
	mat = glm::translate(mat, _pos);
	mat *= glm::rotate(rotation, glm::vec3(0, 0, 1));
	return mat;
}


bool Node_Block::checkCollision2D(glm::vec2 np2, glm::vec2 ns2) const
{
	//node pos np, node size ns
	glm::vec3 globPos = getGlobalTransform()[3];
	

	if (globPos.y + _size.y >= np2.y &&      // r1 top edge past r2 bottom
		globPos.y <= np2.y + ns2.y &&      // r1 bottom edge past r2 top
		globPos.x + _size.x >= np2.x &&      // r1 right edge past r2 left
		globPos.x <= np2.x + ns2.x)        // r1 left edge past r2 right
	{
		LOG_MSG("Collision !!");
		return true;
	}

	return false;
}

glm::vec3 Node_Block::center() const
{ 
	glm::vec3 globPos = getGlobalTransform()[3];
	return globPos + glm::vec3(_size.x / 2.f, _size.y / 2.f, 0.f);
}

glm::vec3 Node_Block::getPosition() const
{
	return getGlobalTransform()[3];
}

glm::mat4 Node_Block::getGlobalTransform() const
{
	if (_inherited_transform == false || _parent == 0)
		return getLocalTransform();

	Node_Block* p_pNode = (Node_Block*)_sg->at(_parent);
	return p_pNode->getGlobalTransform() * getLocalTransform();
}
SSS_END