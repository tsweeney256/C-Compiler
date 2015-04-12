#ifndef SYNTAXINFO_HPP
#define SYNTAXINFO_HPP

#include <string>
#include <ostream>

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

	static std::string getSyntaxFlagName(int flag)
	{
		switch(flag){
		case PROGRAM:
			return "PROGRAM";
		case VAR_DEC:
			return "VAR_DEC";
		case FUN_DEC:
			return "FUN_DEC";
		case PARAMS:
			return "PARAMS";
		case COMPOUND_STMT:
			return "COMPOUND_STMT";
		case LOCAL_DECS:
			return "LOCAL_DECS";
		case STMT_LIST:
			return "STMT_LIST";
		case ASSIGNMENT:
			return "ASSIGNMENT";
		case IF:
			return "IF";
		case WHILE:
			return "WHILE";
		case RETURN:
			return "RETURN";
		case INT_LITERAL:
			return "INT_LITERAL";
		case FLOAT_LITERAL:
			return "FLOAT_LITERAL";
		case VAR:
			return "VAR";
		case CALL:
			return "CALL";
		case ADD:
			return "ADD";
		case SUB:
			return "SUB";
		case MULT:
			return "MULT";
		case DIV:
			return "DIV";
		case EQ:
			return "EQ";
		case NEQ:
			return "NEQ";
		case GT:
			return "GT";
		case GTEQ:
			return "GTEQ";
		case LT:
			return "LT";
		case LTEQ:
			return "LTEQ";
		default:
			return "ERROR";
		}
	}

	static std::string getTypeFlagName(int flag)
	{
		switch(flag){
		case VOID:
			return "VOID";
		case INT:
			return "INT";
		case FLOAT:
			return "FLOAT";
		case INT_ARRAY:
			return "INT_ARRAY";
		case FLOAT_ARRAY:
			return "FLOAT_ARRAY";
		default:
			return "ERROR";

		}
	}

	friend std::ostream& operator<<(std::ostream& os, const SyntaxInfo& si)
	{
		os << SyntaxInfo::getSyntaxFlagName(si.syntaxFlag);
		if(si.name.size() > 0){
			os << " \"" << si.name << "\"";
		}
		if(si.typeFlag != -1){
			os << " [" << SyntaxInfo::getTypeFlagName(si.typeFlag) << "]";
		}
		return os;
	}
};

#endif
