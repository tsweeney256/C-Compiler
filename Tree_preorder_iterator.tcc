#ifndef TREE_PREORDER_ITERATOR
#define TREE_PREORDER_ITERATOR

#include <iterator>
#include <stack>

template <typename T>
class Tree<T>::preorder_iterator : public std::iterator<std::bidirectional_iterator_tag, T>
{
public:
	preorder_iterator();
	preorder_iterator(Tree<T>*, Tree<T>*);
	preorder_iterator& operator++();
	preorder_iterator operator++(int);
	preorder_iterator& operator--();
	preorder_iterator operator--(int);
	bool operator==(const preorder_iterator&) const;
	bool operator!=(const preorder_iterator& other) const;
	T& operator*() const;
	T* operator->() const;

private:
	Tree<T>* m_val;
	Tree<T>* m_original; //just so decrementing end() can work...
	std::stack<int> m_beenTo;
};

template <typename T>
Tree<T>::preorder_iterator::preorder_iterator()
	: m_val(NULL), m_original(NULL)
{
	m_beenTo.push(-1);
}

template <typename T>
Tree<T>::preorder_iterator::preorder_iterator(Tree<T>* val, Tree<T>* copy)
	: m_val(val), m_original(copy)
{
	m_beenTo.push(-1);
}

template <typename T>
typename Tree<T>::preorder_iterator& Tree<T>::preorder_iterator::operator++()
{
	if((m_val->numChildren() == 0 || m_val->numChildren() == m_beenTo.top()+1) && !m_val->getParent()){
		m_val = NULL;
		return *this;
	}
	else{
		while(true){
			if(m_beenTo.top()+1 == m_val->numChildren() && m_val->getParent()){
				m_val = m_val->getParent();
				m_beenTo.pop();
			}
			else if(m_beenTo.top()+1 != m_val->numChildren()){
				++m_beenTo.top();
				m_val = m_val->getChild(m_beenTo.top());
				m_beenTo.push(-1);
				return *this;
			}
			else{
				m_val = NULL;
				return *this;
			}
		}
	}
}

template <typename T>
typename Tree<T>::preorder_iterator Tree<T>::preorder_iterator::operator++(int)
{
	typename Tree<T>::preorder_iterator temp = *this;
	++(*this);
	return temp;
}

template <typename T>
typename Tree<T>::preorder_iterator& Tree<T>::preorder_iterator::operator--()
{
    int debug;
	if(!m_val){ //end()
		m_val = m_original;
		if(m_beenTo.empty()){
			m_beenTo.push(-1);
		}
		while(m_val->numChildren()){
			m_val = m_val->getChild(m_val->numChildren()-1);
		}
		return *this;
	}
	else if(m_beenTo.top() == -1){
        debug = m_beenTo.top();
		m_val = m_val->getParent();
		m_beenTo.pop();
		if(m_beenTo.empty()){
			m_beenTo.push(m_val->numChildren()-2);
            debug = m_beenTo.top();
		}
		else{
			--m_beenTo.top();
			debug = m_beenTo.top();
		}
		if(m_beenTo.top() == -1){
            return *this;
		}
		while(m_val->numChildren()){
			m_val = m_val->getChild(m_beenTo.top());
			m_beenTo.push(m_val->numChildren()-1);
			debug = m_beenTo.top();
		}
		return *this;
	}
	else{
		m_val = m_val->getChild(m_beenTo.top());
		while(m_val->numChildren()){
			m_val = m_val->getChild(m_beenTo.top());
			m_beenTo.push(m_val->numChildren()-1);
		}
		return *this;
	}

}

template <typename T>
typename Tree<T>::preorder_iterator Tree<T>::preorder_iterator::operator--(int)
{
	typename Tree<T>::preorder_iterator temp = *this;
	--(*this);
	return temp;
}

template <typename T>
bool Tree<T>::preorder_iterator::operator==(const preorder_iterator& other) const
{
	return m_val == other.m_val;
}

template <typename T>
bool Tree<T>::preorder_iterator::operator!=(const preorder_iterator& other) const
{
	return m_val != other.m_val;
}

template <typename T>
T& Tree<T>::preorder_iterator::operator*() const
{
	return m_val->val;
}

template <typename T>
T* Tree<T>::preorder_iterator::operator->() const
{
	return &m_val->val;
}

#endif
