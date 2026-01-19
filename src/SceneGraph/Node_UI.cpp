#include "SceneGraph/Node_UI.h"

#define BLACK glm::vec4(0,0,0,1)
#define WHITE glm::vec4(1,1,1,1)
#define FOCUS_INTENSITY		0.5f
#define HOVER_INTENSITY		0.3f
#define HOVER_RADIUS_EFFECT	0.4f

SSS_BEGIN

void Node_UI::boundingBox()
{
	//BB
	SSS::UIPrimitive bb;
	bb.shapeId = SSS::sdSegment;
	bb.pos = _pos;
	bb.pos2 = _pos + glm::vec3(_size.x, 0.0, 0.0);
	bb.size.x = 3;
	bb.color = glm::vec4(0.0, 0.0, 0.0, 0.5);
	prims.push_back(bb);

	bb.shapeId = SSS::sdSegment;
	bb.pos = _pos + glm::vec3(_size.x, 0.0, 0.0);
	bb.pos2 = _pos + _size;
	prims.push_back(bb);

	bb.shapeId = SSS::sdSegment;
	bb.pos = _pos + _size;
	bb.pos2 = _pos + glm::vec3(0.0, _size.y, 0.0);
	prims.push_back(bb);

	bb.shapeId = SSS::sdSegment;
	bb.pos = _pos + glm::vec3(0.0, _size.y, 0.0);
	bb.pos2 = _pos;
	prims.push_back(bb);
}

void Node_UI::_register()
{
	REGISTER_EVENT("NODE_UI_HOVER");
	REGISTER_EVENT("NODE_UI_MOUSE_LEFT");
}

void Node_UI::setColor(const std::string& hex)
{
	setColor(SSS::RGBA_f(hex));
}

void Node_UI::setVerticalOffset(const int& keyVO)
{
	_vOffset = keyVO;
	Node_Block* UIelem = static_cast<Node_Block*>(_sg->at(_vOffset));
	_pos.y += UIelem->_pos.y;
	_pos.y -= UIelem->_size.y;

	translateElem();
}

void Node_UI::setHorizontalOffset(const int& keyHO)
{
	_hOffset = keyHO;
	Node_Block* UIelem = static_cast<Node_Block*>(_sg->at(_hOffset));
	_pos = UIelem->_pos;
	_pos.x += UIelem->_size.x;

	translateElem();
}

void Node_UI::setDepthOffset(const int& keyDO)
{
	_dOffset = keyDO;
	Node_Block* UIelem = static_cast<Node_Block*>(_sg->at(_dOffset));
	_pos.z += UIelem->_pos.z;
	_pos.z += UIelem->_size.z;

	translateElem();
}

void Node_UI::update()
{
	translateElem();
}

bool Node_UI::_checkPointCollision(glm::vec2 const& pt)
{
	glm::vec3 absPos = getGlobalTransform()[3]; // Translation from Transform

	if ((pt.x <= (absPos.x + _size.x) && (pt.x >= absPos.x)
		&& pt.y >= (absPos.y + _size.y) && (pt.y <= absPos.y)))
	{
		// Cursor enter the UI elem
		if (!_hover)
		{
			EMIT_EVENT("NODE_UI_HOVER");
			SSS::log_msg("Hover the node " + std::to_string(_key));
		}

		_hover = true;
		return true; // Pt in the UI elem
	}

	// Cursor leave the UI elem
	if (_hover)
	{
		EMIT_EVENT("NODE_UI_MOUSE_LEFT");
		SSS::log_msg("Leave the node " + std::to_string(_key));
	}

	_hover = false;
	return false;
}


/* -------------------------------------------------------
---------------------- TEXT NODES ------------------------
--------------------------------------------------------*/

void Node_Text::_register()
{
	REGISTER_EVENT("NODE_TEXT_RESIZE");
	REGISTER_EVENT("NODE_TEXT_CONTENT_UPDATE");
}

