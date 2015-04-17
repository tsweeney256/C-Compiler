#include <ostream>
#include <stack>
#include <queue>
#include <string>
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include "generateSICXE.hpp"
#include "Tree.hpp"
#include "SyntaxInfo.hpp"

namespace IntermediateCode
{
    namespace //private to this translation unit
    {
    	typedef std::pair<SyntaxInfo, bool> SyntaxAndPointer;
    	typedef std::queue<SyntaxAndPointer> SyntaxAndPointerQueue;

    	bool higherPrec(int newOp, int oldOp);
        void handleExpr(SyntaxAndPointerQueue& writeLater);

        bool higherPrec(int newOp, int oldOp)
        {
        	if(oldOp == SyntaxInfo::MULT || oldOp == SyntaxInfo::DIV){
        		return false;
        	}
        	else if(oldOp == SyntaxInfo::ADD || oldOp == SyntaxInfo::SUB){
        		if(newOp == SyntaxInfo::MULT || newOp == SyntaxInfo::DIV){
        			return true;
        		}
        		else{
        			return false;
        		}
        	}
        	else if(oldOp == SyntaxInfo::GT || oldOp == SyntaxInfo::GTEQ || oldOp == SyntaxInfo::LT || oldOp == SyntaxInfo::LTEQ ||
        			oldOp == SyntaxInfo::EQ || oldOp == SyntaxInfo::NEQ){
        		return true;
        	}
        	else{
        		return true; //assignment
        	}
        }

        void handleExpr(SyntaxAndPointerQueue& writeLater)
        {
        	int lastSawOperator = 0;
        	int operandsInARow = 0;
        	while(!writeLater.empty()){
        		if(higherPrec(writeLater.front().first.syntaxFlag, lastSawOperator)){

        		}
        	}
        }
    }

