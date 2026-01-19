#include "SceneGraph/Node_Input.h"

SSS_BEGIN
Node_MouseInput::Node_MouseInput(const glm::vec3& pos, const float& r)
{
	_pos	= pos;
	_radius = r;
	_borderWidth = 3.f;
	build();
}

void Node_MouseInput::build()
{

	glm::vec2 begin		= _pos;
	glm::vec2 end		= begin + glm::vec2(0, _radius);


	glm::vec4 color			= glm::vec4(0.87, 0.53, 0.56, 1.0);
	glm::vec4 emptyColor	= glm::vec4(0.118, 0.118, 0.118, 1.0);
	glm::vec4 background	= glm::vec4(0.27f, 0.27f, 0.27f, 1.0f);

	SSS::UIPrimitive mouse;
	// Background
	mouse.shapeId = SSS::sdSegment;
	mouse.pos = begin;
	mouse.pos2 = end;
	mouse.size.r = _radius;
	mouse.color = glm::vec4(0.0);
	mouse.border = glm::vec4(1.0);
	mouse.borderWidth = _borderWidth;
	mouse.blendMode = SSS::GROUP;
	prims.push_back(mouse);

	mouse.shapeId = SSS::sdOrientedBox;
	mouse.pos = begin + glm::vec2(-_radius*0.55, -(_radius*1.1)*0.5);
	mouse.pos2 = mouse.pos + glm::vec2(0, _radius - 3*_borderWidth);
	mouse.size.r = _radius * 2.5f;
	mouse.blendMode = SSS::GROUP| SSS::SUBTRACT;
	prims.push_back(mouse);

	float y = mouse.pos2.y;

	// Center button
	mouse.shapeId = SSS::sdSegment;
	mouse.pos = begin + glm::vec2(0, -_radius/4);
	mouse.pos2 = mouse.pos + glm::vec2(0, _radius *0.4);
	mouse.size.r = _radius * 0.15f;
	mouse.blendMode = SSS::DEFAULT;
	prims.push_back(mouse);

	// left click
	mouse.shapeId = SSS::sdSegment;
	mouse.pos = begin;
	mouse.pos2 = end;
	mouse.size.r = _radius;
	mouse.blendMode = SSS::GROUP;
	prims.push_back(mouse);

	mouse.shapeId = SSS::sdSegment;
	mouse.pos = begin + glm::vec2(0, -_radius / 4);
	mouse.pos2 = mouse.pos + glm::vec2(0, _radius * 0.4);
	mouse.size.r = _radius * 0.15f + 2.f*_borderWidth;
	mouse.blendMode = SSS::SUBTRACT;
	prims.push_back(mouse);

	mouse.shapeId = SSS::sdOrientedBox;
	mouse.pos = begin + glm::vec2(-_radius / 2 , -_radius/2);
	mouse.pos2 = glm::vec2(mouse.pos.x , y - _borderWidth );
	mouse.size.r = _radius - _borderWidth ;
	mouse.blendMode = SSS::INTERSECT | SSS::GROUP;
	prims.push_back(mouse);

	// Right click
	mouse.shapeId = SSS::sdSegment;
	mouse.pos = begin;
	mouse.pos2 = end;
	mouse.size.r = _radius;
	mouse.blendMode = SSS::GROUP;
	prims.push_back(mouse);

	mouse.shapeId = SSS::sdSegment;
	mouse.pos = begin + glm::vec2(0, -_radius / 4);
	mouse.pos2 = mouse.pos + glm::vec2(0, _radius * 0.4);
	mouse.size.r = _radius * 0.15f + 2.f*_borderWidth;
	mouse.blendMode = SSS::SUBTRACT;
	prims.push_back(mouse);

	mouse.shapeId = SSS::sdOrientedBox;
	mouse.pos = begin + glm::vec2(_radius / 2, -_radius / 2);
	mouse.pos2 = glm::vec2(mouse.pos.x, y - _borderWidth);
	mouse.size.r = _radius - _borderWidth;
	mouse.blendMode = SSS::INTERSECT | SSS::GROUP;
	prims.push_back(mouse);

	// Side buttons
	mouse.shapeId = SSS::sdRoundedBox;
	mouse.size = glm::vec2(_radius*.03, 1.5*_radius*.1);
	mouse.pos = begin + glm::vec2(-0.5 *_radius - 2*_borderWidth - (mouse.size.x)/2, _radius*0.4);
	mouse.pos2 = glm::vec2(0, 0);
	mouse.pos3 = glm::vec2(5, 0);
	mouse.blendMode = SSS::DEFAULT;
	prims.push_back(mouse);

	mouse.shapeId = SSS::sdRoundedBox;
	mouse.pos = mouse.pos + glm::vec2(0, 2*mouse.size.y + _borderWidth );
	mouse.pos2 = glm::vec2(5, 0);
	mouse.pos3 = glm::vec2(0, 0);
	mouse.size = glm::vec2(_radius * .03, 1.5 * _radius * .1);
	mouse.blendMode = SSS::DEFAULT;
	prims.push_back(mouse);

	// trackball
	mouse.shapeId = SSS::sdCircle;
	mouse.pos = begin + glm::vec2(0, _radius);
	mouse.size.r = _radius * 0.2f;
	mouse.border = glm::vec4(0.5);
	mouse.blendMode = SSS::DEFAULT;
	prims.push_back(mouse);

	mouse.shapeId = SSS::sdCircle;
	mouse.size.r = _radius * 0.1f;
	mouse.color = glm::vec4(1.0);
	mouse.borderWidth = 0.0f;
	mouse.color.a = 0.7f;
	prims.push_back(mouse);

	origin = mouse.pos;
}



void Node_MouseInput::_subjectUpdate(SSS::Subject const& subject, int event_id) {
	if (_hidden)
		return;

	if (event_id == EVENT_ID("SSS_WINDOW_MOUSE_POSITION")) {
		const SSS::GL::Window& window = static_cast<SSS::GL::Window const&>(subject);
		int c_y, c_x;
		window.getCursorPos(c_x, c_y);

		glm::vec2 delta = glm::vec2(c_x, c_y) - old_cpos;
		float r = std::clamp(glm::sqrt(delta.x * delta.x + delta.y * delta.y), 0.f, _radius * 0.2f);
		float theta = glm::atan2(delta.y, delta.x);

		prims[12].pos = origin + glm::vec2(r * glm::cos(theta), r * glm::sin(theta));

		old_cpos = glm::vec2(c_x, c_y);
	}

	if (event_id == EVENT_ID("SSS_WINDOW_MOUSE_INPUT")) {
		const SSS::GL::Window& window = static_cast<SSS::GL::Window const&>(subject);
		auto const& clicks = window.getClickInputs();

		// indexes
		// 2 center
		// 5 left click
		// 8 right click
		// 9 button up
		// 10 button down 
		auto showIpt = [&](int ipt, int index) {
			if (clicks[ipt].is_pressed()) {
				prims[index].color = glm::vec4(1.0);
			}

			if (clicks[ipt].is_released()) {
				prims[index].color = glm::vec4(0.0);
			}

			return;
		};

		int c_y, c_x;
		window.getCursorPos(c_x, c_y);

		showIpt(GLFW_MOUSE_BUTTON_1, 5);
		showIpt(GLFW_MOUSE_BUTTON_2, 8);
		showIpt(GLFW_MOUSE_BUTTON_3, 2);
		showIpt(GLFW_MOUSE_BUTTON_4, 10);
		showIpt(GLFW_MOUSE_BUTTON_5, 9);
	}
}

SSS_END;