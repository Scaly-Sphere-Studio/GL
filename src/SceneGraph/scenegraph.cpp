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

	//std::call_once(_RegistryDone, [&]() { _register(); });
}

Node::Node(SceneGraph* p_Sg)
{
	// Seed with a real random value, if available
	std::mt19937 rng((unsigned int)std::chrono::steady_clock::now().time_since_epoch().count());
	_key = rng();
	_sg = p_Sg;
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
		if (_sg->contains(cKey.second)) 
		{
			_sg->pop(cKey.second);
		}
	}

	_sg->pop(_key);
}

void Node::pop_child(const int& keyNode)
{

}

void Node::detach_parent(const int& keyNode)
{
	_sg[keyNode];
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
			res += "\t" + _sg->at(node.second)->to_string() + "\n";
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

void SceneGraph::init()
{
	_cam = SSS::GL::Camera::create();
	_cam->setPosition({ 0, 0, 20.f });
	_cam->setZFar(40.f);
	//_cam->setProjectionType(SSS::GL::Camera::Projection::UI);

	_rd = SSS::GL::PlaneRenderer::create();
	_rd->camera = SSS::GL::Camera::create();
	_rd->camera->setPosition({ 0, 0, 20.f });
	_rd->camera->setZFar(40.f);
	_rd->camera->setProjectionType(SSS::GL::Camera::Projection::OrthoFixed);

}

void SceneGraph::update()
{
	// Clear the delete node queue
	while(!_deleteNode.empty())
	{
		int key = _deleteNode.front();
		
		//remove elem if not already removed
		if (_nodeList.contains(key)) 
		{
			delete _nodeList[key];
			_nodeList.erase(key);
			list.erase(std::remove(list.begin(), list.end(), key), list.end());
		}

		_deleteNode.pop();
	}
}

void SceneGraph::push(Node* n)
{
	emplace(n);
	list.push_back(n->_key);
}

void SceneGraph::emplace(Node* n)
{
	_nodeList.emplace(n->_key, n);
}

//int SceneGraph::Text(const std::string& s, const SSS::GUI_Layout& lyt)
//{
//	//Node_Text* node = new Node_Text{ this, s, lyt };
//	return node->_key;
//}
//
//int SceneGraph::Block(const glm::vec3& pos)
//{
//	//Node_Block* node = new Node_Block{this};
//	return node->_key;
//}


Node* SceneGraph::at(const int& keyNode)
{
	if (_nodeList.contains(keyNode))
		return _nodeList.at(keyNode);

	return nullptr;
}

void SceneGraph::setCamera(SSS::GL::Camera::Shared pCam)
{
	_cam = pCam;
	_rd->camera = _cam;
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