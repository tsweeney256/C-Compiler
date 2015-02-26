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
			ID,
			NUM
		};

    	bool match(const std::string& str);
    	bool match(int flag);

    	bool program();
    	bool declarationList();
    	bool declaration();
    	bool varDeclaration();
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
    	bool addop();
    	bool mulop();
    	bool factor();
    	bool call();
    	bool args();
    	bool argList();

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

        bool declaration() //(("int" | "float"), "ID", (varDeclaration | funDeclaration)) | ("void", "ID", funDeclaration);
        {
            if(match("int")){
            	if(!match(ID)){
            		return false;
            	}
            	if(varDeclaration()){}
            	else if(funDeclaration()){}
            	else{
            		return false;
            	}
            }
            else if(match("float")){
            	if(!match(ID)){
            		return false;
            	}
            	if(varDeclaration()){}
            	else if(funDeclaration()){}
            	else{
            		return false;
            	}
            }
            else if(match("void")){
            	if(!(match(ID) && funDeclaration())){
            		return false;
            	}
            }
            else{
            	return false;
            }
            return true;
        }

        bool varDeclaration() //["[NUM]"], ";";
        {
            if(match(";")){}
            else if(match("[")){
            	if(!(match(NUM) && match("]") && match(";"))){
            		return false;
            	}
            }
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

        bool params() //paramList | "void";
        {
        	if(paramList()){}
        	else if(match("void")){}
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
            while(match(",")){
                if(!oneParam()){
                    return false;
                }
            }
            return true;
        }

        bool oneParam() //("int" | "float"), "ID", ["[]"];
        {
            if(match("int")){}
            else if(match("float")){}
            else{
                return false;
            }
            if(!match(ID)){
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

        bool localDeclarations() //{("int" | "float"), "ID", varDeclaration};
        {
        	bool isDone = false;
        	while(!isDone){
        		if(match("int") || match("float")){
        			if(!(match(ID) && varDeclaration())){
        				isDone = true;
        				return false;
        			}
        		}
        		else{
        			isDone = true;
        		}
        	}
        	return true;
        }

        bool stmtList() //{statement}
        { //can be empty, so need to peek at follow set to know if it's supposed to be empty
            while(currTok.compare("}")){
            	if(!statement()){
            		return false;
            	}
            }
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
            if(match(";")){}
            else if(expression()){
            	if(!match(";")){
            		return false;
            	}
            }
            else{
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

        bool expression() //("ID", (call, simpleExpression) | (var, ("=", expression) | simpleExpression)) |
                          //((("(", expression, ")") | "NUM"), simpleExpression);
        {
            if(match(ID)){
                if(call()){
                    if(!simpleExpression()){
                        return false;
                    }
                }
                else if(var()){
                    if(match("=")){
                        if(!expression()){
                            return false;
                        }
                    }
                    else if(simpleExpression()){}
                    else{
                        return false;
                    }
                }
                else{
                    return false;
                }
            }
            else if(match("(")){
                if(!(expression() && match(")") && simpleExpression())){
                    return false;
                }
            }
            else if(match(NUM)){
                if(!simpleExpression()){
                    return false;
                }
            }
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

        bool simpleExpression() //{mulop, factor}, {addop, factor, {mulop, factor}},
                                //{relop, factor, {mulop, factor}, {addop, factor, {mulop, factor}}};
        {
        	while(mulop()){
                if(!factor()){
                    return false;
                }
        	}
        	while(addop()){
                if(!factor()){
                    return false;
                }
                while(mulop()){
                    if(!factor()){
                        return false;
                    }
                }
        	}
        	while(relop()){
                if(!factor()){
                    return false;
                }
                while(mulop()){
                    if(!factor()){
                        return false;
                    }
                }
                while(addop()){
                    if(!factor()){
                        return false;
                    }
                    while(mulop()){
                        if(!factor()){
                            return false;
                        }
                    }
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

        bool addop() //"+" | "-";
        {
        	if(match("+")){}
        	else if(match("-")){}
        	else{
        		return false;
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

        bool factor() //("(", expression, ")") | ("ID", (var | call)) | "NUM"
        {
        	if(match("(")){
        		if(!(expression() && match(")"))){
        			return false;
        		}
        	}
        	else if(match(ID)){
                if(var()){}
                else if(call()){}
                else{
                    return false;
                }
        	}
        	else if(match(NUM)){}
        	else{
        		return false;
        	}
        	return true;
        }

        bool call() //"(", args, ")";
        {
        	if(!(match("(") && args() && match(")"))){
        		return false;
        	}
        	return true;
        }

        bool args() //[arglist];
        { //arglist can be empty so we have to check for the follow set to make sure that it actually is or not
        	while(currTok.compare(")")){
        		if(!argList()){
        			return false;
        		}
        	}
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
