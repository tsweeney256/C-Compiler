#ifndef SYMBOLTABLE_HPP
#define SYMBOLtABLE_HPP

#include <string>

class SymbolTable
{
public:
	SymbolTable(int size = 256);
	~SymbolTable();
	//returns true if add() succeeded, returns false if name already exists
	bool add(const std::string& name, int type);
	//returns the type as a flag if the name is found. Returns the NOT_FOUND flag otherwise.
	int peek(const std::string& name);

	//flags returned by peek()
	enum{
		VOID,
		INT,
		FLOAT,
		NOT_FOUND
	};

private:
	class Impl;
	Impl* pimpl;
};

#endif
