#pragma once

#include <SSS/Math.hpp>
#include <SSS/Commons.hpp>

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
	~Node();

	unsigned int _key			= 0;
	int _type					= 0;
	bool _inherited_transform	= true;
	bool hide					= false;

	int _parent = -1;
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
	
	SSS::GL::PlaneRenderer::Weak _rd;

	std::string getLabel() { return _label; };
	void		setLabel(const std::string& lab) { _label = lab; };


	virtual std::vector<SSS::UIPrimitive> renderUI() const { return _UIprims; };
protected:
	std::vector<SSS::UIPrimitive>	_UIprims;
	std::string						_label;

private:
	static void _register();
};


class SSS_GL_API SceneGraph
{
public:
	static SceneGraph& get() {
		static SceneGraph instance;
		return instance;
	}


	// delete copy/move to prevent duplicates
	SceneGraph(const SceneGraph&) = delete;
	SceneGraph& operator=(const SceneGraph&) = delete;

private:
	SceneGraph();
	~SceneGraph();
public:

	static void init();
	static void update();
	//Add the node to the nodelist and add it to the arborescence, to be used on free nodes
	static void push(Node* n);	
	static void emplace(Node* n);	// Add the node to the nodelist

	//int Text(const std::string& s, const SSS::GUI_Layout& lyt = SSS::GUI_Layout{});
	//int Block(const glm::vec3 &pos = glm::vec3(0));

	static void pop(const int& key) { get()._deleteNode.push(key); };
	static bool contains(const int& key) { return get()._nodeList.contains(key); };
	static Node* at(const int & keyNode);


	// Current renderers for the scenegraph, to be used in nodes and for rendering
	static void setCurrentRenderer(SSS::GL::PlaneRenderer::Shared pRenderer) { get()._currentRenderer = pRenderer; };
	static SSS::GL::PlaneRenderer::Shared getCurrentRenderer() { return get()._currentRenderer; };
	// UI renderer, used for UI elements on specific render pass
	static void setUIRenderer(SSS::GL::UIRenderer::Shared pUIRenderer) { get()._uiRenderer = pUIRenderer; };
	static SSS::GL::UIRenderer::Shared getUIRenderer() { return get()._uiRenderer; };

	static void clearRenderers();
	
	
	//to_string
	std::string to_string() const;
	operator std::string() const;

	Node* operator[](const int &keyNode);
	std::vector<int> list; // Node tree first level

protected:

	std::unordered_map<int, Node*> _nodeList;
	std::queue<int> _deleteNode;

	SSS::GL::PlaneRenderer::Shared 	_currentRenderer;
	SSS::GL::UIRenderer::Shared 	_uiRenderer;
};

SSS_END;