Node_Text::Node_Text(SceneGraph* p_Sg, const std::string& s, const SSS::GUI_Layout& lyt)
	:Node_UI(p_Sg)
{
	SSS::TR::Format fmt = lyt._fmt;
	fmt.charsize = 58;
	auto area = SSS::TR::Area::create();
	auto plane = TextPlane::create(SSS::GL::Texture::create(area));
	//plane->setBox(nullptr);
	_observe(*plane->getTexture());


	glm::vec4 tex_col = SSS::RGBA_f(BLACK).to_HSL();
	glm::vec4 bg_col = tex_col;

	tex_col.b = 0.3f;
	fmt.text_color = SSS::RGBA_f::from_HSL(tex_col);

	bg_col.b -= 0.15f;
	area->setClearColor(SSS::RGBA_f::from_HSL((bg_col)));
	area->setClearColor(static_cast<SSS::RGBA32>(SSS::RGBA_f{ BLACK }));
	area->setFocusable(true);
	area->setWrapping(true);
	area->setMargins(lyt._marginv, lyt._marginh);
	area->setWrappingMaxWidth(_maxStrSize);
	area->setFormat(fmt);
	area->parseString(s);

	//Create the model
	plane->translate(_pos);

	plane->setHitbox(SSS::GL::Plane::Hitbox::Full);
	model = plane;

	_size_update();

	_sg->_rd->addPlane(plane);
	_sg->emplace(this);
}

void Node_Text::_subjectUpdate(SSS::Subject const& subject, int event_id)
{
	if (_hidden)
		return;

	if (event_id == EVENT_ID("SSS_TEXTURE_CONTENT")) {
		auto [w, h] = model->getTexture()->getCurrentDimensions();
		model->setScaling(glm::vec3(static_cast<float>(std::min(w, h))));


		EMIT_EVENT("NODE_TEXT_RESIZE");
		return;
	}


	auto& texture = static_cast<SSS::GL::Texture const&>(subject);
	if (texture.getType() == SSS::GL::Texture::Type::Text &&
		texture.getTextArea() && texture.getTextArea()->isFocused())
	{
		auto area = model->getTextArea();
		if (!area) return;
		if (area->getUsedWidth() == _size.x)
			return;

		_size_update();
		EMIT_EVENT("NODE_TEXT_RESIZE");
	}
}


Node_Text::~Node_Text()
{
	clear();
}

void Node_Text::clear()
{
	_sg->_rd->removePlane(model);
	model.reset();
	EMIT_EVENT("NODE_TEXT_CONTENT_UPDATE");
}

void Node_Text::setWrappingMin(const int& min)
{
	model->getTextArea()->setWrappingMinWidth(min);
}

void Node_Text::setMaxStrSize(const int maxSize)
{
	_maxStrSize = maxSize;
}

void Node_Text::setTextColor(const  SSS::RGBA_f& col)
{
	SSS::TR::Format fmt = model->getTextArea()->getFormat();
	fmt.text_color = col;

	model->getTextArea()->setFormat(fmt);
}

void Node_Text::setBackgroundColor(const  SSS::RGBA_f& bgCol)
{
	model->getTextArea()->setClearColor(bgCol);
}

void Node_Text::_size_update()
{
	glm::vec3 const old_size = _size;
	_size = glm::vec3(0);

	model->setOffset(glm::vec3(0, -_size.y, _pos.z));
	auto [w, h] = model->getTexture()->getCurrentDimensions();
	model->setScaling(glm::vec3(static_cast<float>(std::min(w, h))));
	if (auto area = model->getTextArea(); area)
	{
		w = area->getUsedWidth();
		h = area->getHeight();
	}
	_size.x = std::max(static_cast<float>(w), _size.x);
	_size.y += static_cast<float>(h);

	// Set min width
	if (_size == old_size)
		return;

	if (_size.x != old_size.x) {
		int w = static_cast<int>(_size.x);
		if (_maxStrSize - static_cast<int>(_size.x) < 10)
			w = _maxStrSize;

		if (auto area = model->getTextArea(); area)
			area->setWrappingMinWidth(w);
	}
}


