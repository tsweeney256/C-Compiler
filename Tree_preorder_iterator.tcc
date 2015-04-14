#ifndef TREE_PREORDER_ITERATOR
#define TREE_PREORDER_ITERATOR

#include <iterator>
#include <stack>

template <typename T>
class Tree<T>::preorder_iterator : public std::iterator<std::bidirectional_iterator_tag, T>
{
public:
	preorder_iterator();
	preorder_iterator(Tree<T>*);
	preorder_iterator& operator++();
	preorder_iterator operator++(preorder_iterator);
	preorder_iterator& operator--();
	preorder_iterator operator--(preorder_iterator);
	bool operator==(const preorder_iterator&, const preorder_iterator&) const;
	bool operator!=(const preorder_iterator&, const preorder_iterator&) const;
	T& operator*() const;
	T* operator->() const;

private:
	Tree<T>* m_val;
	Tree<T>* m_original; //just so decrementing end() can work...
	std::stack<int> m_beenTo;

	inline void plusplus();
};

template <typename T>
Tree<T>::preorder_iterator::preorder_iterator()
	: m_val(NULL), m_original(NULL)
{
	m_beenTo.push(-1);
}

template <typename T>
Tree<T>::preorder_iterator::preorder_iterator(Tree<T>* val)
	: m_val(val), m_original(val)
{
	m_beenTo.push(-1);
}

template <typename T>
typename Tree<T>::preorder_iterator& Tree<T>::preorder_iterator::operator++()
{
	if((m_val->numChildren() == 0 || m_val->numChildren() == m_beenTo.top()+1) && !m_val->getParent){
		m_val = NULL;
		return *this;
	}
	else{
		while(true){
			if(m_beenTo.top()+1 == m_val->numChildren() && m_val->getParent){
				m_val = m_val->getParent;
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
typename Tree<T>::preorder_iterator Tree<T>::preorder_iterator::operator++(preorder_iterator val)
{
	typename Tree<T>::preoder_iterator temp(val);
	++(*this);
	return temp;
}

template <typename T>
typename Tree<T>::preorder_iterator& Tree<T>::preorder_iterator::operator--()
{
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
		m_val = m_val->getParent();
		m_beenTo.pop();
		if(m_beenTo.empty()){
			m_beenTo.push(m_val->numChildren()-2);
		}
		else{
			--m_beenTo.top();
		}
		while(m_val->numChildren()){
			m_val = m_val->getChild(m_beenTo.top());
			m_beenTo.push(m_val->numChildren()-1);
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
typename Tree<T>::preorder_iterator Tree<T>::preorder_iterator::operator--(preorder_iterator val)
{
	typename Tree<T>::preoder_iterator temp(val);
	--(*this);
	return temp;
}

#endif
