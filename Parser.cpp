#include <iostream>
#include <string>
#include <cstdlib>
#include "Parser.hpp"
#include "LexicalAnalyzer.hpp"

//making a "static class" through a namespace makes it less useful
//for multi-core programs, but eh. I'm just doing it for practice.
namespace Parser
{
	namespace //private to this translation unit
    {
		//flags for match(int id)
		enum
		{
			ID = -100,
			NUM
		};

    	bool match(const std::string& str);
    	bool match(int flag);

    	bool program();
    	bool declarationList();
    	bool declaration();
    	bool varDeclaration();
    	bool typeSpecifier();
    	bool funDeclaration();
    	bool params();
    	bool paramList();
    	bool oneParam();
    	bool compoundStmt();
    	bool localDeclarations();
    	bool stmtList();
    	bool statement();
    	bool expressionStmt();
    	bool selectionStmt();
    	bool iterationStmt();
    	bool returnStmt();
    	bool expression();
    	bool var();
    	bool simpleExpression();
    	bool relop();
    	bool additiveExpression();
    	bool addop();
    	bool term();
    	bool mulop();
    	bool factor();
    	bool call();
    	bool args();
    	bool argList();
    	//needed to mostly duplicate some rules to left factor
    	bool annoyingExpression();
    	bool annoyingAdditiveExpression();
    	bool annoyingTerm();
    	bool annoyingFactor();

        std::string currTok;
        LexicalAnalyzer lex;

        bool match(const std::string& str)
        {
        	if(str.compare(currTok)){
        		return false;
        	}
        	currTok = lex.getNextToken();
        	return true;
        }

        bool match(int flag)
        {
        	if(flag == ID){
        		if(lex.lastTokenFlag() != LexicalAnalyzer::ID){
        			return false;
        		}
        	}
        	else if(flag == NUM){
        		if(!(lex.lastTokenFlag() == LexicalAnalyzer::INT_LITERAL ||
        				lex.lastTokenFlag() == LexicalAnalyzer::FLOAT_LITERAL)){
        			return false;
        		}
        	}
        	else{
        		std::cerr << "Error: Incorrect flag given" << std::endl;
        		exit(1);
        	}

        	currTok = lex.getNextToken();
        	return true;
        }

        bool program() //declarationList;
        {
        	if(lex.eof()){
        		return true;
        	}
            if(!declarationList()){
                return false;
            }
            if(lex.eof()){
            	return true;
            }
            else{
            	return false;
            }
        }

        bool declarationList() //declaration, {declaration};
        {
            if(!declaration()){
                return false;
            }
            while(declaration()){}
            return true;
        }

        bool declaration() //typeSpecifier, "ID", (varDeclaration | funDeclaration);
        {
            if(!(typeSpecifier() && match(ID))){
            	return false;
            }
            if(varDeclaration()){}
            else if(funDeclaration()){}
            else{
            	return false;
            }
            return true;
        }

        bool varDeclaration() //";" | "[NUM];";
        {
            if(match(";")){}
            else if(match("[") && match(NUM) && match("]") && match(";")){}
            else{
                return false;
            }
            return true;
        }

        bool typeSpecifier() //"int" | "void" | "float"
        {
            if(match("int")){}
            else if(match("void")){}
            else if(match("float")){}
            else{
                return false;
            }
            return true;
        }

        bool funDeclaration() //"(", params, ")", compoundStmt;
        {
            if(!(match("(") && params() && match(")") && compoundStmt())){
                return false;
            }
            return true;
        }

        bool params() //"void" | (",", paramList) | paramList
        {
        	//(",", paramList) as a hack rule so I don't have to care
        	//about "void" being the first set of both "void" and paramList
            if(match("void")){}
            else if(match(",") && paramList()){}
            else if(paramList()){}
            else{
                return false;
            }
            return true;
        }

        bool paramList() //oneParam, {",", oneParam};
        {
            if(!oneParam()){
                return false;
            }
            bool repeat = true;
            while(repeat){
                if(match(",")){
                    if(!oneParam()){
                        return false;
                    }
                }
                else{
                    repeat = false;
                }
            }
            return true;
        }

        bool oneParam() //typeSpecifier, "ID", ["[]"];
        {
            if(!(typeSpecifier() && match(ID))){
                return false;
            }
            if(match("[")){
                if(!match("]")){
                    return false;
                }
            }
            return true;
        }

        bool compoundStmt() //"{", localDeclarations, stmtList, "}";
        {
            if(match("{") && localDeclarations() && stmtList() && match("}")){}
            else{
                return false;
            }
            return true;
        }

        bool localDeclarations() //[{type-specifier, "ID", varDeclaration}];
        {
        	while(typeSpecifier()){
        		if(!(match(ID) && varDeclaration())){
        			return false;
        		}
        	}
            return true;
        }

        bool stmtList() //{statements}
        {
            while(statement()){}
            return true;
        }