void Node_Text::update()
{
	glm::mat4 transform = getGlobalTransform(); // your transformation matrix.
	glm::vec3 scale;
	glm::quat rotation;
	glm::vec3 translation;
	glm::vec3 skew;
	glm::vec4 perspective;
	glm::decompose(transform, scale, rotation, translation, skew, perspective);
	glm::vec3 rot = glm::eulerAngles(rotation);

	model->setTranslation(translation);
	model->setRotation(glm::degrees(rot));
}

void Node_Text::parseText(const std::string& str)
{
	model->getTextArea()->parseString(str);
}


/* -------------------------------------------------------
----------------------- SLIDERS --------------------------
--------------------------------------------------------*/

Node_Slider::~Node_Slider()
{
}

Node_Slider::Node_Slider(const int &min, const int &max, int* cur, const glm::vec3& pos)
{
	_pos = pos;
	_radius = 15.0/2.0;
	_borderWidth = 3.0;
	_len = 300;

	_min = min;
	_max = max;
	_current = cur;

	build();
}

void Node_Slider::build()
{
	prims.clear();

	SSS::UIPrimitive ui;
	ui.shapeId = SSS::sdSegment;
	ui.size.x = std::max(2.f, _radius *0.5f);
	ui.size.y = 0;
	ui.pos = _pos;
	ui.pos2 = ui.pos + glm::vec2(ui.pos.x + _len, 0);
	ui.color = glm::vec4(1.0);
	ui.blendMode = SSS::GROUP;
	prims.push_back(ui);

	ui.shapeId = SSS::sdCircle;
	ui.pos = _pos;
	ui.size.x = _radius + _borderWidth;
	ui.blendMode = SSS::GROUP| SSS::SUBTRACT;
	prims.push_back(ui);

	//focus circle 
	ui.shapeId = SSS::sdCircle;
	ui.size.x = _radius*2.f;
	ui.blendMode = SSS::DEFAULT;
	ui.color = glm::vec4(1.0, 1.0, 1.0, 0.5);
	prims.push_back(ui);

	ui.shapeId = SSS::sdCircle;
	ui.size.x = _radius;
	ui.color = glm::vec4(1.0);
	prims.push_back(ui);

	_pos = { prims[0].pos.x - prims[3].size.x,  prims[0].pos.y - prims[3].size.x, 0.0 };
	_size = { prims[0].pos2.x - prims[0].pos.x + 2 * prims[3].size.x, 2 * prims[3].size.x, 0.0 };


	//BB
	//boundingBox();
}


void Node_Slider::clear()
{

}

void Node_Slider::getCursorPos(const float& x, const float& y)
{
	float cx = std::clamp(x, prims[0].pos.x, prims[0].pos2.x);
	prims[2].pos.x = cx; 
	prims[1].pos.x = cx;
	prims[3].pos.x = cx;

	float progress = (cx - prims[0].pos.x) / (prims[0].pos2.x - prims[0].pos.x);

	*_current = (_max - _min) * progress + _min;

}

bool Node_UI::checkCollision(const glm::vec3& pos)
{
	if (pos.x >= _pos.x && pos.x <= (_pos.x + _size.x)
		&& pos.y >= _pos.y  && pos.y <= (_pos.y + _size.y)) {
		return true;
	}
	return false;
}


