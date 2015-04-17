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
		ARRAY_DEC_SIZE,
		FUN_DEC,
		PARAMS,
		EXIT_PARAMS,
		PARAM_DEC,
		COMPOUND_STMT,
		EXIT_COMPOUND_STMT,
		LOCAL_DECS,
		STMT_LIST,
		EXIT_EXPR_STMT,
		ASSIGNMENT,
		EXIT_ASSIGNMENT,
		IF,
		EXIT_IF_PREDICATE,
		BEGIN_IF_STMT,
		BEGIN_ELSE_STMT,
		WHILE,
		EXIT_WHILE_PREDICATE,
		BEGIN_WHILE_STMT,
		EXIT_WHILE_STMT,
		RETURN,
		EXIT_RETURN_STMT,
		INT_LITERAL,
		FLOAT_LITERAL,
		VAR,
		INDEX,
		EXIT_INDEX,
		CALL,
		EXIT_CALL,
		ARG,
		EXIT_ARG,
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
			return "LARCH";
		case VAR_DEC:
			return "VAR_DEC";
        case ARRAY_DEC_SIZE:
            return "ARRAY_DEC_SIZE";
		case FUN_DEC:
			return "FUN_DEC";
		case PARAMS:
			return "PARAMS";
        case EXIT_PARAMS:
            return "EXIT_PARAMS";
        case PARAM_DEC:
            return "PARAM_DEC";
		case COMPOUND_STMT:
			return "COMPOUND_STMT";
        case EXIT_COMPOUND_STMT:
            return "EXIT_COMPOUND_STMT";
		case LOCAL_DECS:
			return "LOCAL_DECS";
		case STMT_LIST:
			return "STMT_LIST";
        case EXIT_EXPR_STMT:
            return "EXIT_EXPR_STMT";
		case ASSIGNMENT:
			return "ASSIGNMENT";
        case EXIT_ASSIGNMENT:
            return "EXIT_ASSIGNMENT";
		case IF:
			return "IF";
        case BEGIN_IF_STMT:
            return "BEGIN_IF_STMT";
        case BEGIN_ELSE_STMT:
            return "BEGIN_ELSE_STMT";
		case WHILE:
			return "WHILE";
        case BEGIN_WHILE_STMT:
            return "BEGIN_WHILE_STMT";
        case EXIT_WHILE_STMT:
            return "EXIT_WHILE_STMT";
		case RETURN:
			return "RETURN";
        case EXIT_RETURN_STMT:
            return "EXIT_RETURN_STMT";
		case INT_LITERAL:
			return "INT_LITERAL";
		case FLOAT_LITERAL:
			return "FLOAT_LITERAL";
		case VAR:
			return "VAR";
		case INDEX:
			return "INDEX";
		case EXIT_INDEX:
			return "EXIT_INDEX";
		case CALL:
			return "CALL";
		case EXIT_CALL:
			return "EXIT_CALL";
		case ARG:
			return "ARG";
		case EXIT_ARG:
			return "EXIT_ARG";
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
