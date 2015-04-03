#ifndef SYNTAXINFO_HPP
#define SYNTAXINFO_HPP

#include <string>

struct SyntaxInfo
{
	int syntaxFlag;
	int typeFlag;
	int array;
	int leftTypeFlag;
	int leftArray;
	int rightTypeFlag;
	int rightArray;
	std::string name;

	SyntaxInfo()
		: syntaxFlag(-1),
		  typeFlag(-1),
		  array(0),
		  leftTypeFlag(-1),
		  leftArray(0),
		  rightTypeFlag(-1),
		  rightArray(0),
		  name() {}

	//syntax flags
	enum
	{
		VAR_DEC,
		FUN_DEC,
		PARAMS,
		COMPOUND_STMT,
		STATEMENT_LIST,
		ASSIGNMENT,
		IF,
		WHILE,
		RETURN,
		INT_LITERAL,
		FLOAT_LITERAL,
		ID,
		ADD,
		SUB,
		MULT,
		DIV,
		EQ,
		NEQ,
		GT,
		GTEQ,
		LT,
		LTEQ
	};

	//type flags
	enum
	{
		VOID,
		INT,
		FLOAT,
		INT_ARRAY,
		FLOAT_ARRAY
	};
};

#endif