    void generateSICXE(Tree<SyntaxInfo>* syntaxTree, std::ostream& stream)
    {
    	//bool designates whether or not indirect mode is needed, i.e. @
        std::stack<SyntaxAndPointerQueue> writeLater;
        int lastSyntaxFlag = -1;
        int ifCounter = 0;
        int whileCounter = 0;
        int varCounter = 0;
        std::stack<int> writeIfLabelLater;
        std::stack<int> writeWhileLableLater;
        std::stack<int> writeVarLabelLater;
        bool waitingOnArraySize = false;
        bool arrayDecIsInt;

        for(Tree<SyntaxInfo>::preorder_iterator it = syntaxTree->preorder_begin(); it != syntaxTree->preorder_end(); ++it){
        	writeLater.push(SyntaxAndPointerQueue());
            switch(it->syntaxFlag)
            {
            case SyntaxInfo::PROGRAM:
                stream << "\tSTART\t100\n\tBLOC" << std::endl;
                break;
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
                writeLater.top().push(std::pair<SyntaxInfo, bool>(*it, false)); //gotta write parameters first. This gets written upon seeing COMPOUND_STMT
                break;
            case SyntaxInfo::PARAMS:
                break;
            case SyntaxInfo::EXIT_PARAMS:
                stream << writeLater.top().front().first.name << "\tFDCL" << std::endl; //seen all parameters now we can write function
                writeLater.pop();
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
                stream << "\tBLOC" << std::endl;
            	break;
            case SyntaxInfo::EXIT_COMPOUND_STMT:
                stream << "\tEND\tBLOC" << std::endl;
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
            case SyntaxInfo::EXIT_EXPR_STMT:
                //TODO:
                break;
            case SyntaxInfo::ASSIGNMENT:
            	writeLater.top().push(SyntaxAndPointer(*it, false));
                break;
            case SyntaxInfo::EXIT_ASSIGNMENT:
                stream << "\t+LDA\t" <<
						((writeLater.top().front().first.syntaxFlag == SyntaxInfo::VAR) ? ("") : ("#")) <<
						writeLater.top().front().first.name << std::endl;
                writeLater.pop();
                stream << "\t+STA\t" << (writeLater.top().front().second ? "" : "@") << writeLater.top().front().first.name << std::endl;
                writeLater.pop();
                break;
            case SyntaxInfo::IF:
                break;
            case SyntaxInfo::BEGIN_IF_STMT:
            	handleExpr(writeLater.top()); //the if statement predicate
                stream << "\t+LDA\t" << (writeLater.top().front().second ? "" : "@") << writeLater.top().front().first.name << std::endl;
                stream << "\t+COMP\t#0" << std::endl;
                stream << "\t+JEQ\t" << "_ELSE" << ifCounter << std::endl;
                stream << "\t+J\t" << "_IF" << ifCounter << std::endl;
                writeIfLabelLater.push(ifCounter++);
                stream << "_IF" << writeIfLabelLater.top();
                break;
            case SyntaxInfo::BEGIN_ELSE_STMT:
                stream << "_ELSE" << writeIfLabelLater.top();
                writeIfLabelLater.pop();
                break;
            case SyntaxInfo::WHILE:
                stream << "_LOOP" << whileCounter;
                writeWhileLableLater.push(whileCounter++);
                break;
            case SyntaxInfo::BEGIN_WHILE_STMT:
            	handleExpr(writeLater.top()); //the while statement predicate
                stream << "\t+LDA\t" << (writeLater.top().front().second ? "" : "@") << writeLater.top().front().first.name << std::endl;
                stream << "\t+COMP\t#0" << std::endl;
                stream << "\t+JEQ\t_WHILE" << writeWhileLableLater.top() << std::endl;
                writeLater.pop();
                break;
            case SyntaxInfo::EXIT_WHILE_STMT:
                stream << "\t+J\t_LOOP" << writeWhileLableLater.top() << std::endl;
                stream << "_NWHILE" << writeWhileLableLater.top();
                writeWhileLableLater.pop();
                break;
            case SyntaxInfo::RETURN:
                break;
            case SyntaxInfo::EXIT_RETURN_STMT:
            	handleExpr(writeLater.top());
                stream << "\tFRET\t" << (writeLater.top().front().second ? "" : "@") << writeLater.top().front().first.name << std::endl;
                writeLater.pop();
                break;
            case SyntaxInfo::INT_LITERAL:
            case SyntaxInfo::FLOAT_LITERAL:
            case SyntaxInfo::VAR:
                writeLater.top().push(SyntaxAndPointer(*it, false));
                break;
            case SyntaxInfo::INDEX:
            	writeLater.push(SyntaxAndPointerQueue());
            	break;
            case SyntaxInfo::EXIT_INDEX:
                handleExpr(writeLater.top());
                stream << "\t+LDX\t" << (writeLater.top().front().second ? "" : "@") << writeLater.top().front().first.name << std::endl;
                writeLater.pop();
                stream << "\t+LDA\t" << (writeLater.top().front().second ? "" : "@") << writeLater.top().front().first.name << ",X" << std::endl;
                writeLater.pop();
                writeVarLabelLater.push(varCounter);
                {
                    SyntaxInfo temp;
                    temp.syntaxFlag = SyntaxInfo::VAR;
                    std::stringstream I_Miss_CPP_11;
                    I_Miss_CPP_11 << "_t" << varCounter;
                    temp.name = I_Miss_CPP_11.str();
                    writeLater.top().push(SyntaxAndPointer(temp, false));
                }
                stream << "\t+STA\t" << "_t" << varCounter++ << std::endl;
                writeLater.pop();
                break;
            case SyntaxInfo::CALL:
            	writeLater.top().push(SyntaxAndPointer(*it, false)); //need to see args before calling
            	break;
            case SyntaxInfo::EXIT_CALL:
            	stream << "\tFCLL\t" << writeLater.top().front().first.name << std::endl;
            	break;
            case SyntaxInfo::ARG:
            	writeLater.push(SyntaxAndPointerQueue());
            	break;
            case SyntaxInfo::EXIT_ARG:
            	handleExpr(writeLater.top());
            	stream << "\tFARG\t" << (writeLater.top().front().second ? "" : "@") << writeLater.top().front().first.name << std::endl;
            	writeLater.pop();
            	break;
            case SyntaxInfo::ADD:
            case SyntaxInfo::SUB:
            case SyntaxInfo::MULT:
            case SyntaxInfo::DIV:
            case SyntaxInfo::EQ:
            case SyntaxInfo::NEQ:
            case SyntaxInfo::GT:
            case SyntaxInfo::GTEQ:
            case SyntaxInfo::LT:
            case SyntaxInfo::LTEQ:
            	writeLater.top().push(SyntaxAndPointer(*it, false));
            	break;
            default:
            	stream << "ERROR";
            	break;
            }
		}
    }
}
