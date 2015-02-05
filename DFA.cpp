#include <vector>
#include <map>
#include <string>
#include <cctype>
#include "DFA.hpp"

class DFA::DFA_
{
public:
	void add(int target, int dest, char val);
	void accept(int target);
	void start(int target);
	bool isValidString(const std::string& str);
	int feedNextCharacter(char val);
	void reset();

private:
	class State;

	State* m_currState;
	std::map<int, State> m_stateMap;
	int m_start;

    class State
    {
    public:
        State();
        ~State();
        void addLink(State* next, char val);
        void accept();
        bool isAccepting();
        State* getNextState(char val);

    private:
        class Link;

        std::vector<Link*> m_containedLinks;
        bool m_accepts;

        class Link
        {
        public:
            Link(State* next, char val);
            char getVal();
            State* getNextState();

        private:
            State* m_next;
            char m_val;
        };
    };
};

DFA::DFA_::State::State() : m_accepts(false) {}

DFA::DFA_::State::~State()
{
	for(std::vector<Link*>::const_iterator it = m_containedLinks.begin(); it != m_containedLinks.end(); ++it){
		delete *it;
	}
}

void DFA::DFA_::State::addLink(State* next, char val)
{
	Link* link = new Link(next, val);
    m_containedLinks.push_back(link);
}

void DFA::DFA_::State::accept()
{
	m_accepts = true;
}

bool DFA::DFA_::State::isAccepting()
{
	return m_accepts;
}

DFA::DFA_::State* DFA::DFA_::State::getNextState(char val)
{
	std::vector<Link*>::const_iterator it = m_containedLinks.begin();
	bool foundDigit = false, foundAlpha = false, foundBlank = false;
	State* nextState = NULL;

	while(it != m_containedLinks.end()){
		char itVal = (*it)->getVal();

		//links with only a singular value trump all others and so return instantly
		if(itVal == val){
			return (*it)->getNextState();
		}
		//overwrites nextState if it was using an ANY link before because DIGIT and ALPHA trump ANY
		else if(itVal == DIGIT && isdigit(val)){
			nextState =  (*it)->getNextState();
			foundDigit = true;
		}
		else if(itVal == ALPHA && isalpha(val))
		{
			nextState = (*it)->getNextState();
			foundAlpha = true;
		}
		else if(itVal == BLANK && isspace(val)){
			nextState = (*it)->getNextState();
			foundBlank = true;
		}
		//DIGIT and ALPHA trump ANY and so don't use any ANY links if there are a DIGIT or ALPHA link
		else if(itVal == ANY && !foundDigit && !foundAlpha && !foundBlank){
			nextState = (*it)->getNextState();
		}
		++it;
	}
	return nextState; //will be null if no next state was found
}

DFA::DFA_::State::Link::Link(State* next, char val) : m_next(next), m_val(val) {}

char DFA::DFA_::State::Link::getVal()
{
	return m_val;
}

DFA::DFA_::State* DFA::DFA_::State::Link::getNextState()
{
	return m_next;
}

void DFA::DFA_::add(int target, int dest, char val)
{
	m_stateMap[target].addLink(&m_stateMap[dest], val);
}

void DFA::DFA_::accept(int target)
{
	m_stateMap[target].accept();
}

void DFA::DFA_::start(int target)
{
	m_start = target;
	m_currState = &m_stateMap[m_start];
}

bool DFA::DFA_::isValidString(const std::string& str)
{
	std::string::const_iterator it = str.begin();
	State* currState = &m_stateMap[m_start];

	while(it != str.end()){
		if(!(currState = currState->getNextState(*it))){
			return false;
		}
		++it;
	}
	if(currState->isAccepting()){
		return true;
	}
	else{
		return false;
	}
}

int DFA::DFA_::feedNextCharacter(char val)
{
	if(m_currState){ //since short circuiting wasn't working for some reason
		if(!(m_currState = m_currState->getNextState(val))){
			return REJECTED;
		}
		else if(m_currState->isAccepting()){
			return ACCEPTED;
		}
		else{
			return NOT_DONE;
		}
	}
	else
		return REJECTED;
}

void DFA::DFA_::reset()
{
	m_currState = &m_stateMap[m_start];
}

DFA::DFA()
{
	impl = new DFA_;
}

DFA::~DFA()
{
	delete impl;
}

void DFA::add(int target, int dest, char val)
{
	impl->add(target, dest, val);
}

void DFA::accept(int target)
{
	impl->accept(target);
}

void DFA::start(int target)
{
	impl->start(target);
}

bool DFA::isValidString(const std::string& str)
{
	return impl->isValidString(str);
}

int DFA::feedNextCharacter(char val)
{
	return impl->feedNextCharacter(val);
}

void DFA::reset()
{
	impl->reset();
}
