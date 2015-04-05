#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <list>

#include "Parser.hpp"
#include "Tree.hpp"

int main(int argc, char* argv[])
{
	bool errorMsgsEnabled = false;

	if(!(argc == 2 || argc == 3)){
		std::cerr << "Error: Program only accepts one or two arguments: the location of the input file and optionally an error message flag (-e)." << std::endl;
		exit(1);
	}
	if(argc == 3){
		if(!strcmp("-e", argv[2])){
			errorMsgsEnabled = true;
		}
		else{
			std::cerr << "unknown argument: " << argv[2] <<std::endl;
		}
	}
	std::fstream inputFile;
	inputFile.open(argv[1], std::fstream::in);
	if(!inputFile.is_open()){
		std::cerr << "Error: File does not exist." << std::endl;
		exit(1);
	}
	if(Parser::parse(inputFile, errorMsgsEnabled)){
		std::cout << "ACCEPT" << std::endl;
	}
	else{
		std::cout << "REJECT" << std::endl;
	}
}