void Node_Slider::_subjectUpdate(SSS::Subject const& subject, int event_id)
{
	if (_hidden)
		return;

	if (event_id == EVENT_ID("SSS_WINDOW_MOUSE_POSITION")) {
		const SSS::GL::Window &window = static_cast<SSS::GL::Window const&>(subject);
		int c_y, c_x;
		window.getCursorPos(c_x, c_y);

		_hover = checkCollision(glm::vec3(c_x, c_y, 0));

		if (!_focus) {
			float dist = std::clamp(glm::distance(prims[2].pos, glm::vec2(c_x, c_y)), 0.f, 2*prims[2].size.r);
			float factor = glm::smoothstep(0.f, 1.0f, 1.0f - dist / prims[2].size.r);
			prims[2].color.a = FOCUS_INTENSITY * factor;
		}

		if (_held) {
			getCursorPos((float)c_x, (float)c_y);
			prims[2].color.a = 0.6f;
		}
	}

	if (event_id == EVENT_ID("SSS_WINDOW_MOUSE_INPUT")) {
		const SSS::GL::Window& window = static_cast<SSS::GL::Window const&>(subject);
		auto const& clicks = window.getClickInputs();

		int c_y, c_x;
		window.getCursorPos(c_x, c_y);
		if (clicks[GLFW_MOUSE_BUTTON_1].is_pressed()) {
			_focus = _hover;
			_held = _hover;
		}
		if (clicks[GLFW_MOUSE_BUTTON_1].is_released()) {
			_held = false;
			if (!_focus) {
				float d = glm::distance(prims[2].pos, glm::vec2(c_x, c_y));
				float alpha = std::clamp(1.0f - glm::smoothstep(HOVER_RADIUS_EFFECT, _radius * 1.2f, d), 0.f, HOVER_INTENSITY);
				prims[2].color.a = alpha;
			}
		}

		if (_held) {
			getCursorPos((float)c_x, (float)c_y);
			prims[2].color.a = FOCUS_INTENSITY;
		}
	}
}


/* -------------------------------------------------------
----------------------- TOGGLES --------------------------
--------------------------------------------------------*/


Node_Toggle::Node_Toggle(bool* b, const glm::vec3& pos)
{
	_active = b;
	_pos = pos;
	_radius = 15;
	_borderWidth = 3.0;
	build();
}

void Node_Toggle::build() {
	prims.clear();

	glm::vec2 begin = _pos;
	glm::vec2 end = begin + glm::vec2( _radius*0.7, 0);


	glm::vec4 color = glm::vec4(0.87, 0.53, 0.56, 1.0);
	glm::vec4 emptyColor = glm::vec4(0.118, 0.118, 0.118, 1.0);
	glm::vec4 background = glm::vec4(0.27f, 0.27f, 0.27f, 1.0f);

	SSS::UIPrimitive toggleButton;
	// Background
	toggleButton.shapeId = SSS::sdSegment;
	toggleButton.pos = begin;
	toggleButton.pos2 = end;
	toggleButton.cornerRadius = 0.02f;
	toggleButton.size.r = _radius * 2;
	toggleButton.color = glm::vec4(1.0);
	toggleButton.color.a = 0.0;
	toggleButton.blendMode = 0;
	prims.push_back(toggleButton);

	// Background
	toggleButton.shapeId = SSS::sdSegment;
	toggleButton.pos = begin;
	toggleButton.pos2 = end;
	toggleButton.cornerRadius = 0.02f;
	toggleButton.size.r = _radius;
	toggleButton.color = glm::vec4(0.0);
	toggleButton.border = glm::vec4(1.0);
	toggleButton.borderWidth = _borderWidth;
	toggleButton.blendMode = 0;
	prims.push_back(toggleButton);


	// Button
	toggleButton.shapeId = SSS::sdCircle;
	toggleButton.cornerRadius = 0;
	toggleButton.pos = glm::mix(begin, end, (float)*_active);
	toggleButton.size.r = _radius * 0.35f;
	toggleButton.color = glm::vec4(1.0);
	toggleButton.borderWidth = 0.0;
	prims.push_back(toggleButton);

	_size = glm::vec3(end.x - begin.x + _radius, _radius, 0);
	_pos = glm::vec3(begin.x - _radius/2, begin.y - _radius/2, 0.f);

	//BB
	//boundingBox();
}

