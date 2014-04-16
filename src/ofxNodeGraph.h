#pragma once

#include "ofMain.h"

#define OFX_NODE_GRAPH_BEGIN_NAMESPACE namespace ofx { namespace NodeGraph {
#define OFX_NODE_GRAPH_END_NAMESPACE } }

OFX_NODE_GRAPH_BEGIN_NAMESPACE

#pragma mark - Node

class Node
{
	friend class RootNode;

public:
	typedef shared_ptr<Node> Ref;

	virtual void update() {}
	virtual void draw() {}

	const string& getName() const;

	const Node* const getParent() const;
	const Node* const getRootNode() const;

	// children
	
	void clear();
	
	template <typename T>
	shared_ptr<T> addChild(const string& name = "");
	void removeChild(Node::Ref o);

	bool empty() const;
	size_t size() const;
	
	Node::Ref at(size_t index) const;

	// NOTE: it will return null reference if cast failed
	template <typename T>
	shared_ptr<T> at(size_t index) const;

	vector<Node::Ref> find(const string& name, bool recursive = false);

	// local transform

	const ofMatrix4x4& getMatrix() const;

	void setPosition(const ofVec3f& v);
	void setRotation(const ofQuaternion& v);
	void setScale(const ofVec3f& v);

	const ofVec3f& getPosition() const;
	const ofQuaternion& getRotation() const;
	const ofVec3f& getScale() const;

	// global transform

	const ofMatrix4x4& getGlobalMatrix() const;
	const ofMatrix4x4& getParentGlobalMatrixInv() const;

	void setGlobalPosition(const ofVec3f& v);
	void setGlobalRotation(const ofQuaternion& v);
	void setGlobalScale(const ofVec3f& v);

	ofVec3f getGlobalPosition() const;
	ofQuaternion getGlobalRotation() const;
	ofVec3f getGlobalScale() const;

	// node to node transform

	template <typename T0, typename T1>
	static ofMatrix4x4 getNodeToNodeTransform(const T0& from, const T1& to);

	// utility

	void move(const ofVec3f& v);
	void rotate(float angle, const ofVec3f& axis);

protected:
	string name;

	ofVec3f position;
	ofQuaternion rotation;
	ofVec3f scale;

protected:
	vector<Node::Ref> children;

	// composite pattern

	void update_(const ofMatrix4x4& parent_global_matrix)
	{
		parent_global_matrix_inv = parent_global_matrix.getInverse();
		global_matrix = matrix * parent_global_matrix;

		update();

		global_matrix = matrix * parent_global_matrix;

		vector<Node::Ref>::iterator it = children.begin();
		while (it != children.end())
		{
			Node::Ref& o = *it;
			o->update_(global_matrix);
			it++;
		}
	}

	void draw_()
	{
		ofPushMatrix();
		ofMultMatrix(global_matrix);
		draw();
		ofPopMatrix();

		vector<Node::Ref>::iterator it = children.begin();
		while (it != children.end())
		{
			Node::Ref& o = *it;
			o->draw_();
			it++;
		}
	}

private:
	const Node* parent;
	const Node* root_node;

	ofMatrix4x4 matrix;
	ofMatrix4x4 global_matrix;
	ofMatrix4x4 parent_global_matrix_inv;

	void updateLocalMatrix()
	{
		matrix.makeScaleMatrix(scale);
		matrix.rotate(rotation);
		matrix.setTranslation(position);
	}

	void updateGlobalMatrix()
	{
		// TODO: cache update state

		if (parent)
		{
			const_cast<Node*>(parent)->updateGlobalMatrix();
			const ofMatrix4x4& parent_global_matrix = parent->global_matrix;
			parent_global_matrix_inv = parent_global_matrix.getInverse();
			global_matrix = matrix * parent_global_matrix;
		}
	}

private:
	static void find_children(const string& name, Node* target,
							  vector<Node::Ref>& out, bool recursive)
	{
		for (int i = 0; i < target->children.size(); i++)
		{
			Node::Ref child = target->children[i];

			if (child->getName() == name) out.push_back(child);

			if (recursive) find_children(name, child.get(), out, recursive);
		}
	}

	template <typename T>
	static ofMatrix4x4 globalMatrix(const T& node)
	{
		return node.getGlobalMatrix();
	}

	template <typename T>
	static ofMatrix4x4 globalMatrix(const shared_ptr<T>& node)
	{
		return node->getGlobalMatrix();
	}

	template <typename T>
	static ofMatrix4x4 globalMatrix(T* node)
	{
		return node->getGlobalMatrix();
	}

protected:
	Node()
		: parent(NULL)
		, root_node(NULL)
		, scale(1)
	{
	}

