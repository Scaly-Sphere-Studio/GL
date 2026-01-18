//#pragma once
//
//#include "SSS/GL.hpp"
//#include <SSS/Commons.hpp>
//#include "SceneGraph/Node_UI.h"
//
//SSS_BEGIN
//
//class  Node_MouseInput : public Node_UI
//{
//public:
//	Node_MouseInput() = default;
//	Node_MouseInput(const glm::vec3 &pos, const float &r = 150.f);
//	~Node_MouseInput() = default;
//	std::string name() const { return "Mouse Input"; };
//	int _type = 4;
//
//	void build();
//
//	virtual void _subjectUpdate(SSS::Subject const& subject, int event_id) override;
//
//private:
//	float _radius;
//	void _size_update() {};
//
//	glm::vec2 origin;
//	glm::vec2 old_cpos;
//};
//
//SSS_END;