        bool statement() //expressionStmt | compoundStmt | selectionStmt | iterationStmt | returnStmt
        {
            if(expressionStmt()){}
            else if(compoundStmt()){}
            else if(selectionStmt()){}
            else if(iterationStmt()){}
            else if(returnStmt()){}
            else{
                return false;
            }
            return true;
        }

        bool expressionStmt() //[expression], ";";
        {
            if(expression()){}
            if(!match(";")){
                return false;
            }
            return true;
        }

        bool selectionStmt() //"if(", expression, ")", statement, ["else", statement];
        {
            if(!(match("if") && match("(") && expression() && match(")") && statement())){
                return false;
            }
            if(match("else")){
                if(!statement()){
                    return false;
                }
            }
            return true;
        }

        bool iterationStmt() //"while(", expression, ")", statement;
        {
            if(!(match("while") && match("(") && expression() && match(")") && statement())){
                return false;
            }
            return true;
        }

        bool returnStmt() //"return", [expression], ";";
        {
            if(match("return")){
                if(expression()){}
                if(!match(";")){
                    return false;
                }
            }
            else{
                return false;
            }
            return true;
        }

        bool expression() //("ID", var, (( "=", expression) | annoyingExpression)) | simpleExpression;
        {
            if(match(ID) && var()){
            	if(match("=") && expression()){}
            	else if(annoyingExpression()){}
            	else{
            		return false;
            	}
            }
            else if(simpleExpression()){}
            else{
            	return false;
            }
            return true;
        }

        bool var() //["[", expression, "]"];
        {
        	if(match("[")){
        		if(!(expression() && match("]"))){
        			return false;
        		}
        	}
        	return true;
        }

        bool simpleExpression() //additiveExpression, {relop, additiveExpression};
        {
        	if(!additiveExpression()){
        		return false;
        	}
        	while(relop()){
        		if(!additiveExpression()){
        			return false;
        		}
        	}
        	return true;
        }

        bool relop() //"<=" | "<" | ">" | ">=" | "==" | "!=";
        {
        	if(match("<=")){}
        	else if(match("<")){}
        	else if(match(">")){}
        	else if(match(">=")){}
        	else if(match("==")){}
        	else if(match("!=")){}
        	else{
        		return false;
        	}
        	return true;
        }

        bool additiveExpression() //term, {addop, term};
        {
        	if(!term()){
        		return false;
        	}
        	while(addop()){
        		if(!term()){
        			return false;
        		}
        	}
        	return true;
        }

        bool addop() //"+" | "-";
        {
        	if(match("+")){}
        	else if(match("-")){}
        	else{
        		return false;
        	}
        	return true;
        }

        bool term() //factor, {mulop, factor};
        {
        	if(!factor()){
        		return false;
        	}
        	while(mulop()){
        		if(!factor()){
        			return false;
        		}
        	}
        	return true;
        }

        bool mulop() //"*" | "/";
        {
        	if(match("*")){}
        	else if(match("/")){}
        	else{
        		return false;
        	}
        	return true;;
        }

        bool factor() //("(", expression, ")") | "NUM"
        {
        	if(match("(")){
        		if(!expression() && match(")")){
        			return false;
        		}
        	}
        	else if(match(NUM)){}
        	else{
        		return false;
        	}
        	return true;
        }

        bool call() //"ID(", args, ")";
        {
        	if(!match(ID) && match("(") && args() && match(")")){
        		return false;
        	}
        	return true;
        }

        bool args() //arglist | "empty";
        {
        	if(argList()){}
        	return true;
        }

        bool argList() //expression, {",", expression};
        {
        	if(!expression()){
        		return false;
        	}
        	while(match(",")){
        		if(!expression()){
        			return false;
        		}
        	}
        	return true;
        }

        bool annoyingExpression() //annoyingAdditiveExpression, {relop, annoyingAdditiveExpression};
        {
        	if(!annoyingAdditiveExpression()){
        		return false;
        	}
        	while(relop()){
        		if(!annoyingAdditiveExpression()){
        			return false;
        		}
        	}
        	return true;
        }

        bool annoyingAdditiveExpression() //annoyingTerm, {addop, annoyingTerm};
        {
        	if(!annoyingTerm()){
        		return false;
        	}
        	while(addop()){
        		if(!annoyingTerm()){
        			return false;
        		}
        	}
        	return true;
        }

        bool annoyingTerm() //annoyingFactor, {mulop, annoyingFactor};
        {
        	if(!annoyingFactor()){
        		return false;
        	}
        	while(mulop()){
        		if(!annoyingFactor()){
        			return false;
        		}
        	}
        	return true;
        }

        bool annoyingFactor() //var | call;
        {
        	if(var()){}
        	else if(call()){}
        	else{
        		return false;
        	}
        	return true;
        }
    }

	bool parse(std::istream& input)
	{
		lex.setInput(input);
		currTok = lex.getNextToken();
	    if(program()){
	    	return true;
	    }
	    else{
	    	return false;
	    }
	}
}