	virtual ~Node() {}

private:
	// noncopyable

	Node(const Node&) {}
	Node& operator=(const Node&) {}
};

#pragma mark - RootNode

class RootNode : public Node
{
	friend class Node;

public:
	RootNode()
		: Node()
	{
		root_node = this;
		name = "root";
	}

	~RootNode() {}

	void update()
	{
		matrix.makeIdentityMatrix();
		global_matrix.makeIdentityMatrix();

		vector<Node::Ref>::iterator it = children.begin();
		while (it != children.end())
		{
			Node::Ref& o = *it;
			o->update_(global_matrix);
			it++;
		}
	}

	void draw()
	{
		ofPushStyle();

		vector<Node::Ref>::iterator it = children.begin();
		while (it != children.end())
		{
			Node::Ref& o = *it;
			o->draw_();
			it++;
		}

		ofPopStyle();
	}
};

#pragma mark - impl

inline const string& Node::getName() const { return name; }

inline const Node* const Node::getParent() const { return parent; }

inline const Node* const Node::getRootNode() const { return root_node; }

// children

template <typename T>
inline shared_ptr<T> Node::addChild(const string& name)
{
	T* x = new T;
	shared_ptr<T> o(x);

	o->name = name;
	o->parent = this;
	o->root_node = root_node;
	o->updateGlobalMatrix();

	children.push_back(o);
	return o;
}

inline void Node::removeChild(Node::Ref o)
{
	vector<Node::Ref>::iterator it =
		remove(children.begin(), children.end(), o);
	children.erase(it, children.end());
}

inline void Node::clear() { children.clear(); }

inline bool Node::empty() const { return children.empty(); }

inline size_t Node::size() const { return children.size(); }

inline Node::Ref Node::at(size_t index) const { return children.at(index); }

// NOTE: it will return null reference if cast failed
template <typename T>
inline shared_ptr<T> Node::at(size_t index) const
{
	return std::dynamic_pointer_cast<T>(children.at(index));
}

inline vector<Node::Ref> Node::find(const string& name, bool recursive)
{
	// TODO: add find_first_of
	// TODO: find with wildcard

	vector<Node::Ref> result;
	find_children(name, this, result, recursive);
	return result;
}

// local transform

inline const ofMatrix4x4& Node::getMatrix() const { return matrix; }

inline void Node::setPosition(const ofVec3f& v)
{
	position = v;
	updateLocalMatrix();
}

inline void Node::setRotation(const ofQuaternion& v)
{
	rotation = v;
	updateLocalMatrix();
}

inline void Node::setScale(const ofVec3f& v)
{
	scale = v;
	updateLocalMatrix();
}

inline const ofVec3f& Node::getPosition() const { return position; }

inline const ofQuaternion& Node::getRotation() const { return rotation; }

inline const ofVec3f& Node::getScale() const { return scale; }

// global transform

inline const ofMatrix4x4& Node::getGlobalMatrix() const { return global_matrix; }

inline const ofMatrix4x4& Node::getParentGlobalMatrixInv() const
{
	return parent_global_matrix_inv;
}

inline void Node::setGlobalPosition(const ofVec3f& v)
{
	updateGlobalMatrix();
	setPosition(parent_global_matrix_inv.preMult(v));
}

inline void Node::setGlobalRotation(const ofQuaternion& v)
{
	updateGlobalMatrix();
	setRotation(parent_global_matrix_inv.getRotate() * v);
}

inline void Node::setGlobalScale(const ofVec3f& v)
{
	updateGlobalMatrix();
	setScale(parent_global_matrix_inv.getScale() * v);
}

inline ofVec3f Node::getGlobalPosition() const
{
	return global_matrix.getTranslation();
}

inline ofQuaternion Node::getGlobalRotation() const
{
	return global_matrix.getRotate();
}

inline ofVec3f Node::getGlobalScale() const { return global_matrix.getScale(); }

// node to node transform

template <typename T0, typename T1>
inline ofMatrix4x4 Node::getNodeToNodeTransform(const T0& from, const T1& to)
{
	return globalMatrix(to) * globalMatrix(from).getInverse();
}

// utility

inline void Node::move(const ofVec3f& v) { matrix.glTranslate(v.x, v.y, v.z); }

inline void Node::rotate(float angle, const ofVec3f& axis)
{
	matrix.glRotate(angle, axis.x, axis.y, axis.z);
}

OFX_NODE_GRAPH_END_NAMESPACE

namespace ofxNodeGraph = ofx::NodeGraph;
