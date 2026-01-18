#pragma once

#include <SSS/Math.hpp>
#include <SSS/Commons.hpp>

//#include "SSS/GL.hpp"

#include "../GL.hpp"
#include "../GL/Objects/Models/Shapes.hpp"

#include <nlohmann/json.hpp>

#include <unordered_map>
#include <queue>


SSS_BEGIN

class SceneGraph;

class SSS_GL_API Node : public SSS::Observer, public SSS::Subject, public SSS::_EventRegistry<Node>
{
public:
	friend _EventRegistry;
	Node();
	Node(SceneGraph* p_Sg);
	~Node();

	unsigned int _key			= 0;
	int _type					= 0;
	bool _inherited_transform	= true;
	bool hide					= false;

	int _parent;
	std::unordered_map<std::string, int> _children;


	//Template node
	Node* push(Node* n);
	void pop();
	virtual void clear() {};

	virtual void update() {};
	bool _update = false;

	void pop_child(const int& keyNode);
	void detach_parent(const int& keyNode);
	void set_parent(const int& keyNode) { _parent = keyNode; };

	//to_string
	std::string to_string() const;
	operator std::string() const;

	virtual std::string name() const  { return "Node"; };
	
	SceneGraph* _sg;
	SSS::GL::PlaneRenderer::Weak _rd;

	std::string getLabel() { return _label; };
	void		setLabel(const std::string& lab) { _label = lab; };


	virtual std::vector<SSS::UIPrimitive> renderUI() const { return _UIprims; };
	//static std::once_flag _RegistryDone;
protected:
	std::vector<SSS::UIPrimitive>	_UIprims;
	std::string					_label;

private:
	static void _register();
};


class SSS_GL_API SceneGraph
{
public:
	SceneGraph();
	~SceneGraph() {};

	void init();
	void update();
	//Add the node to the nodelist and add it to the arborescence, to be used on free nodes
	void push(Node* n);	
	void emplace(Node* n);	// Add the node to the nodelist

	//int Text(const std::string& s, const SSS::GUI_Layout& lyt = SSS::GUI_Layout{});
	//int Block(const glm::vec3 &pos = glm::vec3(0));

	void pop(const int& key) { _deleteNode.push(key); };
	bool contains(const int& key) { return _nodeList.contains(key); };
	Node* at(const int & keyNode);

	void setCamera(SSS::GL::Camera::Shared pCam);

	//to_string
	std::string to_string() const;
	operator std::string() const;
	
	Node* operator[](const int &keyNode);
	std::vector<int> list;

	SSS::GL::PlaneRenderer::Shared _rd;
protected:

	std::unordered_map<int, Node*> _nodeList;
	SSS::GL::Camera::Shared _cam;
	std::queue<int> _deleteNode;

	//std::unordered_map<std::string, SSS::GL::PlaneRenderer::Shared> _rdList;

};

SSS_END;