void Node_Toggle::_subjectUpdate(SSS::Subject const& subject, int event_id) {
	if (_hidden)
		return;

	if (event_id == EVENT_ID("SSS_WINDOW_MOUSE_POSITION")) {
		const SSS::GL::Window& window = static_cast<SSS::GL::Window const&>(subject);
		int c_y, c_x;
		window.getCursorPos(c_x, c_y);

		_hover = checkCollision(glm::vec3(c_x, c_y, 0));

		if (!_focus) {
			float d = glm::distance(prims[2].pos + glm::vec2(_radius * .35, 0), glm::vec2(c_x, c_y));
			float alpha = std::clamp(1.0f - glm::smoothstep(HOVER_RADIUS_EFFECT, _radius*1.2f, d), 0.f, HOVER_INTENSITY);
			prims[0].color.a = alpha;
		}
		return;
	}

	if (event_id == EVENT_ID("SSS_WINDOW_MOUSE_INPUT")) {
		const SSS::GL::Window& window = static_cast<SSS::GL::Window const&>(subject);
		auto const& clicks = window.getClickInputs();

		int c_y, c_x;
		window.getCursorPos(c_x, c_y);

		if (clicks[GLFW_MOUSE_BUTTON_1].is_pressed()) {
			_focus = _hover;
			if (_hover) {
				*_active ^= true;
				prims[2].pos = glm::vec3(prims[1].pos, 0) + glm::vec3((_radius * 0.7f) * (float)(*_active), 0, 0);
				prims[0].color.a = FOCUS_INTENSITY;
			}
		}

		if (clicks[GLFW_MOUSE_BUTTON_1].is_released()) {
			if (!_focus) {
				float d = glm::distance(prims[2].pos + glm::vec2(_radius * .35, 0), glm::vec2(c_x, c_y));
				float alpha = std::clamp(1.0f - glm::smoothstep(HOVER_RADIUS_EFFECT, _radius * 1.2f, d), 0.f, HOVER_INTENSITY);
				prims[0].color.a = alpha;
			}
		}
		return;
	}
}


/* -------------------------------------------------------
----------------------- CHECKBOX  ------------------------
--------------------------------------------------------*/


Node_CheckBox::Node_CheckBox(bool* b, const glm::vec3& pos)
{
	_active = b;
	_pos = pos;
	_radius = 15;
	_borderWidth = 2.0;
	build();
}

void Node_CheckBox::build() {

	prims.clear();
	float factor = 0.6f;
	glm::vec2 begin = _pos;
	glm::vec2 end = begin + glm::vec2(_radius, 0);
	glm::vec4 color = glm::vec4(0.87, 0.53, 0.56, 1.0);
	glm::vec4 emptyColor = glm::vec4(0.118, 0.118, 0.118, 1.0);
	glm::vec4 background = glm::vec4(0.27f, 0.27f, 0.27f, 1.0f);

	SSS::UIPrimitive cb;

	// Focus
	cb.shapeId = SSS::sdRoundedBox;
	cb.pos = begin + glm::vec2(_radius * 0.5f, 0);
	cb.size = glm::vec2(_radius*0.9f);
	cb.pos2 = glm::vec2(5, 5);
	cb.pos3 = glm::vec2(5, 5);
	cb.color = glm::vec4(1.0);
	cb.color.a = 0.0;
	cb.blendMode = SSS::DEFAULT;
	prims.push_back(cb);

	// Background
	cb.shapeId = SSS::sdOrientedBox;
	cb.pos = begin;
	cb.pos2 = end;
	cb.size.r = _radius;
	cb.color = glm::vec4(1.0);
	cb.border = glm::vec4(1.0);
	cb.borderWidth = 0;
	cb.blendMode = SSS::GROUP;
	prims.push_back(cb);

	cb.shapeId = SSS::sdOrientedBox;
	cb.pos.x = begin.x + _borderWidth;
	cb.pos2.x = end.x - _borderWidth;
	cb.size.r = _radius - 2* _borderWidth;
	cb.blendMode = SSS::SUBTRACT| SSS::GROUP;
	prims.push_back(cb);

	// Button
	cb.shapeId = SSS::sdOrientedBox;
	cb.cornerRadius = 0;
	cb.pos = begin + glm::vec2(2*_borderWidth,0);
	cb.pos2 = end - glm::vec2(2*_borderWidth, 0);
	cb.size.r = _radius - 4*_borderWidth;
	cb.color = glm::mix(glm::vec4(0.0), glm::vec4(1.0), (float)*_active);
	cb.blendMode = SSS::DEFAULT;
	cb.borderWidth = 0.0;
	prims.push_back(cb);

	_size = glm::vec3(_radius, _radius, 0);
	_pos = glm::vec3(_pos.x, _pos.y - _radius / 2, 0.f);

	//BB
	//boundingBox();
}

