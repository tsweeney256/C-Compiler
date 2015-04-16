#include <ostream>
#include <stack>
#include <string>
#include <cstdlib>
#include <cstdio>
#include "generateSICXE.hpp"
#include "Tree.hpp"
#include "SyntaxInfo.hpp"

namespace IntermediateCode
{
    namespace //private to this translation unit
    {

    }

    void generateSICXE(Tree<SyntaxInfo>* syntaxTree, std::ostream& stream)
    {
        std::stack<SyntaxInfo*> writeLater;
        bool waitingOnArraySize = false;
        bool arrayDecIsInt;

        for(Tree<SyntaxInfo>::preorder_iterator it = syntaxTree->preorder_begin(); it != syntaxTree->preorder_end(); ++it){
            switch(it->syntaxFlag)
            {
            case SyntaxInfo::PROGRAM:
                stream << "\tSTART\t100\n\tBLOC" << std::endl;
                break;
            }
            case SyntaxInfo::VAR_DEC:
                stream << it->name << "\t";
                if(it->typeFlag == SyntaxInfo::INT){
                    stream << "\tWORD" << std::endl;
                }
                else if(it->typeFlag == SyntaxInfo::INT_ARRAY){
                    stream << "\tRESW\t"; //next iteration will say how large the array is
                    waitingOnArraySize = true;
                    arrayDecIsInt = true;
                }
                else if(it->typeFlag == SyntaxInfo::FLOAT){
                    stream << "\tRESW\t2" << std::endl; //floats are twice the size of ints
                }
                else if(it->typeFlag == SyntaxInfo::FLOAT_ARRAY){
                    stream << "\tRESW\t"; //next iteration will say how large the array is
                    waitingOnArraySize = true;
                    arrayDecIsInt = false;
                }
                break;
            case SyntaxInfo::ARRAY_DEC_SIZE:
                if(arrayDecIsInt){
                    stream << it->name << std::endl;
                }
                else{
                	char buffer[32];
                	snprintf(buffer, 32, "%d", atoi(it->name.c_str())*2); //I miss C++11 :(
                    stream << buffer << std::endl;
                }
                break;
            case SyntaxInfo::FUN_DEC:
                writeLater.push(&(*it)); //gotta write parameters first. This gets written upon seeing COMPOUND_STMT
                break;
            case SyntaxInfo::PARAMS:
                break;
            case SyntaxInfo::PARAM_DEC:
            	stream << "\tFPRM\t";
            	if(it->typeFlag == SyntaxInfo::INT || it->typeFlag == SyntaxInfo::INT_ARRAY || it->typeFlag == SyntaxInfo::FLOAT_ARRAY){
            		stream << "1" << std::endl; //ints are one word big and arrays are pass by reference and addresses are one word big
            	}
            	else{
            		stream << "2" << std::endl; //floats are two words big
            	}
            	break;
            case SyntaxInfo::COMPOUND_STMT:
            		stream << writeLater.top()->name << "\tFDCL" << std::endl; //seen all parameters now we can write function
            		writeLater.pop();
            	break;
            case SyntaxInfo::LOCAL_DECS:
            		stream << it->name << "\t";
            		if(it->typeFlag == SyntaxInfo::INT){
            			stream << "WORD" << std::endl;
            		}
            		else{
            			stream << "RESW\t";
            			if(it->typeFlag == SyntaxInfo::FLOAT){
            				stream << "2" << std::endl;
            			}
            		}
            	break;
            case SyntaxInfo::STMT_LIST:
            	break;

            default:
            	stream << "ERROR";
            	break;
		}
    }
}
