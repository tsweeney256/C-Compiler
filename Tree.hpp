#ifndef TREE_HPP
#define TREE_HPP

#include <cstddef>
#include <ostream>
#include <vector>
#include "SyntaxInfo.hpp"

template <typename T>
class Tree
{
public:
    Tree();
    Tree(const T& val);
    static void destroy(Tree<T>* tree); //destroys all children, grandchildren, etc.
    static void destroyNode(Tree<T>* tree); //doesn't destroy any children
    void connectChild(Tree<T>* child);
    int numChildren();
    Tree<T>* getChild(int idx);
    Tree<T>* getParent();
    void print(std::ostream& stream);

    T val;

    class preorder_iterator;
    preorder_iterator preorder_begin();
    preorder_iterator preorder_end();

protected:
    ~Tree();

private:
    std::vector<Tree<T>*> m_child;
    Tree<T>* m_parent;
    void print(std::string prefix, bool isTail, std::ostream& os); //recursive function called by printTree();
};

template <typename T>
Tree<T>::Tree()
        : val(), m_parent(NULL) {}

template <typename T>
Tree<T>::Tree(const T& val)
        : val(val), m_parent(NULL) {}

template <typename T>
Tree<T>::~Tree(){}

template <typename T>
void Tree<T>::destroy(Tree<T>* tree)
{
	if(tree){
		for(typename std::vector<Tree<T>*>::const_iterator it=tree->m_child.begin(); it != tree->m_child.end(); ++it){
			 destroy(*it);
		}
		delete tree;
	}

}

template <typename T>
void Tree<T>::destroyNode(Tree<T>* tree)
{
	delete tree;
}

template <typename T>
void Tree<T>::connectChild(Tree<T>* child)
{
	m_child.push_back(child);
	child->m_parent=this;
}

template <typename T>
int Tree<T>::numChildren()
{
	return m_child.size();
}

template <typename T>
Tree<T>* Tree<T>::getChild(int idx)
{
	return m_child[idx];
}

template <typename T>
Tree<T>* Tree<T>::getParent()
{
	return m_parent;
}

template <typename T>
void Tree<T>::print(std::ostream& os)
{
	print("", true, os);
}

//adapted from Vasya Novikov's java implementation
//http://stackoverflow.com/questions/4965335/how-to-print-binary-tree-diagram
//I just have this for debugging and it's never actually called in p4, so please don't actually flag it as plagiarism
template <typename T>
void Tree<T>::print(std::string prefix, bool isTail, std::ostream& os)
{
	os << prefix << (isTail ? "└──" : "├──") << val << std::endl;
	for(int i=0; i<numChildren()-1; ++i){
		getChild(i)->print(prefix + (isTail ? "    " : "│   "), false, os);
	}
	if(numChildren() > 0){
		getChild(numChildren()-1)->print(prefix + (isTail ?"    " : "│   "), true, os);
	}
}

template <typename T>
typename Tree<T>::preorder_iterator Tree<T>::preorder_begin()
{
	return preorder_iterator(this, this);
}

template <typename T>
typename Tree<T>::preorder_iterator Tree<T>::preorder_end()
{
	return preorder_iterator(NULL, this);
}

#include "Tree_preorder_iterator.tcc"

#endif // TREE_HPP
