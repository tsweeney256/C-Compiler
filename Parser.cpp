#include <iostream>
#include <string>
#include <list>
#include <vector>
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
    	const std::vector<int>* getSignatureFromSymTabList(const std::string& name);
    	bool simpleExpressionHigherPrecAttach(const char op);
    	bool simpleExpressionAttach(const char op);
    	inline void attachLastChildTree();
    	void attachOperandTreeToChildTree();

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
    	bool additiveExpression();
    	bool term();
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
        std::string globalNameDecl;
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
        //This could probably actually be made into a 2d array, but it works and this is just
        //for a school assignment and I don't want to break anything so I won't.
        std::vector<std::pair<std::vector<std::vector<int> >, int > > exprType; //3dpd
        std::vector<int> exprTypeLevel; //initialized to one int of -1 in parse()
        std::vector<std::vector<Tree<SyntaxInfo>*> > childTree;
        std::vector<Tree<SyntaxInfo>*> operandTree;
        Tree<SyntaxInfo>* syntaxTree = NULL;
        int baseType = -1;
        int funType = -1;
        int lastTokFlag = -1;
        int expressionType = -1;
        int returnType = -1;
        int opFlag;
        std::vector<int> toPopBackinSE;
        size_t lastLineNum = 0;
        bool showingErrorMsgs;
        bool semanticError = false;
        bool funDeclScope = false;
        bool seenReturn = false;
        std::vector<bool> higherPrec;
        std::vector<bool> empty;
        //hack so expressions like "foo(x[foo(1)])" with mismatching parenthesis don't get accepted
        std::vector<bool> supposedToBeACall;

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

        bool simpleExpressionHigherPrecAttach(const char op)
        {
    		childTree.back().push_back(new Tree<SyntaxInfo>());
    		++toPopBackinSE.back();
    		childTree.back().back()->val.syntaxFlag = opFlag;
            attachLastChildTree();
            attachOperandTreeToChildTree();
            childTree.back().pop_back();
			if(!factor()){
				return false;
			}
			//dumb hack that makes sure that the precedence is actually right
			//there's most likely a better way to do this
			if(!((op == '>' && (currTok[0] == '+' || currTok[0] == '-' || currTok[0] == '*' || currTok[0] == '/')) ||
					((op == '+') && (currTok[0] == '*' || currTok[0] == '/')))){
				attachOperandTreeToChildTree();
				++toPopBackinSE.back();
			}
            return true;
        }

        bool simpleExpressionAttach(const char op)
        {
    		childTree.back().push_back(new Tree<SyntaxInfo>());
    		childTree.back().back()->val.syntaxFlag = opFlag;
            attachLastChildTree();
            ++toPopBackinSE.back();
			if(!factor()){
				return false;
			}
			if(!((op == '>' && (currTok[0] == '+' || currTok[0] == '-' || currTok[0] == '*' || currTok[0] == '/')) ||
					((op == '+') && (currTok[0] == '*' || currTok[0] == '/')))){
				attachOperandTreeToChildTree();
				++toPopBackinSE.back();
			}
            return true;
        }

        void attachLastChildTree()
        {
			childTree.back()[childTree.back().size()-2]->connectChild(childTree.back().back());
        }

        void attachOperandTreeToChildTree()
        {
        	childTree.back().push_back(operandTree.back());
        	attachLastChildTree();
        	operandTree.pop_back();
        }

        bool program() //declarationList;
        {
            syntaxTree->val.syntaxFlag = SyntaxInfo::PROGRAM;
            childTree.push_back(std::vector<Tree<SyntaxInfo>*>());
        	if(lex.eof()){
        		return false;
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
            else if(symTabList.front()->getSignature("main")->size() != 0){
            	semanticError = true;
            	if(showingErrorMsgs){
            		std::cout << "Error:" << lastLineNum << ": main function must be declared with void parameter list." << std::endl;
            	}
            }
            else if(globalNameDecl.compare("main")){
            	semanticError = true;
            	if(showingErrorMsgs){
            		std::cout << "Error:" << lastLineNum << ": main function needs to be last declaration in file." << std::endl;
            	}
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
            childTree.back().push_back(new Tree<SyntaxInfo>());
            syntaxTree->connectChild(childTree.back().back());
            if(match("int")){
                childTree.back().back()->val.typeFlag = SyntaxInfo::INT;
            	baseType = SymbolTable::INT;
            	nameDecl = currTok;
            	globalNameDecl = nameDecl;
            	if(!match(ID)){
            		return false;
            	}
                childTree.back().back()->val.name = globalNameDecl;
            	if(varDeclaration()){
            		childTree.back().back()->val.syntaxFlag = SyntaxInfo::VAR_DEC;
            	}
            	else if(funDeclaration()){}
            	else{
            		return false;
            	}
            }
            else if(match("float")){
                childTree.back().back()->val.typeFlag = SyntaxInfo::FLOAT;
            	baseType = SymbolTable::FLOAT;
            	nameDecl = currTok;
            	globalNameDecl = nameDecl;
            	childTree.back().back()->val.name = globalNameDecl;
            	if(!match(ID)){
            		return false;
            	}
            	if(varDeclaration()){
            		childTree.back().back()->val.syntaxFlag = SyntaxInfo::VAR_DEC;
            	}
            	else if(funDeclaration()){}
            	else{
            		return false;
            	}
            }
            else if(match("void")){
                childTree.back().back()->val.typeFlag = SyntaxInfo::VOID;
            	baseType = SymbolTable::VOID;
            	nameDecl = currTok;
            	globalNameDecl = nameDecl;
            	childTree.back().back()->val.name = globalNameDecl;
            	if(!(match(ID) && funDeclaration())){
            		return false;
            	}
            }
            else{
            	return false;
            }
            childTree.back().pop_back();
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
            	if(baseType == SymbolTable::INT){
            		if(!(*symTabList.rbegin())->add(nameDecl, SymbolTable::INT_ARRAY)){
            			duplicateDecl = true;
            		}
            		childTree.back().back()->val.typeFlag = SyntaxInfo::INT_ARRAY;
            	}
            	else if(baseType == SymbolTable::FLOAT){
            		if(!(*symTabList.rbegin())->add(nameDecl, SymbolTable::FLOAT_ARRAY)){
            			duplicateDecl = true;
            		}
            		childTree.back().back()->val.typeFlag = SyntaxInfo::FLOAT_ARRAY;
            	}
            	nameDecl = currTok;
            	if(match(NUM)){
            		childTree.back().push_back(new Tree<SyntaxInfo>());
            		childTree.back().back()->val.syntaxFlag = SyntaxInfo::INT_LITERAL;
                    childTree.back().back()->val.name = nameDecl;
                    attachLastChildTree();
                    childTree.back().pop_back();
            	}
            	else{
                    return false;
            	}
            	if(!(match("]") && match(";"))){
            		return false;
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
            childTree.back().back()->val.syntaxFlag = SyntaxInfo::FUN_DEC;
            return true;
        }

        bool params() //paramList | "void";
        {
            childTree.back().push_back(new Tree<SyntaxInfo>());
            childTree.back().back()->val.syntaxFlag = SyntaxInfo::PARAMS;
            attachLastChildTree();
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
			childTree.back().pop_back();
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
            childTree.back().push_back(new Tree<SyntaxInfo>());
            childTree.back().back()->val.syntaxFlag = SyntaxInfo::VAR_DEC;
            childTree.back().back()->val.typeFlag = baseType;
            attachLastChildTree();
            nameDecl = currTok;
            if(!match(ID)){
                return false;
            }
            childTree.back().back()->val.name = nameDecl;
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
            childTree.back().pop_back();
            return true;
        }

        bool compoundStmt() //"{", localDeclarations, stmtList, "}";
        {
        	childTree.back().push_back(new Tree<SyntaxInfo>());
        	attachLastChildTree();
        	childTree.back().back()->val.syntaxFlag = SyntaxInfo::COMPOUND_STMT;
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
            childTree.back().pop_back();
            return true;
        }

        bool localDeclarations() //{("int" | "float"), "ID", varDeclaration};
        {
        	//sure would be nice to use a closure to get rid of this code duplication...
        	childTree.back().push_back(new Tree<SyntaxInfo>());
        	childTree.back().back()->val.syntaxFlag = SyntaxInfo::LOCAL_DECS;
        	attachLastChildTree();
        	bool isDone = false;
        	while(!isDone){
        		if(match("int")){
        			baseType = SymbolTable::INT;
        			nameDecl = currTok;
        			childTree.back().push_back(new Tree<SyntaxInfo>());
        			childTree.back().back()->val.syntaxFlag = SyntaxInfo::VAR_DEC;
        			childTree.back().back()->val.typeFlag = SyntaxInfo::INT;
        			childTree.back().back()->val.name = nameDecl;
        			if(!(match(ID) && varDeclaration())){
        				isDone = true;
        				return false;
        			}
        			attachLastChildTree();
        			childTree.back().pop_back();
        		}
        		else if(match("float")){
        			baseType = SymbolTable::FLOAT;
        			nameDecl = currTok;
        			childTree.back().push_back(new Tree<SyntaxInfo>());
        			childTree.back().back()->val.syntaxFlag = SyntaxInfo::VAR_DEC;
        			childTree.back().back()->val.typeFlag = SyntaxInfo::FLOAT;
        			childTree.back().back()->val.name = nameDecl;
        			if(!(match(ID) && varDeclaration())){
						isDone = true;
						return false;
					}
        			attachLastChildTree();
        			childTree.back().pop_back();
        		}
        		else{
        			isDone = true;
        		}
        	}
            childTree.back().pop_back();
        	return true;
        }

        bool stmtList() //{statement}
        {
        	//instead of checking the next token to predict if it's an error or not, we just leave that to statement() to handle.
        	//This saves us from having to do redundant comparisons in the event of a well-formed program. The only downside is
        	//ill-formed programs have to make an extra function call. Whoopty doo.

        	childTree.back().push_back(new Tree<SyntaxInfo>());
        	attachLastChildTree();
        	childTree.back().back()->val.syntaxFlag = SyntaxInfo::STMT_LIST;
        	//can be empty, so need to peek at follow set to know if it's supposed to be empty
            while(currTok.compare("}")){
            	if(!statement()){
            		return false;
            	}
            }
            childTree.back().pop_back();
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
        	childTree.back().push_back(new Tree<SyntaxInfo>());
        	attachLastChildTree();
        	childTree.back().back()->val.syntaxFlag = SyntaxInfo::IF;
            if(!(match("if") && match("(") && expression() && match(")") && statement())){
                return false;
            }
            if(match("else")){
                if(!statement()){
                    return false;
                }
            }
            childTree.back().pop_back();
            return true;
        }

        bool iterationStmt() //"while(", expression, ")", statement;
        {
        	childTree.back().push_back(new Tree<SyntaxInfo>());
        	attachLastChildTree();
        	childTree.back().back()->val.syntaxFlag = SyntaxInfo::WHILE;
            if(!(match("while") && match("(") && expression() && match(")") && statement())){
                return false;
            }
            childTree.back().pop_back();
            return true;
        }

        bool returnStmt() //"return", [expression], ";";
        {
        	childTree.back().push_back(new Tree<SyntaxInfo>());
        	attachLastChildTree();
        	childTree.back().back()->val.syntaxFlag = SyntaxInfo::RETURN;
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
            childTree.back().pop_back();
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
                else if(var() && !supposedToBeACall.back()){
                	supposedToBeACall.pop_back();
                    if(match("=")){
                    	childTree.back().push_back(new Tree<SyntaxInfo>());
                    	childTree.back().back()->val.syntaxFlag = SyntaxInfo::ASSIGNMENT;
                    	attachLastChildTree();
                    	attachOperandTreeToChildTree();
                    	childTree.back().pop_back();
                        if(!expression()){
                            return false;
                        }
                        childTree.back().pop_back();
                    }
                    else if(simpleExpression()){}
                    else{
                        return false;
                    }
                }
                else{
                	supposedToBeACall.pop_back();
                    return false;
                }
            }
            else if(match("(")){
                if(!(expression() && match(")") && simpleExpression())){
                    return false;
                }
            }
            else if(match(NUM)){
            	operandTree.push_back(new Tree<SyntaxInfo>());
            	operandTree.back()->val.name = nameDecl;
            	exprType.back().first.push_back(std::vector<int>());
            	if(lastTokFlag == LexicalAnalyzer::FLOAT_LITERAL){
            		operandTree.back()->val.syntaxFlag = SyntaxInfo::FLOAT_LITERAL;
            		exprType.back().first.back().push_back(SymbolTable::FLOAT);
            	}
            	else{
            		operandTree.back()->val.syntaxFlag = SyntaxInfo::INT_LITERAL;
            		exprType.back().first.back().push_back(INT_LITERAL);
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
            	expressionType = exprType.back().first[0][0];
            	//if the first part of the expressin is an int literal, the whole expression itself could still be either an int or float
            	//so we go through every token type to find out the type of the whole expression
            	if(expressionType == INT_LITERAL){
					for(size_t i=0; i<exprType.back().first.size(); ++i){
						for(size_t j=0; j<exprType.back().first[i].size(); ++j){
							if(exprType.back().first[i][j] == SymbolTable::INT && expressionType == INT_LITERAL){
								expressionType = SymbolTable::INT;
							}
							else if(exprType.back().first[i][j] == SymbolTable::FLOAT &&
									(expressionType == INT_LITERAL || expressionType == SymbolTable::INT)){
								expressionType = SymbolTable::FLOAT;
							}
						}
					}
            	}
				bool done = false;
				//now we actually go through it to see if all the tokens' types are compatible with the expression's type
				for(size_t i=0; i<exprType.back().first.size(); ++i){
					for(size_t j=0; j<exprType.back().first[i].size(); ++j){
						if(exprType.back().first[i][j] != expressionType &&
								(expressionType != INT_LITERAL &&
								(exprType.back().first[i][j] != SymbolTable::INT || exprType.back().first[i][j] != SymbolTable::FLOAT)) &&
								(exprType.back().first[i][j] != INT_LITERAL &&
								(expressionType != SymbolTable::INT || expressionType != SymbolTable::FLOAT))){
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
            --exprTypeLevel.back();
            return true;
        }

        bool var() //["[", expression, "]"];
        {
        	operandTree.push_back(new Tree<SyntaxInfo>());
        	operandTree.back()->val.syntaxFlag = SyntaxInfo::VAR;
        	operandTree.back()->val.name = nameDecl;
        	if(match("[")){
        		childTree.push_back(std::vector<Tree<SyntaxInfo>*>());
        		childTree.back().push_back(new Tree<SyntaxInfo>());
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
        		//hack to get around having to create a bogus parent tree earlier when creating the new second dimension
        		operandTree.back()->connectChild(childTree.back().back()->getChild(0));
        		Tree<SyntaxInfo>::destroyNode(childTree.back().back());
        		childTree.pop_back();
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

        bool simpleExpression() //additiveExpression, [relop, factor, additiveExpression]];
                                //additiveExpression can be empty
        {
        	empty.push_back(true);
        	toPopBackinSE.push_back(0);
        	higherPrec.push_back(true);
        	if(additiveExpression()){
				if(relop()){
					if(empty.back()){
						if(!simpleExpressionHigherPrecAttach('>')){
							return false;
						}
					}
					else if(!simpleExpressionAttach('>'))
					{
						return false;
					}
					empty.back() = false;
					higherPrec.back() = true;
					if(additiveExpression()){}
					else{
						return false;
					}
				}
        	}
        	else{
        		return false;
        	}
        	for(int i=0; i<toPopBackinSE.back(); ++i){
        		childTree.back().pop_back();
        	}
        	if(empty.back()){
        		attachOperandTreeToChildTree();
        		childTree.back().pop_back();
        	}
        	toPopBackinSE.pop_back();
        	empty.pop_back();
        	higherPrec.pop_back();
        	return true;
        }

        bool additiveExpression() //term, {addop, factor}, term;
                                  //term can be empty
        {
        	if(term()){
        		if(addop()){
        			empty.back() = false;
        			if(higherPrec.back()){
        				higherPrec.back() = false;
                		if(!simpleExpressionHigherPrecAttach('+')){ //calls factor()
                			return false;
                		}
        			}
        			else{
        				if(!simpleExpressionAttach('+')){ //calls factor()
        					return false;
        				}
        			}
        			if(term()){}
        			while(addop()){
						if(!simpleExpressionAttach('+')){ //calls factor()
							return false;
						}
						if(term()){}
						else{
							return false;
						}
					}
        		}
        	}
        	else{
        		return false;
        	}
        	return true;
        }

        bool term() //{mulop, factor};
        {
        	if(mulop()){
        		higherPrec.back() = false;
        		empty.back() = false;
        		if(!simpleExpressionHigherPrecAttach('*')){ //calls factor()
        			return false;
        		}
        		while(mulop()){
					if(!simpleExpressionAttach('*')){ //calls factor()
						return false;
					}
				}
        	}
        	return true;
        }

        bool relop() //"<=" | "<" | ">" | ">=" | "==" | "!=";
        {
        	if(match("<=")){
        		opFlag = SyntaxInfo::LTEQ;
        	}
        	else if(match("<")){
        		opFlag = SyntaxInfo::LT;
        	}
        	else if(match(">")){
        		opFlag = SyntaxInfo::GT;
        	}
        	else if(match(">=")){
        		opFlag = SyntaxInfo::GTEQ;
        	}
        	else if(match("==")){
        		opFlag = SyntaxInfo::EQ;
        	}
        	else if(match("!=")){
        		opFlag = SyntaxInfo::NEQ;
        	}
        	else{
        		return false;
        	}
        	return true;
        }

        bool addop() //"+" | "-";
        {
        	if(match("+")){
        		opFlag = SyntaxInfo::ADD;
        	}
        	else if(match("-")){
        		opFlag = SyntaxInfo::SUB;
        	}
        	else{
        		return false;
        	}
        	return true;
        }

        bool mulop() //"*" | "/";
        {
        	if(match("*")){
        		opFlag = SyntaxInfo::MULT;
        	}
        	else if(match("/")){
        		opFlag = SyntaxInfo::DIV;
        	}
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
                else if(var() && !supposedToBeACall.back()){
                	supposedToBeACall.pop_back();
                }
                else{
                	supposedToBeACall.pop_back();
                    return false;
                }
        	}
        	else if(match(NUM)){
            	operandTree.push_back(new Tree<SyntaxInfo>());
            	operandTree.back()->val.name = nameDecl;
            	exprType.back().first.push_back(std::vector<int>());
            	if(lastTokFlag == LexicalAnalyzer::FLOAT_LITERAL){
            		operandTree.back()->val.syntaxFlag = SyntaxInfo::FLOAT_LITERAL;
            		exprType.back().first.back().push_back(SymbolTable::FLOAT);
            	}
            	else{
            		operandTree.back()->val.syntaxFlag = SyntaxInfo::INT_LITERAL;
            		exprType.back().first.back().push_back(INT_LITERAL);
            	}
        	}
        	else{
        		return false;
        	}
        	return true;
        }

        bool call() //"(", args, ")";
        {
        	supposedToBeACall.push_back(false);
        	if(!currTok.compare("(")){
        		supposedToBeACall.back() = true;
            	operandTree.push_back(new Tree<SyntaxInfo>());
            	operandTree.back()->val.syntaxFlag = SyntaxInfo::CALL;
            	operandTree.back()->val.name = nameDecl;
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
        	supposedToBeACall.pop_back();
        	return true;
        }

        bool args() //[arglist];
        { //arglist can be empty so we have to check for the follow set to make sure that it actually is or not
        	calledFunc = nameDecl;
        	if(currTok.compare(")")){
        		if(!argList()){
        			//hack to prevent a segfault
        			exprType.back().first.push_back(std::vector<int>());
        			exprType.back().first.back().push_back(-1);
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

    		childTree.push_back(std::vector<Tree<SyntaxInfo>*>());
    		childTree.back().push_back(new Tree<SyntaxInfo>());
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
    		operandTree.back()->connectChild(childTree.back().back()->getChild(0));
    		Tree<SyntaxInfo>::destroyNode(childTree.back().back());
    		childTree.pop_back();
        	while(match(",")){
        		childTree.push_back(std::vector<Tree<SyntaxInfo>*>());
        		childTree.back().push_back(new Tree<SyntaxInfo>());
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
        		operandTree.back()->connectChild(childTree.back().back()->getChild(0));
        		Tree<SyntaxInfo>::destroyNode(childTree.back().back());
        		childTree.pop_back();
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
        syntaxTree = new Tree<SyntaxInfo>();
	    if(program() && !semanticError){
	    	delete *symTabList.begin();
	    	return syntaxTree;
	    }
	    else{
	    	//need to make sure to delete the symbol table for all scopes in case of incorrect number of brackets
	    	for(std::list<SymbolTable*>::const_reverse_iterator it = symTabList.rbegin(); it != symTabList.rend(); ++it){
	    		delete *it;
	    	}
	    	Tree<SyntaxInfo>::destroy(syntaxTree);
	    	return NULL;
	    }
	}
}
