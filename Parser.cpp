#include <iostream>
#include <string>
#include <list>
#include <cstdlib>
#include <utility>
#include "Parser.hpp"
#include "LexicalAnalyzer.hpp"
#include "SymbolTable.hpp"
#include "Tree.hpp"
#include "SyntaxInfo.hpp"

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
    	int peekInSymTabList(const std::string& name);

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

    	//dumb hack to make numerical literals work right
    	//int literals need to be able to decay into float literals.
    	//needed because SymbolTable doesn't have an enum for int literals
    	const int INT_LITERAL = -500;
    	const int FUNC_ARG = -501;

    	LexicalAnalyzer lex;
        std::string currTok;
        std::string nameDecl;
        std::string funDecl;
        std::string calledFunc;
        std::list<SymbolTable*> symTabList;
        std::vector<int>* sigList = NULL;
        const std::vector<int>* signature;
        //the second part of std::pair is for storing a flag to see if the expression is for a
        //array index, function argument, or neither
        //the first dimension ([][][*]) is for storing what type the expression is
        //the second dimeninsion ([][*][]) acts as a sort of stack to support the recursive nature of
        //expressions and allow sub-expressions, e.g. expression + (sub-expression + (sub-sub-expression))
        //the third dimension ([*][][]) allows for each recursive expression to have different types when
        //need be. Like for instance, func(expr1, expr2). func, expr1, and expr2 all need to be able to
        //have their own type to support calling functions that have parameters of different types.
        //it also allows us to index arrays of floats with integers.
        std::vector<std::pair<std::vector<std::vector<int> >, int > > exprType; //3dpd
        std::vector<int> exprTypeLevel; //initialized to one int of -1 in parse()
        int baseType = -1;
        int funType = -1;
        int lastTokFlag = -1;
        int expressionType = -1;
        int returnType = -1;
        size_t lastLineNum = 0;
        bool showingErrorMsgs;
        bool semanticError = false;
        bool funDeclScope = false;
        bool seenReturn = false;

        bool match(const std::string& str)
        {
        	if(str.compare(currTok)){
        		return false;
        	}
        	lastLineNum = lex.getCurrLine();
        	lastTokFlag = lex.getCurrTokenFlag();
        	currTok = lex.getNextToken();
        	return true;
        }

        bool match(int flag)
        {
        	if(flag == ID){
        		if(lex.getCurrTokenFlag() != LexicalAnalyzer::ID){
        			return false;
        		}
        	}
        	else if(flag == NUM){
        		if(!(lex.getCurrTokenFlag() == LexicalAnalyzer::INT_LITERAL ||
        				lex.getCurrTokenFlag() == LexicalAnalyzer::FLOAT_LITERAL)){
        			return false;
        		}
        	}
        	else{
        		std::cerr << "Error: Incorrect flag given" << std::endl;
        		exit(1);
        	}
        	lastLineNum = lex.getCurrLine();
        	lastTokFlag = lex.getCurrTokenFlag();
        	currTok = lex.getNextToken();
        	return true;
        }

        int peekInSymTabList(const std::string& name)
		{
			int flag = -1;
			for(std::list<SymbolTable*>::const_reverse_iterator it = symTabList.rbegin(); it != symTabList.rend(); ++it){
				if((flag = (*it)->peek(name)) != SymbolTable::NOT_FOUND){
					return flag;
				}
			}
			return SymbolTable::NOT_FOUND;

		}

        const std::vector<int>* getSignatureFromSymTabList(const std::string& name)
        {
			int flag = -1;
			for(std::list<SymbolTable*>::const_reverse_iterator it = symTabList.rbegin(); it != symTabList.rend(); ++it){
				if((flag = (*it)->peek(name)) != SymbolTable::NOT_FOUND){
					return (*it)->getSignature(name);
				}
			}
			return NULL;
        }

        bool program() //declarationList;
        {
        	if(lex.eof()){
        		return true;
        	}
            if(!declarationList()){
                return false;
            }
            else if((*symTabList.begin())->peek("main") == SymbolTable::NOT_FOUND){
            	if(showingErrorMsgs){
            		std::cout << "Error:" << lastLineNum << ": main function not found." << std::endl;
            	}
            	semanticError = true;
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
            while(!lex.eof()){
            	if(!declaration()){
            		return false;
            	}
            }
            return true;
        }

        bool declaration() //(("int" | "float"), "ID", (varDeclaration | funDeclaration)) | ("void", "ID", funDeclaration);
        {
            if(match("int")){
            	baseType = SymbolTable::INT;
            	nameDecl = currTok;
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
            	baseType = SymbolTable::FLOAT;
            	nameDecl = currTok;
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
            	baseType = SymbolTable::VOID;
            	nameDecl = currTok;
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
        	bool duplicateDecl = false;

            if(match(";")){
            	if(!(*symTabList.rbegin())->add(nameDecl, baseType)){
            		duplicateDecl = true;
        		}
            }
            else if(match("[")){
            	if(!currTok.compare("0")){
            		semanticError = true;
            		if(showingErrorMsgs){
            			std::cout << "Error:" << lastLineNum << ": Array declaration must have more than 0 elements" << std::endl;
            		}
            	}
            	if(!(match(NUM) && match("]") && match(";"))){
            		return false;
            	}
            	if(baseType == SymbolTable::INT){
            		if(!(*symTabList.rbegin())->add(nameDecl, SymbolTable::INT_ARRAY)){
            			duplicateDecl = true;
            		}
            	}
            	else if(baseType == SymbolTable::FLOAT){
            		if(!(*symTabList.rbegin())->add(nameDecl, SymbolTable::FLOAT_ARRAY)){
            			duplicateDecl = true;
            		}
            	}
            }
            else{
            	return false;
            }
            if(duplicateDecl){
            	semanticError = true;
            	if(showingErrorMsgs){
            		std::cout << "Error:" << lastLineNum << ": '" << nameDecl << "' declared multiple times in the same scope." << std::endl;
            	}
            }
            return true;
        }

        bool funDeclaration() //"(", params, ")", compoundStmt;
        {
        	funDecl = nameDecl;
        	funType = baseType;
        	if(funType == SymbolTable::VOID){
        		seenReturn = true;
        		returnType = SymbolTable::VOID;
        	}
        	else{
        		seenReturn = false;
        	}
        	sigList = new std::vector<int>();
            if(!(match("(") && params() && match(")") && compoundStmt())){
            	return false;
            }
            if(!seenReturn){
            	semanticError = true;
            	if(showingErrorMsgs){
            		std::cout << "Error:" << lastLineNum << ": Function has no return statement." << std::endl;
            	}
            }
            return true;
        }

        bool params() //paramList | "void";
        {
        	funDeclScope = true;
        	symTabList.push_back(new SymbolTable());
        	if((!currTok.compare("int") || !currTok.compare("float"))){
        		if(!paramList()){
        			return false;
        		}
        	}
        	else if(match("void")){}
        	else{
                return false;
        	}
        	if(!symTabList.front()->add(funDecl, funType, sigList)){ //function declarations are always at global scope
				semanticError = true;
				if(showingErrorMsgs){
					std::cout << "Error:" << lastLineNum << ": '" << funDecl << "' declared multiple times in the same scope." << std::endl;
				}
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
        	bool duplicateDecl = false;

            if(match("int")){
            	baseType = SymbolTable::INT;
            }
            else if(match("float")){
            	baseType = SymbolTable::FLOAT;
            }
            else{
                return false;
            }
            nameDecl = currTok;
            if(!match(ID)){
                return false;
            }
            if(match("[")){
                if(!match("]")){
                    return false;
                }
                if(baseType == SymbolTable::INT){
                	baseType = SymbolTable::INT_ARRAY;
                }
                else if(baseType == SymbolTable::FLOAT){
                	baseType = SymbolTable::FLOAT_ARRAY;
                }
            }
            if(!(*symTabList.rbegin())->add(nameDecl, baseType)){
            	duplicateDecl = true;
			}
            sigList->push_back(baseType);
            if(duplicateDecl){
            	semanticError = true;
            	if(showingErrorMsgs){
            		std::cout << "Error:" << lastLineNum << ": '" << nameDecl << "' declared multiple times in parameter list." << std::endl;
            	}
            }
            return true;
        }

        bool compoundStmt() //"{", localDeclarations, stmtList, "}";
        {
        	if(!funDeclScope){
        		symTabList.push_back(new SymbolTable());
        	}
        	else{
        		funDeclScope = false;
        	}
            if(match("{") && localDeclarations() && stmtList() && match("}")){}
            else{
                return false;
            }
            delete *symTabList.rbegin();
            symTabList.pop_back();
            return true;
        }

        bool localDeclarations() //{("int" | "float"), "ID", varDeclaration};
        {
        	//sure would be nice to use a closure to get rid of this code duplication...
        	bool isDone = false;
        	while(!isDone){
        		if(match("int")){
        			baseType = SymbolTable::INT;
        			nameDecl = currTok;
        			if(!(match(ID) && varDeclaration())){
        				isDone = true;
        				return false;
        			}
        		}
        		else if(match("float")){
        			baseType = SymbolTable::FLOAT;
        			nameDecl = currTok;
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
        {
        	//instead of checking the next token to predict if it's an error or not, we just leave that to statement() to handle.
        	//This saves us from having to do redundant comparisons in the event of a well-formed program. The only downside is
        	//ill-formed programs have to make an extra function call. Whoopty doo.

        	//can be empty, so need to peek at follow set to know if it's supposed to be empty
            while(currTok.compare("}")){
            	if(!statement()){
            		return false;
            	}
            }
            return true;
        }

        bool statement() //expressionStmt | compoundStmt | selectionStmt | iterationStmt | returnStmt
        {
        	if(lex.getCurrTokenFlag() == LexicalAnalyzer::ID || lex.getCurrTokenFlag() == LexicalAnalyzer::INT_LITERAL ||
        			lex.getCurrTokenFlag() == LexicalAnalyzer::FLOAT_LITERAL || !currTok.compare("(") || !currTok.compare(";")){
        		if(!expressionStmt()){
        			return false;
        		}
        	}
        	else if(!currTok.compare("{")){
        		if(!compoundStmt()){
        			return false;
        		}
        	}
        	else if(!currTok.compare("if")){
        		if(!selectionStmt()){
        			return false;
        		}
        	}
            else if(!currTok.compare("while")){
            	if(!iterationStmt()){
            		return false;
            	}
            }
            else if(!currTok.compare("return")){
            	if(!returnStmt()){
            		return false;
            	}
            }
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
            	seenReturn = true;
                if(expression()){
                	if(funType != SymbolTable::VOID){
                		returnType = expressionType;
                	}
                	else{
                		semanticError = true;
                		if(showingErrorMsgs){
                			std::cout << "Error:" << lastLineNum << ": void functions may not have an expression after a return." << std::endl;
                		}
                	}
                }
                else{
                	returnType = SymbolTable::VOID;
                }
                if(returnType != funType && !(returnType == INT_LITERAL && (funType == SymbolTable::FLOAT || funType == SymbolTable::INT))){
					semanticError = true;
					if(showingErrorMsgs){
						std::cout << "Error:" << lastLineNum << ": Function type and return type mismatch." << std::endl;
					}
				}
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
        	int idType = -1;

        	++exprTypeLevel.back();
        	nameDecl = currTok;
            if(match(ID)){
            	exprType.back().first.push_back(std::vector<int>());
            	if((idType = peekInSymTabList(nameDecl)) == SymbolTable::NOT_FOUND){
            		exprType.back().first.back().push_back(idType);
					semanticError = true;
					if(showingErrorMsgs){
						std::cout << "Error:" << lastLineNum << ": '" << nameDecl << "' not found." << std::endl;
					}
				}
            	else{
            		exprType.back().first.back().push_back(idType);
            		signature = getSignatureFromSymTabList(nameDecl);
            	}
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
            	if(lastTokFlag == LexicalAnalyzer::FLOAT_LITERAL){
            		exprType.back().first.push_back(std::vector<int>());
            		exprType.back().first.back().push_back(SymbolTable::FLOAT);
            	}
                if(!simpleExpression()){
                    return false;
                }
            }
            else{
            	--exprTypeLevel.back(); //for "return;"
                return false;
            }
            if(exprTypeLevel.back() == 0){
            	if(exprType.back().first.size() != 0){ //this can happen when there's just a lone int literal
					expressionType = exprType.back().first[0][0];
					bool done = false;
					for(size_t i=0; i<exprType.back().first.size(); ++i){
						for(size_t j=0; j<exprType.back().first[i].size(); ++j){
							if(exprType.back().first[i][j] != expressionType){
								semanticError = true;
								if(showingErrorMsgs){
									std::cout << "Error:" << lastLineNum << ": Type mismatch." << std::endl;
									done = true;
									break;
								}
							}
						}
						if(done){
							break;
						}
					}
					exprType.back().first.clear();
				}
            	else{
            		expressionType = INT_LITERAL;
            	}
            }
            --exprTypeLevel.back();
            return true;
        }

        bool var() //["[", expression, "]"];
        {
        	if(match("[")){
        		exprTypeLevel.push_back(-1);
				if(exprType.back().first.back().back() == SymbolTable::INT_ARRAY){
					exprType.back().first.back().back() = SymbolTable::INT;
				}
				else if(exprType.back().first.back().back() == SymbolTable::FLOAT_ARRAY){
					exprType.back().first.back().back() = SymbolTable::FLOAT;
				}
				else{
					semanticError = true;
					if(showingErrorMsgs){
						std::cout << "Error:" << lastLineNum << ": Trying to index non-array type." << std::endl;
					}
				}
				exprType.push_back(std::make_pair(std::vector<std::vector<int> >(), static_cast<int>(SymbolTable::INT_ARRAY)));
        		if(!(expression() && match("]"))){
        			exprTypeLevel.pop_back();
        			exprType.pop_back();
        			return false;
        		}
        		if(expressionType != SymbolTable::INT && expressionType != INT_LITERAL){
					semanticError = true;
					if(showingErrorMsgs){
						std::cout << "Error:" << lastLineNum <<": Trying to index array with non-int type." << std::endl;
					}
				}
        		exprTypeLevel.pop_back();
        		exprType.pop_back();
        	}
        	else if((exprType.back().first.back().back() == SymbolTable::INT_ARRAY ||
        			exprType.back().first.back().back() == SymbolTable::FLOAT_ARRAY) &&
        			exprType.back().second != FUNC_ARG){
        		semanticError= true;
        		if(showingErrorMsgs){
        			std::cout << "Error:" << lastLineNum << ": Trying to use array without indexing it." << std::endl;
        		}
        	}
        	return true;
        }

        bool simpleExpression() //{mulop, factor}, {addop, factor, {mulop, factor}},
                                //[relop, factor, {mulop, factor}, {addop, factor, {mulop, factor}}];
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
        	if(relop()){
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
        	nameDecl = currTok; //might possibly be an ID
        	if(match("(")){
        		if(!(expression() && match(")"))){
        			return false;
        		}
        	}
        	else if(match(ID)){
        		int idType = -1;

        		if((idType = peekInSymTabList(nameDecl)) == SymbolTable::NOT_FOUND){
					semanticError = true;
					if(showingErrorMsgs){
						std::cout << "Error:" << lastLineNum << ": '" << nameDecl << "' not found." << std::endl;
					}
				}
        		else{
        			exprType.back().first.back().push_back(idType);
            		signature = getSignatureFromSymTabList(nameDecl);
        		}
                if(call()){}
                else if(var()){}
                else{
                    return false;
                }
        	}
        	else if(match(NUM)){
        		if(lastTokFlag == LexicalAnalyzer::FLOAT_LITERAL){
        			exprType.back().first.back().push_back(SymbolTable::FLOAT);
				}
        	}
        	else{
        		return false;
        	}
        	return true;
        }

        bool call() //"(", args, ")";
        {
        	if(!currTok.compare("(")){
        		if(!signature){
        			semanticError = true;
        			if(showingErrorMsgs){
        				std::cout << "Error:" << lastLineNum << ": '" << nameDecl <<  "' does not refer to a function." << std::endl;
        			}
        		}
        	}
        	else if(signature){
    			semanticError = true;
    			if(showingErrorMsgs){
    				std::cout << "Error:" << lastLineNum << ": '" << nameDecl <<  "' refers to a function. \"()\" required" << std::endl;
    			}
    		}
        	if(!(match("(") && args() && match(")"))){
        		return false;
        	}
        	return true;
        }

        bool args() //[arglist];
        { //arglist can be empty so we have to check for the follow set to make sure that it actually is or not
        	calledFunc = nameDecl;
        	if(currTok.compare(")")){
        		if(!argList()){
        			return false;
        		}
        	}
        	else if(signature && signature->size()){
        		semanticError = true;
        		if(showingErrorMsgs){
        			std::cout << "Error:" << lastLineNum << ": call to " << calledFunc << "() has mismatching number of arguments to parameters." << std::endl;
        		}
        	}
        	return true;
        }

        bool argList() //expression, {",", expression};
        {
        	std::vector<int> types;
        	const std::vector<int>* sig = signature;

        	exprTypeLevel.push_back(-1);
        	exprType.push_back(std::make_pair(std::vector<std::vector<int> >(), FUNC_ARG));
        	if(!expression()){
        		exprTypeLevel.pop_back();
        		exprType.pop_back();
        		return false;
        	}
        	else{
        		types.push_back(expressionType);
        	}
        	exprTypeLevel.pop_back();
        	exprType.pop_back();
        	while(match(",")){
        		exprTypeLevel.push_back(-1);
        		exprType.push_back(std::make_pair(std::vector<std::vector<int> >(), FUNC_ARG));
        		if(!expression()){
        			return false;
        		}
            	else{
            		types.push_back(expressionType);
            	}
        		exprTypeLevel.pop_back();
        		exprType.pop_back();
        	}
        	if(sig && types.size() != sig->size()){
        		semanticError = true;
        		if(showingErrorMsgs){
        			std::cout << "Error:" << lastLineNum << ": call to " << calledFunc << "() has mismatching number of arguments to parameters." << std::endl;
        		}
        	}
        	else{
        		for(size_t i=0; i<types.size(); ++i){
        			if(sig && (types[i] != (*sig)[i] && !(types[i] == INT_LITERAL && ((*sig)[i] == SymbolTable::FLOAT || (*sig)[i] == SymbolTable::INT)))){
        				semanticError = true;
        				if(showingErrorMsgs){
        					std::cout << "Error:" << lastLineNum << ": call to " << calledFunc << "() has mismatching argument and parameter types." << std::endl;
        				}
        			}
        		}
        	}
        	return true;
        }
    }

	Tree<SyntaxInfo>* parse(std::istream& input, bool errorMsgsEnabled)
	{
		showingErrorMsgs = errorMsgsEnabled;
		lex.setInput(input);
		currTok = lex.getNextToken();
		symTabList.push_back(new SymbolTable());
		exprTypeLevel.push_back(-1);
		//the second value in this pair doesn't actually matter since exprType[0] will always be the base
		exprType.push_back(std::make_pair(std::vector<std::vector<int> >(), static_cast<int>(SymbolTable::INT)));
	    if(program() && !semanticError){
	    	delete *symTabList.begin();
	    	return new Tree<SyntaxInfo>(); //CHANGE THIS!!!!!
	    }
	    else{
	    	//need to make sure to delete the symbol table for all scopes in case of incorrect number of brackets
	    	for(std::list<SymbolTable*>::const_reverse_iterator it = symTabList.rbegin(); it != symTabList.rend(); ++it){
	    		delete *it;
	    	}
	    	return NULL;
	    }
	}
}
