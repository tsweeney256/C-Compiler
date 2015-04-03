#ifndef TREE_HPP
#define TREE_HPP

#include <cstddef>
#include <vector>

template <typename T>
class Tree
{
public:
    Tree();
    Tree(const T& val);
    Tree<T>* clone(); //deep copy
    static void destroy(Tree<T>* tree);
    T getVal() const;
    void setVal(const T& val);

    void connectChild(Tree<T>* child);

    int numChildren();
    Tree<T>* getChild(int idx);
    Tree<T>* getParent();

protected:
    ~Tree();

private:
    T m_val;
    std::vector<Tree<T>*> m_child;
    Tree<T>* m_parent;

};

template <typename T>
Tree<T>::Tree()
        : m_val(), m_parent(NULL) {}

template <typename T>
Tree<T>::Tree(const T& val)
        : m_val(val), m_parent(NULL) {}

template <typename T>
Tree<T>::~Tree()
{
	for(typename std::vector<Tree<T>*>::const_iterator it=m_child.begin(); it != m_child.end(); ++it){
		delete *it;
	}
}

template <typename T>
void Tree<T>::destroy(Tree<T>* tree)
{
	delete tree;
}

template <typename T>
Tree<T>* Tree<T>::clone()
{
	return NULL; //maybe I'll implement it later...
}

template <typename T>
T Tree<T>::getVal() const
{
    return m_val;
}

template <typename T>
void Tree<T>::setVal(const T& val)
{
    m_val = val;
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

#endif // TREE_HPP
