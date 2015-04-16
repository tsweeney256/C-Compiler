#ifndef GENERATESICXE
#define GENERATESICXE

#include <ostream>
class Tree<SyntaxInfo>;

namespace IntermediateCode
{
    void generateSICXE(Tree<SyntaxInfo>* syntaxTree, std::ostream& stream);
}

#endif // GENERATESICXE
