#ifndef DFA_HPP
#define DFA_HPP

#include <string>

class DFA
{
public:
    DFA();
    ~DFA();
    void add(int target, int dest, char val);
    void accept(int target);
    void start(int target);
    bool isValidString(const std::string& str);
    int feedNextCharacter(char val); //returns the listed enum flags
    void reset(); //for use with feedNextCharacter(). resets back to the starting state

    //flags returned by feedNextCharacter()
    enum
	{
    	REJECTED,
		ACCEPTED,
		NOT_DONE
	};
    //special flags you can give to the val parameter of add()
    //more specific flags or chars will always take priority over a more general flag
    enum
	{
    	DIGIT = -100,
		ALPHA,
		ANY,
		BLANK
	};

private:
    class DFA_;
    DFA_* impl;
};

#endif // DFA_HPP
