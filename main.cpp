#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <list>
#include <string>

#include "Parser.hpp"
#include "Tree.hpp"
#include "Tree.hpp"
#include "SyntaxInfo.hpp"

int main(int argc, char* argv[])
{
	const char* inputFileLoc;
	bool errorMsgsEnabled = false;
	bool printTreeEnabled = false;

	if(argc < 2){
		std::cout << "Error: Program needs an input file to compile given as an argument" << std::endl;
		exit(1);
	}
	for(int i=1; i<argc; ++i){
		bool foundInput = false;
		if(argv[i][0] == '-'){
			if(strlen(argv[i]) != 2){
				std::cout << "Error: unknown argument: " << argv[i] << std::endl;
				exit(1);
			}
			else if(argv[i][1] == 'e'){
				errorMsgsEnabled = true;
			}
			else if(argv[i][1] == 't'){
				printTreeEnabled = true;
			}
			else{
				std::cout << "Error: unkown argument " << argv[i] << std::endl;
				exit(1);
			}
		}
		else if(!foundInput){
			inputFileLoc = argv[i];
		}
		else{
			std::cout << "Error: unkown argument " << argv[i] << std::endl;
			exit(1);
		}
	}

	if(argc == 3){
		if(!strcmp("-e", argv[2])){
			errorMsgsEnabled = true;
		}
		else{
			std::cout << "unknown argument: " << argv[2] << std::endl;
		}
	}
	std::fstream inputFile;
	inputFile.open(inputFileLoc, std::fstream::in);
	if(!inputFile.is_open()){
		std::cout << "Error: File does not exist." << std::endl;
		exit(1);
	}
	Tree<SyntaxInfo>* syntaxTree = Parser::parse(inputFile, errorMsgsEnabled);
	if(syntaxTree){
		std::cout << "ACCEPT" << std::endl;
		if(printTreeEnabled){
			std::ofstream treeFile;
			treeFile.open(std::string(std::string(inputFileLoc) + std::string(".tree.txt")).c_str(), std::ofstream::out);
			syntaxTree->print(treeFile);
		}
	}
	else{
		std::cout << "REJECT" << std::endl;
	}
	Tree<SyntaxInfo>::destroy(syntaxTree);

	return 0;
}
