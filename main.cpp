#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <list>
#include "LexicalAnalyzer.hpp"

int main(int argc, char* argv[])
{
	if(argc != 2){
		std::cerr << "Error: Program only accepts a single argument, the location of the input file." << std::endl;
		exit(1);
	}
	std::fstream inputFile;
	inputFile.open(argv[1], std::fstream::in);
	if(!inputFile.is_open()){
		std::cerr << "Error: File does not exist." << std::endl;
		exit(1);
	}

	LexicalAnalyzer lex;
	std::list<void*> symTabList; //to simulate the list of symbol tables that will be built later

	//END_OF_INPUT not guaranteed to be equal to the number of flags, but it will at least work for this project
	//this map won't even be needed for the finished compiler anyway
	std::string tokenFlagStringMap[LexicalAnalyzer::END_OF_INPUT+1];
	tokenFlagStringMap[LexicalAnalyzer::KEYWORD] = "KEYWORD";
	tokenFlagStringMap[LexicalAnalyzer::OPERATOR] = "OPERATOR";
	tokenFlagStringMap[LexicalAnalyzer::INT_LITERAL] = "INT LITERAL";
	tokenFlagStringMap[LexicalAnalyzer::FLOAT_LITERAL] = "FLOAT LITERAL";
	tokenFlagStringMap[LexicalAnalyzer::ID] = "ID";
	tokenFlagStringMap[LexicalAnalyzer::ERROR] = "ERROR";
	tokenFlagStringMap[LexicalAnalyzer::END_OF_INPUT] = "EOL"; //for debugging

	lex.setInput(inputFile);
	while(!lex.eof()){
		std::string token = lex.getNextToken();

		//a super simple pseudo-grammar that responds to '{' and '}' to keep track of scope
		if(!token.compare("{")){
            symTabList.push_back(NULL);
		}
		else if(!token.compare("}") && !symTabList.empty()){
            symTabList.pop_back();
		}
		if(lex.lastTokenFlag() == LexicalAnalyzer::ID){
            std::cout << tokenFlagStringMap[lex.lastTokenFlag()] << ": " << token << "\tSCOPE: " << symTabList.size() << std::endl;
		}
		else if(lex.lastTokenFlag() != LexicalAnalyzer::END_OF_INPUT){
            std::cout << tokenFlagStringMap[lex.lastTokenFlag()] << ": " << token << std::endl;
		}
	}
}
