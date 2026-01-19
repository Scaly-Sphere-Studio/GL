#pragma once

#include "scenegraph.h"


//#include "Text_data.h"


class SceneGraph;
SSS_BEGIN

// Utility for Text Nodes
// TODO AFTER INTEGRATION
class SSS_GL_API TextPlane : public SSS::GL::PlaneTemplate<TextPlane> {
	friend class SharedClass;
protected:
	virtual glm::mat4 _getTranslationMat4() const override;
private:
	TextPlane() = default;
	glm::vec3 _offset = glm::vec3{ 0 };
	Node* _parent = nullptr;
public:
	inline void setParent(Node* node) { _parent = node; };
	inline auto getParent() const noexcept { return _parent; };
	inline glm::vec3 getOffset() const noexcept { return _offset; };
	void setOffset(glm::vec3 offset) {};
};

// 3D Node for grouping elements
class SSS_GL_API Node_Block : public Node
{
public:
	Node_Block() = default;
	Node_Block(SceneGraph* p_Sg);
	std::string name() const { return "Block"; };

	glm::vec3 _pos	= glm::vec3(0.f);		// Node position and translations
	glm::vec3 _size = glm::vec3(0.f);		// Node bounding box
	int _type = 1;

	float rotation = 0.f;

	//transforms
	virtual void _subjectUpdate(SSS::Subject const& subject, int event_id) override {};
	virtual void update() {};
	glm::mat4 getLocalTransform() const; // Rotation*Translation*Scale
	glm::mat4 getGlobalTransform() const;


	virtual glm::vec3 center() const;

	void translate(const glm::vec3& translation) { _pos += translation; update();};
	void setPosition(const glm::vec3& newPos) { _pos = newPos; update(); };
	glm::vec3 getPosition() const;

	void setZ(const float& depth) { _pos.z = depth; update(); };

	virtual bool checkCollision2D(glm::vec2 pos, glm::vec2 size) const;
};

SSS_END