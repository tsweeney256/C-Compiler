#ifndef GENERATESICXE
#define GENERATESICXE

#include <ostream>
class SyntaxInfo;
template <typename T> class Tree;

namespace IntermediateCode
{
    void generateSICXE(Tree<SyntaxInfo>* syntaxTree, std::ostream& stream);
}

#endif // GENERATESICXE
