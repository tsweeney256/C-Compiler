#ifndef SYMBOLTABLE_HPP
#define SYMBOLtABLE_HPP

#include <string>
#include <vector>

class SymbolTable
{
public:
	SymbolTable(int size = 256);
	~SymbolTable();
	//TODO: implement proper copying
	//returns true if add() succeeded, returns false if name already exists
	//giving a non-null argument for signature means the symbol is a function, a null one means it's not
	//do not ever delete the argument you give signature. the class' destructor will do that for you
	//add() will even delete signature in the event that it returns false.
	bool add(const std::string& name, int type, std::vector<int>* signature = NULL);
	//returns the type as a flag if the name is found. Returns the NOT_FOUND flag otherwise.
	int peek(const std::string& name) const;
	//if the return is null, then the symbol does not represent a function
	//ideally, getSignature() and peek() should be combined into one function,
	//but I made getSignature well afterwards and I just don't feel like remaking them both
	//this is good enough. It doesn't really matter that much
	const std::vector<int>* getSignature(const std::string& name) const;

	//flags returned by peek()
	enum{
		VOID,
		INT,
		FLOAT,
		INT_ARRAY,
		FLOAT_ARRAY,
		NOT_FOUND
	};

private:
	class Impl;
	Impl* pimpl;
};

#endif
