#ifndef SYNTAXINFO_HPP
#define SYNTAXINFO_HPP

#include <string>

struct SyntaxInfo
{
	int syntaxFlag;
	int typeFlag;
	std::string name;

	SyntaxInfo()
		: syntaxFlag(-1),
		  typeFlag(-1),
		  name() {}

	//syntax flags
	enum
	{
	    PROGRAM,
		VAR_DEC,
		FUN_DEC,
		PARAMS,
		COMPOUND_STMT,
		LOCAL_DECS,
		STMT_LIST,
		ASSIGNMENT,
		IF,
		WHILE,
		RETURN,
		INT_LITERAL,
		FLOAT_LITERAL,
		VAR,
		CALL,
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
