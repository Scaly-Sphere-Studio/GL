//#pragma once
//
//#include <SSS/Commons.hpp>
//#include <SSS/Math.hpp>
//
//#include "SSS/GL.hpp"
//#include "SceneGraph/scenegraph.h"
//
//
////#include "Text_data.h"
//
//class SceneGraph;
//
//SSS_BEGIN
//
//// Utility for Text Nodes
//
//class TextPlane : public SSS::GL::PlaneTemplate<TextPlane> {
//	friend class SharedClass;
//protected:
//	virtual glm::mat4 _getTranslationMat4() const override;
//private:
//	TextPlane() = default;
//	glm::vec3 _offset = glm::vec3{ 0 };
//	Node* _parent = nullptr;
//public:
//	inline void setParent(Node* node) { _parent = node; };
//	inline auto getParent() const noexcept { return _parent; };
//	inline glm::vec3 getOffset() const noexcept { return _offset; };
//	void setOffset(glm::vec3 offset) {};
//};
//
//// 3D Node for grouping elements
//class Node_Block : public Node
//{
//public:
//	Node_Block() = default;
//	Node_Block(SceneGraph* p_Sg);
//	std::string name() const { return "Block"; };
//
//	glm::vec3 _pos;		// Node position and translations
//	glm::vec3 _size;	// Node bounding box
//	int _type = 1;
//
//	float rotation = 0.f;
//
//	//transforms
//	virtual void _subjectUpdate(SSS::Subject const& subject, int event_id) override {};
//	virtual void update() {};
//	glm::mat4 getLocalTransform() const; // Rotation*Translation*Scale
//	glm::mat4 getGlobalTransform() const;
//
//
//	virtual glm::vec3 center() const;
//
//	void translate(const glm::vec3& translation) { _pos += translation; update();};
//	void setPosition(const glm::vec3& newPos) { _pos = newPos; update(); };
//	glm::vec3 getPosition() const;
//
//	void setZ(const float& depth) { _pos.z = depth; update(); };
//
//	virtual bool checkCollision2D(glm::vec2 pos, glm::vec2 size) const;
//private:
//};
//
//SSS_END