#include "SceneGraph/scenegraph.h"

//#include "Node_Primitive.h"
//#include "Node_UI.h"
#include <random>
#include <chrono>

#include <mutex>

SSS_BEGIN

Node::Node()
{
	// Seed with a real random value, if available
	std::mt19937 rng(unsigned int(std::chrono::steady_clock::now().time_since_epoch().count()));
	_key = rng();

	SceneGraph::emplace(this);
}

Node::~Node()
{
	pop();
}


Node* Node::push(Node* n)
{
	return nullptr;
}

void Node::pop()
{
	_parent = 0;
	for (const auto& cKey : _children)
	{
		if (SceneGraph::contains(cKey.second)) 
		{
			SceneGraph::pop(cKey.second);
		}
	}

	SceneGraph::pop(_key);
}

void Node::pop_child(const int& keyNode)
{

}

void Node::detach_parent(const int& keyNode)
{
	SceneGraph::at(keyNode);
}

std::string Node::to_string() const
{
	std::string res;
	res = "Node : " + name() + "(" + SSS::toString(_key) + ")\n";

	//print parent list
	if (_parent) {
		res += "\tparents :\n";
		//for (auto node : _parents) {
		//}
		res += "\t id : " + std::to_string(_parent) + "\n";
	}

	//print children description
	if (!_children.empty()) {
		res += "\tchildren :\n";
		for (const auto &node : _children) {
			res += "\t" + SceneGraph::at(node.second)->to_string() + "\n";
		}
	}

	return res;
}

Node::operator std::string() const
{
	return to_string();
}

void Node::_register()
{
}


SceneGraph::SceneGraph()
{


}

void SceneGraph::init(){}

void SceneGraph::update()
{
	// Clear the delete node queue
	while(!get()._deleteNode.empty())
	{
		int key = get()._deleteNode.front();
		
		//remove elem if not already removed
		if (get()._nodeList.contains(key))
		{
			delete get()._nodeList[key];
			get()._nodeList.erase(key);
			get().list.erase(std::remove(get().list.begin(), get().list.end(), key), get().list.end());
		}

		get()._deleteNode.pop();
	}
}

void SceneGraph::push(Node* n)
{
	emplace(n);
	get().list.push_back(n->_key);
}

void SceneGraph::emplace(Node* n)
{
	get()._nodeList.emplace(n->_key, n);
}


Node* SceneGraph::at(const int& keyNode)
{
	if (get()._nodeList.contains(keyNode))
		return get()._nodeList.at(keyNode);

	return nullptr;
}



std::string SceneGraph::to_string() const
{
	std::string res;
	for (const int keyNode : list) {
		res += _nodeList.at(keyNode)->to_string() + "\n";
	}

	return res;
}

SceneGraph::operator std::string() const
{
	return to_string();
}

Node* SceneGraph::operator[](const int& keyNode)
{
	if (_nodeList.contains(keyNode))
	{
		return _nodeList.at(keyNode);
	}
	return nullptr;
}

SSS_END;