void Node_CheckBox::_subjectUpdate(SSS::Subject const& subject, int event_id) {
	if (_hidden)
		return;

	if (event_id == EVENT_ID("SSS_WINDOW_MOUSE_POSITION")) {
		const SSS::GL::Window& window = static_cast<SSS::GL::Window const&>(subject);
		int c_y, c_x;
		window.getCursorPos(c_x, c_y);

		_hover = checkCollision(glm::vec3(c_x, c_y, 0));


		if (!_focus) {
			float dist = std::clamp(glm::distance(prims[0].pos, glm::vec2(c_x, c_y)), 0.f, 2 * prims[0].size.r);
			float factor = glm::smoothstep(0.f, 1.0f, 1.0f - dist / prims[0].size.r);
			prims[0].color.a = FOCUS_INTENSITY * factor;
		}
		return;
	}

	if (event_id == EVENT_ID("SSS_WINDOW_MOUSE_INPUT")) {
		const SSS::GL::Window& window = static_cast<SSS::GL::Window const&>(subject);
		auto const& clicks = window.getClickInputs();

		int c_y, c_x;
		window.getCursorPos(c_x, c_y);
		if (clicks[GLFW_MOUSE_BUTTON_1].is_pressed()) {
			_focus = _hover;
			if (_hover) {
				*_active ^= true;
				prims[3].color = glm::mix(glm::vec4(0.0),  glm::vec4(1.0), (float)*_active);
				prims[0].color.a = FOCUS_INTENSITY;
			}
		}

		if (clicks[GLFW_MOUSE_BUTTON_1].is_released()) {
			if (!_focus) {
				float d = glm::distance(prims[2].pos, glm::vec2(c_x, c_y));
				float alpha = std::clamp(1.0f - glm::smoothstep(HOVER_RADIUS_EFFECT, _radius*1.2f, d), 0.f, HOVER_INTENSITY);
				prims[0].color.a = alpha;
			}
		}
		return;
	}
}

/* -------------------------------------------------------
--------------------- RADIO BUTTONS ----------------------
--------------------------------------------------------*/


void Node_RadioButton::_register()
{
	REGISTER_EVENT("SSS_RADIO_TOOK_FOCUS");
}

Node_RadioButton::Node_RadioButton(bool* b, const glm::vec3& pos) {
	_pos = pos;
	_radius = 15*0.5;
	_borderWidth = 4.0;
	_active = b;
	build(_pos);
}

void Node_RadioButton::observeRadio(Node_RadioButton &subject)
{
	_observe(subject);

	if (subject.isActive()) {
		*_active = false;
	}
	build(_pos);
}

