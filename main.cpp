#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <list>
#include "Parser.hpp"

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

	std::cout << Parser::parse(inputFile) << std::endl;


}
