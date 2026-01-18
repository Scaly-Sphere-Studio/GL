//#pragma once
//#include <SSS/Math.hpp>
//#include <SSS/Commons.hpp>
//
//#include "SSS/GL.hpp"
//#include "SSS/GL/Objects/Models/Shapes.hpp"
//
//#include "SceneGraph/scenegraph.h"
//#include "SceneGraph/Node_Primitive.h"
//
//
////#include "Text_data.h"
//
//
//SSS_BEGIN
//class SceneGraph;
//
//// Utility for Text Nodes
//
//class Node_UI : public Node_Block, public SSS::_EventRegistry<Node_UI>
//{
//public:
//	friend _EventRegistry<Node_UI>;
//	Node_UI() = default;
//	Node_UI(SceneGraph* p_Sg) :Node_Block(p_Sg) {};
//	std::string name() const { return "UI"; };
//	int _type = 2;
//
//	//transforms
//	void setVerticalOffset(const int& keyVO);		// Set the key Node to vertical offset
//	void setHorizontalOffset(const int& keyHO);		// Set the key Node to horizontal offset
//	void setDepthOffset(const int& keyDO);			// Set the key Node to depth offset
//	virtual void setWrappingMin(const int& min) {};
//
//	virtual void update();
//
//	virtual void setColor(const SSS::RGBA_f& col) { _color = col; };
//	virtual void setColor(const std::string& hex);
//
//	virtual SSS::RGBA_f getColor()const { return _color; };
//
//	virtual bool checkCollision(glm::vec3 pos) const { return false; };
//	bool _checkBoxCollision(glm::vec2 const& r1p, glm::vec2 const& r1s, glm::vec2 const& r2p, glm::vec2 const& r2s)
//	{
//		if (r1p.y + r1s.y >= r2p.y &&      // r1 top edge past r2 bottom
//			r1p.y <= r2p.y + r2s.y &&      // r1 bottom edge past r2 top
//			r1p.x + r1s.x >= r2p.x &&      // r1 right edge past r2 left
//			r1p.x <= r2p.x + r2s.x)        // r1 left edge past r2 right
//		{
//			LOG_MSG("Collision !!");
//			return true;
//		}
//		return false;
//	}
//
//	bool checkCollision(const glm::vec3& pos);
//	bool _checkPointCollision(glm::vec2 const& pt);
//
//	void observe(SSS::Subject* wPtr) { _observe(*wPtr); };
//	std::vector<SSS::UIPrimitive> prims;
//
//
//
//protected:
//	SSS::RGBA_f _color;
//	float _borderWidth;
//
//	int _hOffset = -1;	// key node for horizontal offset
//	int _vOffset = -1;	// key node for vertical offset
//	int _dOffset = -1;	// key node for depth offset
//
//	virtual void translateElem() {};
//
//	bool _hover		= false;
//	bool _held		= false;
//	bool _clicked	= false;
//	bool _hidden	= false;
//	bool _movable	= false;
//	bool _focus		= false;
//	bool _disabled	= false;
//	bool _pressed	= false;
//
//	void boundingBox();
//
//public:
//	//STATES 
//	bool isHover()		const { return _hover; };
//	bool isHeld()		const { return _held; };
//	bool isClicked()	const { return _clicked; };
//	bool isHidden()		const { return _hidden; };
//	bool isMovable()	const { return _movable; };
//	bool isFocus()		const { return _focus; };
//	bool isDisabled()	const { return _disabled; };
//	bool isPressed()	const { return _pressed; };
//private : 
//	static void _register();
//};
//
//class Node_Text : public Node_UI, SSS::_EventRegistry<Node_Text>
//{
//public:
//	friend _EventRegistry<Node_Text>;
//
//	Node_Text() = default;
//	~Node_Text();
//	Node_Text(SceneGraph* p_Sg, const std::string& s, const SSS::GUI_Layout& lyt = SSS::GUI_Layout{});
//	std::string name() const { return "Text"; };
//
//	TextPlane::Shared model;
//	int _type = 4;
//
//	virtual void _subjectUpdate(SSS::Subject const& subject, int event_id) override;
//	virtual void setWrappingMin(const int& min);
//
//	void clear() override;
//	void parseText(const std::string& str);
//	void setMaxStrSize(const int maxSize);
//	void setTextColor(const SSS::RGBA_f& col);
//	void setBackgroundColor(const SSS::RGBA_f& bgCol);
//	void rotate(const float &rot) { model->rotate(glm::vec3(0, 0,rot)); };
//	virtual void update();
//private:
//	static void _register();
//	void translateElem() { model->translate(_pos); };
//	int _maxStrSize = 600;
//	void _size_update();
//};
//
//
//class  Node_Slider : public Node_UI
//{
//public:
//	Node_Slider(const int& min, const int& max, int* cur, const glm::vec3& pos = glm::vec3{0});
//	~Node_Slider();
//	std::string name() const { return "Slider"; };
//	int _type = 4;
//
//	void build();
//
//	void clear() override;
//
//	virtual void _subjectUpdate(SSS::Subject const& subject, int event_id) override;
//
//	void setMin(int min) {_min = min;};
//	void setMax(int max) {_max = max;};
//	void setCurrentPtr(int* cur) { _current = cur; };
//
//	int getMinValue() { return _min; };
//	int getMaxValue() { return _max; };
//	int getCurValue() { return *_current; };
//
//private:
//	Node_Slider() = default;
//public:
//	int _min;
//	int _max;
//	int *_current;
//private:
//	void getCursorPos(const float& x, const float& y);
//	void _size_update() {};
//	float _radius;
//	float _len;
//	TextPlane::Shared model;
//};
//
//class  Node_Toggle : public Node_UI
//{
//public:
//	Node_Toggle(bool* b, const glm::vec3& pos = glm::vec3{0});
//	~Node_Toggle() = default;
//	std::string name() const { return "Toggle"; };
//	int _type = 4;
//
//	void build();
//
//	virtual void _subjectUpdate(SSS::Subject const& subject, int event_id) override;
//private:
//	Node_Toggle() = default;
//public:
//	bool isActive() { return *_active; };
//private:
//	float _radius;
//	bool *_active;
//	void _size_update() {};
//};
//
//
//class  Node_CheckBox : public Node_UI
//{
//public:
//	Node_CheckBox() = default;
//	Node_CheckBox(bool* b, const glm::vec3& pos = glm::vec3{ 0 });
//	~Node_CheckBox() = default;
//	std::string name() const { return "Checkbox"; };
//	int _type = 4;
//
//	void build();
//
//	virtual void _subjectUpdate(SSS::Subject const& subject, int event_id) override;
//
//private:
//public:
//	bool isActive() { return *_active; };
//private:
//	bool* _active;
//	float _radius;
//	void _size_update() {};
//};
//
//
//class  Node_RadioButton : public Node_UI, public SSS::_EventRegistry<Node_RadioButton>
//{
//friend _EventRegistry<Node_RadioButton>;
//public:
//	Node_RadioButton(bool* b, const glm::vec3& pos = glm::vec3{ 0 });
//	~Node_RadioButton() = default;
//
//	std::string name() const { return "Radio button"; };
//	int _type = 4;
//
//	void build(const glm::vec2& begin);
//
//	virtual void _subjectUpdate(SSS::Subject const& subject, int event_id) override;
//	void observeRadio(Node_RadioButton& subject);
//private:
//	Node_RadioButton() = default;
//public:
//	bool isActive() { return _active; };
//private:
//	bool _active = true;
//	float _radius;
//	void _size_update() {};
//
//	static void _register();
//};
//
//
//SSS_END;