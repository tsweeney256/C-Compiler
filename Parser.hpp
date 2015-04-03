#ifndef PARSER_HPP
#define PARSER_HPP

#include <istream>
#include "SyntaxInfo.hpp"

template<typename T>
class Tree;



namespace Parser
{
    Tree<SyntaxInfo>* parse(std::istream& input);
}
#endif // PARSER_HPP
