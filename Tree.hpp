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
    Tree<T>* clone(); //deep copy //not implemented yet
    static void destroy(Tree<T>* tree);

    void connectChild(Tree<T>* child);

    int numChildren();
    Tree<T>* getChild(int idx);
    Tree<T>* getParent();

    T val;

protected:
    ~Tree();

private:
    std::vector<Tree<T>*> m_child;
    Tree<T>* m_parent;

};

template <typename T>
Tree<T>::Tree()
        : val(), m_parent(NULL) {}

template <typename T>
Tree<T>::Tree(const T& val)
        : val(val), m_parent(NULL) {}

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
