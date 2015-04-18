#include <ostream>
#include <stack>
#include <string>
#include <sstream>
#include <iomanip>
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
    	typedef std::vector<SyntaxAndPointer> SyntaxAndPointerStack;

    	static const int bufSize = 32; //for converting integers into strings

    	void formattedOutput(std::stringstream input, std::stringstream output, int column);
    	std::string operandPrefix(SyntaxAndPointer& operand);
    	void handleExpression(int& varCounter, int& compCounter, SyntaxAndPointerStack& writeLater, std::ostream& stream);
    	void printBinaryOperation(std::string op, int& varCounter, SyntaxAndPointerStack& writeLater, std::ostream& stream);
    	void printCompOperation(std::string op, bool notEq, bool alsoEQ, int& varCounter, int &compCounter,
    			SyntaxAndPointerStack& writeLater, std::ostream& stream);

    	void formattedOutput(std::stringstream input, std::stringstream output, int column)
    	{
    		static const int width = 20;

    		if(output.peek() == EOF && column > 1){
    			output.clear();
    			output << std::setw(width) << std::left << "" << std::setw(width) << std::left << input;
    		}
    		else if((output.peek() == '\n' || output.peek() == '\r') && column > 1){
    			output << std::setw(width) << std::left << "" << std::setw(width) << std::left << input;
    		}
    		else{
    			output << std::setw(width) << std::left << input;
    		}
    	}

        std::string operandPrefix(SyntaxAndPointer& operand)
        {
        	return (operand.first.syntaxFlag == SyntaxInfo::VAR ? (operand.second ? "@" : "") : "#");
        }

        void handleExpression(int& varCounter, int& compCounter, SyntaxAndPointerStack& writeLater, std::ostream& stream)
        {
        	//while the top two items on the stack are operands
        	while(writeLater.size() > 2 &&
        			(writeLater.back().first.syntaxFlag == SyntaxInfo::INT_LITERAL ||
        			writeLater.back().first.syntaxFlag == SyntaxInfo::FLOAT_LITERAL ||
					writeLater.back().first.syntaxFlag == SyntaxInfo::VAR) &&
        			(writeLater[writeLater.size()-2].first.syntaxFlag == SyntaxInfo::INT_LITERAL ||
        			writeLater[writeLater.size()-2].first.syntaxFlag == SyntaxInfo::FLOAT_LITERAL ||
					writeLater[writeLater.size()-2].first.syntaxFlag == SyntaxInfo::VAR)){
        		switch(writeLater[writeLater.size()-3].first.syntaxFlag)
        		{
        		case SyntaxInfo::ASSIGNMENT:
        		{
        			SyntaxAndPointer lhs;
                	stream << "\t\t+LDA\t\t" << operandPrefix(writeLater.back()) << writeLater.back().first.name << std::endl;
                	writeLater.pop_back();
                	lhs = writeLater.back();
                	stream << "\t\t+STA\t\t" << operandPrefix(writeLater.back()) << writeLater.back().first.name << std::endl;
                	writeLater.pop_back();
                	writeLater.pop_back();
                	writeLater.push_back(lhs);
        			break;
        		}
                case SyntaxInfo::ADD:
                	printBinaryOperation("\t\t+ADD\t\t", varCounter, writeLater, stream);
                	break;
                case SyntaxInfo::SUB:
                	printBinaryOperation("\t\t+SUB\t\t", varCounter, writeLater, stream);
                	break;
                case SyntaxInfo::MULT:
                	printBinaryOperation("\t\t+MUL\t\t", varCounter, writeLater, stream);
                	break;
                case SyntaxInfo::DIV:
                	printBinaryOperation("\t\t+DIV\t\t", varCounter, writeLater, stream);
                	break;
                case SyntaxInfo::EQ:
                	printCompOperation("\t\t+JEQ\t\t", false, false, varCounter, compCounter, writeLater, stream);
                	break;
                case SyntaxInfo::NEQ:
                	printCompOperation("\t\t+JEQ\t\t", true, false, varCounter, compCounter, writeLater, stream);
                	break;
                case SyntaxInfo::GT:
                	printCompOperation("\t\t+JGT\t\t", false, false, varCounter, compCounter, writeLater, stream);
                	break;
                case SyntaxInfo::GTEQ:
                	printCompOperation("\t\t+JEQ\t\t", false, true, varCounter, compCounter, writeLater, stream);
                	break;
                case SyntaxInfo::LT:
                	printCompOperation("\t\t+JLT\t\t", false, false, varCounter, compCounter, writeLater, stream);
                	break;
                case SyntaxInfo::LTEQ:
                	printCompOperation("\t\t+JEQ\t\t", false, true, varCounter, compCounter, writeLater, stream);
                	break;
        		}
        	}
        }

        void printBinaryOperation(std::string op, int& varCounter, SyntaxAndPointerStack& writeLater, std::ostream& stream)
        {
        	std::string rhs = writeLater.back().first.name;
        	std::string rhsPrefix = operandPrefix(writeLater.back());
        	writeLater.pop_back();
			stream << "\t\t+LDA\t\t" << operandPrefix(writeLater.back()) << writeLater.back().first.name << std::endl;
			stream << op << rhsPrefix << rhs << std::endl;
			writeLater.pop_back();
			writeLater.pop_back(); //pop operator
			stream << "\t\t+STA\t\t__t" << varCounter << std::endl;
			char tempVarNum[bufSize];
			snprintf(tempVarNum, bufSize, "%d", varCounter++);
			writeLater.push_back(SyntaxAndPointer(SyntaxInfo(SyntaxInfo::VAR, -1, std::string("__t") + std::string(tempVarNum)), false));
        }

        void printCompOperation(std::string op, bool notEQ, bool alsoEQ, int& varCounter, int &compCounter,
        		SyntaxAndPointerStack& writeLater, std::ostream& stream)
        {
        	std::string rhs = writeLater.back().first.name;
        	std::string rhsPrefix = operandPrefix(writeLater.back());
        	writeLater.pop_back();
			stream << "\t\t+LDA\t\t" << operandPrefix(writeLater.back()) << writeLater.back().first.name << std::endl;
			stream << "\t\t+COMP\t\t" << rhsPrefix << rhs << std::endl;
			writeLater.pop_back();
			writeLater.pop_back(); //pop operator
			stream << op << "__TRUE" << compCounter << std::endl;
			if(alsoEQ){
				stream << "\t\t+JEQ\t\t__TRUE" << compCounter << std::endl;
			}
			stream << "\t\t+J\t\t__FALSE" << compCounter << std::endl;
			stream << "__TRUE" << compCounter << "\t\t+LDA\t\t#" << (notEQ ? "0" : "1") << std::endl;
			stream << "\t\t+J\t\t" << "__CONT" << compCounter << std::endl;
			stream << "__FALSE" << compCounter << "\t\t+LDA\t\t#" << (notEQ ? "1" : "0") << std::endl;
			stream << "__CONT" << compCounter++ << "\t\t+STA\t\t__t" << varCounter << std::endl;
			char tempVarNum[bufSize];
			snprintf(tempVarNum, bufSize, "%d", varCounter++);
			writeLater.push_back(SyntaxAndPointer(SyntaxInfo(SyntaxInfo::VAR, -1, std::string("__t") + std::string(tempVarNum)), false));
        }
    }

    void generateSICXE(Tree<SyntaxInfo>* syntaxTree, std::ostream& stream)
    {
    	//bool designates whether or not indirect mode is needed, i.e. @
        std::stack<SyntaxAndPointerStack> writeLater;
        int ifCounter = 0;
        int whileCounter = 0;
        int varCounter = 0;
        int compCounter = 0;
        std::stack<int> writeIfLabelLater;
        std::stack<int> writeWhileLableLater;
        std::stack<int> writeVarLabelLater;
        std::stack<SyntaxInfo> opStack;

        writeLater.push(SyntaxAndPointerStack());
        for(Tree<SyntaxInfo>::preorder_iterator it = syntaxTree->preorder_begin(); it != syntaxTree->preorder_end(); ++it){
            switch(it->syntaxFlag)
            {
            case SyntaxInfo::PROGRAM:
                stream << "\t\tSTART\t\t100\n\t\tBLOC" << std::endl;
                break;
            case SyntaxInfo::VAR_DEC:
                /*stream << it->name << "\t\t";
                if(it->typeFlag == SyntaxInfo::INT){
                    stream << "\t\tWORD" << std::endl;
                }
                else if(it->typeFlag == SyntaxInfo::INT_ARRAY){
                    stream << "\t\tRESW\t\t"; //next iteration will say how large the array is
                    waitingOnArraySize = true;
                    arrayDecIsInt = true;
                }
                else if(it->typeFlag == SyntaxInfo::FLOAT){
                    stream << "\t\tRESW\t\t2" << std::endl; //floats are twice the size of ints
                }
                else if(it->typeFlag == SyntaxInfo::FLOAT_ARRAY){
                    stream << "\t\tRESW\t\t"; //next iteration will say how large the array is
                    waitingOnArraySize = true;
                    arrayDecIsInt = false;
                }*/
                break;
            case SyntaxInfo::ARRAY_DEC_SIZE:
                /*if(arrayDecIsInt){
                    stream << it->name << std::endl;
                }
                else{
                	char buffer[32];
                	snprintf(buffer, 32, "%d", atoi(it->name.c_str())*2); //I miss C++11 :(
                    stream << buffer << std::endl;
                }*/
                break;
            case SyntaxInfo::FUN_DEC:
                writeLater.top().push_back(std::pair<SyntaxInfo, bool>(*it, false)); //gotta write parameters first
                break;
            case SyntaxInfo::PARAMS:
                break;
            case SyntaxInfo::EXIT_PARAMS:
                stream << writeLater.top().back().first.name << "\t\tFDCL" << std::endl; //seen all parameters now we can write function
                writeLater.top().pop_back();
                break;
            case SyntaxInfo::PARAM_DEC:
            	stream << "\t\tFPRM\t\t";
            	if(it->typeFlag == SyntaxInfo::INT || it->typeFlag == SyntaxInfo::INT_ARRAY || it->typeFlag == SyntaxInfo::FLOAT_ARRAY){
            		stream << "1" << std::endl; //ints are one word big and arrays are pass by reference and addresses are one word big
            	}
            	else{
            		stream << "2" << std::endl; //floats are two words big
            	}
            	break;
            case SyntaxInfo::COMPOUND_STMT:
                stream << "\t\tBLOC" << std::endl;
            	break;
            case SyntaxInfo::EXIT_COMPOUND_STMT:
                stream << "\t\tEND\t\tBLOC" << std::endl;
                break;
            case SyntaxInfo::LOCAL_DECS:
            		/*stream << it->name << "\t\t";
            		if(it->typeFlag == SyntaxInfo::INT){
            			stream << "WORD" << std::endl;
            		}
            		else{
            			stream << "RESW\t\t";
            			if(it->typeFlag == SyntaxInfo::FLOAT){
            				stream << "2" << std::endl;
            			}
            		}*/
            	break;
            case SyntaxInfo::STMT_LIST:
            	break;
            case SyntaxInfo::EXIT_EXPR_STMT:
            	writeLater.top().pop_back();
                break;
            case SyntaxInfo::ASSIGNMENT:
            	writeLater.top().push_back(SyntaxAndPointer(*it, false));
                break;
            case SyntaxInfo::EXIT_ASSIGNMENT:
                break;
            case SyntaxInfo::IF:
                break;
            case SyntaxInfo::BEGIN_IF_STMT:
                stream << "\t\t+LDA\t\t" << writeLater.top().back().first.name << std::endl;
                writeLater.top().pop_back();
                stream << "\t\t+COMP\t\t#0" << std::endl;
                stream << "\t\t+JEQ\t\t" << "__ELSE" << ifCounter << std::endl;
                writeIfLabelLater.push(ifCounter++);
                break;
            case SyntaxInfo::BEGIN_ELSE_STMT:
                stream << "__ELSE" << writeIfLabelLater.top();
                writeIfLabelLater.pop();
                break;
            case SyntaxInfo::WHILE:
                stream << "__LOOP" << whileCounter;
                writeWhileLableLater.push(whileCounter++);
                break;
            case SyntaxInfo::BEGIN_WHILE_STMT:
                stream << "\t\t+LDA\t\t" << operandPrefix(writeLater.top().back()) << writeLater.top().back().first.name << std::endl;
                stream << "\t\t+COMP\t\t#0" << std::endl;
                stream << "\t\t+JEQ\t\t__NWHILE" << writeWhileLableLater.top() << std::endl;
                writeLater.top().pop_back();
                break;
            case SyntaxInfo::EXIT_WHILE_STMT:
                stream << "\t\t+J\t\t_LOOP" << writeWhileLableLater.top() << std::endl;
                stream << "_NWHILE" << writeWhileLableLater.top();
                writeWhileLableLater.pop();
                break;
            case SyntaxInfo::RETURN:
                break;
            case SyntaxInfo::EXIT_RETURN_STMT:
                stream << "\t\tFRET\t\t" << operandPrefix(writeLater.top().back()) << writeLater.top().back().first.name << std::endl;
                writeLater.top().pop_back();
                break;
            case SyntaxInfo::INT_LITERAL:
            case SyntaxInfo::FLOAT_LITERAL:
            case SyntaxInfo::VAR:
            	writeLater.top().push_back(SyntaxAndPointer(*it, false));
            	//if the top two items on the stack are operands
            	if(writeLater.top().size() > 2 &&
            			(writeLater.top().back().first.syntaxFlag == SyntaxInfo::INT_LITERAL ||
            			writeLater.top().back().first.syntaxFlag == SyntaxInfo::FLOAT_LITERAL ||
						writeLater.top().back().first.syntaxFlag == SyntaxInfo::VAR) &&
            			(writeLater.top()[writeLater.top().size()-2].first.syntaxFlag == SyntaxInfo::INT_LITERAL ||
            			writeLater.top()[writeLater.top().size()-2].first.syntaxFlag == SyntaxInfo::FLOAT_LITERAL ||
						writeLater.top()[writeLater.top().size()-2].first.syntaxFlag == SyntaxInfo::VAR)){


            		handleExpression(varCounter, compCounter, writeLater.top(), stream);
            	}
                break;
            case SyntaxInfo::INDEX:
            	writeLater.push(SyntaxAndPointerStack());
            	break;
            case SyntaxInfo::EXIT_INDEX:
                stream << "\t\t+LDX\t\t" << operandPrefix(writeLater.top().back()) << writeLater.top().back().first.name << std::endl;
                writeLater.top().pop_back();
                writeLater.pop();
                stream << "\t\t+LDA\t\t" << operandPrefix(writeLater.top().back()) << writeLater.top().back().first.name << ",X" << std::endl;
                writeLater.top().pop_back();
                writeVarLabelLater.push(varCounter);
                {
                    SyntaxInfo temp;
                    temp.syntaxFlag = SyntaxInfo::VAR;
                    std::stringstream I_Miss_CPP_11;
                    I_Miss_CPP_11 << "__t" << varCounter;
                    temp.name = I_Miss_CPP_11.str();
                    writeLater.top().push_back(SyntaxAndPointer(temp, false));
                }
                stream << "\t\t+STA\t\t" << "__t" << varCounter++ << std::endl;
                break;
            case SyntaxInfo::CALL:
            	writeLater.top().push_back(SyntaxAndPointer(*it, false)); //need to see args before calling
            	break;
            case SyntaxInfo::EXIT_CALL:
            	stream << "\t\tFCLL\t\t" << writeLater.top().back().first.name << std::endl;
            	break;
            case SyntaxInfo::ARG:
            	writeLater.push(SyntaxAndPointerStack());
            	break;
            case SyntaxInfo::EXIT_ARG:
            	stream << "\t\tFARG\t\t" << operandPrefix(writeLater.top().back()) << writeLater.top().back().first.name << std::endl;
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
            	writeLater.top().push_back(SyntaxAndPointer(*it, false));
            	break;
            default:
            	stream << "ERROR";
            	break;
            }
		}
    }
}