void Node_RadioButton::build(const glm::vec2& begin) {
	prims.clear();
	float factor = 0.6f;

	glm::vec4 color = glm::vec4(0.87, 0.53, 0.56, 1.0);
	glm::vec4 emptyColor = glm::vec4(0.118, 0.118, 0.118, 1.0);
	glm::vec4 background = glm::vec4(0.27f, 0.27f, 0.27f, 1.0f);

	SSS::UIPrimitive radio;
	// Background
	radio.shapeId = SSS::sdCircle;
	radio.pos = begin;
	radio.size.r = _radius;
	radio.color = glm::vec4(0.0);
	radio.border = glm::vec4(1.0);
	radio.borderWidth = _borderWidth;
	radio.blendMode = SSS::DEFAULT;
	prims.push_back(radio);

	// Button
	radio.shapeId = SSS::sdCircle;
	radio.size.r = std::max(_radius - _borderWidth, _radius*factor);
	radio.color = glm::mix(glm::vec4(0.0), glm::vec4(1.0), (float)*_active);
	radio.borderWidth = 0.0;
	prims.push_back(radio);

	_pos = glm::vec3(begin.x - _radius , begin.y - _radius, 0.f);
	_size = glm::vec3(_radius * 2, _radius * 2, 0);

	// Focus
	radio.shapeId = SSS::sdCircle;
	radio.size = glm::vec2(_radius  * 2);
	radio.pos2 = glm::vec2(5, 5);
	radio.pos3 = glm::vec2(5, 5);
	radio.color = glm::vec4(1.0, 1.0, 1.0, 0.0);
	radio.blendMode = SSS::DEFAULT;
	prims.push_back(radio);

	//boundingBox();
}

void Node_RadioButton::_subjectUpdate(SSS::Subject const& subject, int event_id) {
	if (_hidden)
		return;

	if (event_id == EVENT_ID("SSS_RADIO_TOOK_FOCUS")) {
		_focus = false;
		*_active = false;
		prims[1].color = glm::mix(glm::vec4(0.0), glm::vec4(1.0), (float)*_active);
		return;
	}

	if (event_id == EVENT_ID("SSS_WINDOW_MOUSE_POSITION")) {
		const SSS::GL::Window& window = static_cast<SSS::GL::Window const&>(subject);
		int c_y, c_x;
		window.getCursorPos(c_x, c_y);

		_hover = checkCollision(glm::vec3(c_x, c_y, 0));

		if (!_focus) {
			float dist = std::clamp(glm::distance(prims[0].pos, glm::vec2(c_x, c_y)), 0.f, 2 * prims[0].size.r);
			float factor = glm::smoothstep(0.f, 1.0f, 1.0f - dist / prims[0].size.r);
			prims[2].color.a = FOCUS_INTENSITY * factor;
		}
		return;
	}

	if (event_id == EVENT_ID("SSS_WINDOW_MOUSE_INPUT")) {
		const SSS::GL::Window& window = static_cast<SSS::GL::Window const&>(subject);
		auto const& clicks = window.getClickInputs();

		int c_y, c_x;
		window.getCursorPos(c_x, c_y);

		if (clicks[GLFW_MOUSE_BUTTON_1].is_pressed()) {
			_focus = _hover;
			if (_hover) {
				EMIT_EVENT("SSS_RADIO_TOOK_FOCUS");
				*_active = true;
				prims[1].color = glm::mix(glm::vec4(0.0), glm::vec4(1.0), (float)*_active);
				prims[2].color.a = FOCUS_INTENSITY;
			}
		}

		if (clicks[GLFW_MOUSE_BUTTON_1].is_released()) {
			if (!_focus) {
				float d = glm::distance(prims[2].pos, glm::vec2(c_x, c_y));
				float alpha = std::clamp(1.0f - glm::smoothstep(HOVER_RADIUS_EFFECT, _radius * 1.2f, d), 0.f, HOVER_INTENSITY);
				prims[0].color.a = alpha;
			}
		}
		return;
	}
}

SSS_END;