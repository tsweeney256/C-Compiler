#include <ostream>
#include <stack>
#include <string>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <cstdio>
#include <algorithm>
#include "generateSICXE.hpp"
#include "Tree.hpp"
#include "SyntaxInfo.hpp"

namespace IntermediateCode
{
    namespace //private to this translation unit
    {
    	typedef std::pair<SyntaxInfo, bool> SyntaxAndPointer;
    	typedef std::vector<SyntaxAndPointer> SyntaxAndPointerStack;
    	typedef std::pair<SyntaxInfo, std::string> VarAndSize;
    	typedef std::stack<VarAndSize> VarList;

    	static const int bufSize = 32; //for converting integers into strings

    	void formattedOutput(const std::string& input, std::stringstream& output, int column);
    	std::string operandPrefix(const SyntaxAndPointer& operand);
    	void handleExpression(int& varCounter, int& compCounter, SyntaxAndPointerStack& writeLater, std::stringstream& stream);
    	void printBinaryOperation(std::string op, int& varCounter, SyntaxAndPointerStack& writeLater, std::stringstream& output);
    	void printCompOperation(std::string op, bool notEq, bool alsoEQ, int& varCounter, int &compCounter,
    			SyntaxAndPointerStack& writeLater, std::stringstream& output);

    	void formattedOutput(const std::string& input, std::stringstream& output, int column)
    	{
    		//this function is dumb and I hate it

    		static const int width = 20;

    		bool needsToBeANewLine = false;
    		static bool wasNewLineLastTime = false; //well this just killed thread safety
    		std::string inputCopy = input;

    		if(input[input.size()-1] == '\n'){
    			needsToBeANewLine = true;
    			inputCopy.erase(--inputCopy.end());
    		}
    		else{
    			needsToBeANewLine = false;
    		}

    		if(output.peek() == EOF && column > 1){
				output.clear();
				output << std::setw(width) << std::left << " " << std::setw(width) << std::left << inputCopy;
			}
    		else if(wasNewLineLastTime && column > 1){
    			output << std::setw(width) << std::left << " " << std::setw(width) << std::left << inputCopy;
    		}
    		else{
    			output << std::setw(width) << std::left << inputCopy;
    		}

    		if(needsToBeANewLine){
    			output << std::endl;
    			wasNewLineLastTime = true;
    		}
    		else{
    			wasNewLineLastTime = false;
    		}
    	}

        std::string operandPrefix(const SyntaxAndPointer& operand)
        {
        	return (operand.first.syntaxFlag == SyntaxInfo::VAR ? (operand.second ? "@" : "") : "#");
        }

        void handleExpression(int& varCounter, int& compCounter, SyntaxAndPointerStack& writeLater, std::stringstream& output)
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
        			formattedOutput("+LDA", output, 2);
        			formattedOutput(operandPrefix(writeLater.back()) + writeLater.back().first.name + "\n", output, 3);
                	writeLater.pop_back();
                	lhs = writeLater.back();
                	formattedOutput("+STA", output, 2);
                	formattedOutput(operandPrefix(writeLater.back()) + writeLater.back().first.name + "\n", output, 3);
                	writeLater.pop_back();
                	writeLater.pop_back();
                	writeLater.push_back(lhs);
        			break;
        		}
                case SyntaxInfo::ADD:
                	printBinaryOperation("+ADD", varCounter, writeLater, output);
                	break;
                case SyntaxInfo::SUB:
                	printBinaryOperation("+SUB", varCounter, writeLater, output);
                	break;
                case SyntaxInfo::MULT:
                	printBinaryOperation("+MUL", varCounter, writeLater, output);
                	break;
                case SyntaxInfo::DIV:
                	printBinaryOperation("+DIV", varCounter, writeLater, output);
                	break;
                case SyntaxInfo::EQ:
                	printCompOperation("+JEQ", false, false, varCounter, compCounter, writeLater, output);
                	break;
                case SyntaxInfo::NEQ:
                	printCompOperation("+JEQ", true, false, varCounter, compCounter, writeLater, output);
                	break;
                case SyntaxInfo::GT:
                	printCompOperation("+JGT", false, false, varCounter, compCounter, writeLater, output);
                	break;
                case SyntaxInfo::GTEQ:
                	printCompOperation("+JGT", false, true, varCounter, compCounter, writeLater, output);
                	break;
                case SyntaxInfo::LT:
                	printCompOperation("+JLT", false, false, varCounter, compCounter, writeLater, output);
                	break;
                case SyntaxInfo::LTEQ:
                	printCompOperation("+JLT", false, true, varCounter, compCounter, writeLater, output);
                	break;
        		}
        	}
        }

        void printBinaryOperation(std::string op, int& varCounter, SyntaxAndPointerStack& writeLater, std::stringstream& output)
        {
        	std::string rhs = writeLater.back().first.name;
        	std::string rhsPrefix = operandPrefix(writeLater.back());
        	writeLater.pop_back();
        	formattedOutput("+LDA", output, 2);
        	formattedOutput(operandPrefix(writeLater.back()) + writeLater.back().first.name + "\n", output, 3);
        	formattedOutput(op, output, 2);
        	formattedOutput(rhsPrefix + rhs + "\n", output, 3);
			writeLater.pop_back();
			writeLater.pop_back(); //pop operator
			formattedOutput("+STA", output, 2);
			{
				std::stringstream temp;
				temp << "__t" << varCounter << "\n";
				formattedOutput(temp.str(), output, 3);
			}
			char tempVarNum[bufSize];
			snprintf(tempVarNum, bufSize, "%d", varCounter++);
			writeLater.push_back(SyntaxAndPointer(SyntaxInfo(SyntaxInfo::VAR, -1, std::string("__t") + std::string(tempVarNum)), false));
        }

        void printCompOperation(std::string op, bool notEQ, bool alsoEQ, int& varCounter, int &compCounter,
        		SyntaxAndPointerStack& writeLater, std::stringstream& output)
        {
        	std::string rhs = writeLater.back().first.name;
        	std::string rhsPrefix = operandPrefix(writeLater.back());
        	writeLater.pop_back();

        	formattedOutput("+LDA", output, 2);
        	formattedOutput(operandPrefix(writeLater.back()) + writeLater.back().first.name + "\n", output, 3);

        	formattedOutput("+COMP", output, 2);
        	formattedOutput(rhsPrefix + rhs + "\n", output, 3);

			writeLater.pop_back();
			writeLater.pop_back(); //pop operator
			formattedOutput(op, output, 2);

			std::stringstream temp;
			temp << compCounter++;
			std::string compCounterStr(temp.str());

			formattedOutput("__TRUE" + compCounterStr + "\n", output, 3);
			if(alsoEQ){
				formattedOutput("+JEQ", output, 2);
				formattedOutput("__TRUE" + compCounterStr + "\n", output, 3);
			}
			formattedOutput("+J", output, 2);
			formattedOutput("__FALSE" + compCounterStr + "\n", output, 3);

			formattedOutput("__TRUE" + compCounterStr, output, 1);
			formattedOutput("+LDA#", output, 2);
			formattedOutput((notEQ ? "#0" : "#1") + std::string("\n"), output, 3);

			formattedOutput("+J", output, 2);
			formattedOutput("__CONT" + compCounterStr + "\n", output, 3);

			formattedOutput("__FALSE" + compCounterStr, output, 1);
			formattedOutput("+LDA", output, 2);
			formattedOutput((notEQ ? "#1" : "#0") + std::string("\n"), output, 3);

			formattedOutput("__CONT" + compCounterStr, output, 1);
			formattedOutput("+STA", output, 2);

			char tempVarNum[bufSize];
			snprintf(tempVarNum, bufSize, "%d", varCounter++);
			writeLater.push_back(SyntaxAndPointer(SyntaxInfo(SyntaxInfo::VAR, -1, std::string("__t") + std::string(tempVarNum)), false));
			formattedOutput(std::string("__t") + tempVarNum + "\n", output, 3);
        }
    }

    void generateSICXE(Tree<SyntaxInfo>* syntaxTree, std::ostream& finalOutput)
    {
    	std::stringstream output;
    	//bool designates whether or not indirect mode is needed, i.e. @
        std::stack<SyntaxAndPointerStack> writeLater;
        int ifCounter = 0;
        int whileCounter = 0;
        int varCounter = 0;
        int compCounter = 0;
        std::stack<int> writeIfLabelLater;
        std::stack<int> writeWhileLableLater;
        std::stack<int> writeVarLabelLater;
        int writeBlockJumpLater = 0;
        std::stack<SyntaxInfo> opStack;
        std::stack<std::vector<std::string> > pointerVars; //solely for array parameters which are pass by reference
        std::stack<VarList> vars;
        bool isFuncVoid = false;
        std::stack<bool> isFunc;
        int depth = 0;

        writeLater.push(SyntaxAndPointerStack());
        for(Tree<SyntaxInfo>::preorder_iterator it = syntaxTree->preorder_begin(); it != syntaxTree->preorder_end(); ++it){
            switch(it->syntaxFlag)
            {
            case SyntaxInfo::PROGRAM:
            	vars.push(VarList());
            	isFunc.push(false);
                formattedOutput("START", output, 2);
                formattedOutput("100\n", output, 3);
                formattedOutput("FCLL", output, 2);
                formattedOutput("_main\n", output, 1);
                formattedOutput("+J", output, 2);
                formattedOutput("__end\n", output, 3);
                break;
            case SyntaxInfo::EXIT_PROGRAM:
            	formattedOutput("__end", output, 1);
            	formattedOutput("RSUB\n", output, 2);
            	formattedOutput("END\n", output, 2);
            	//write global data after END to avoid executing data
            	while(!vars.top().empty()){
            		formattedOutput(vars.top().top().first.name, output, 1);
            		if(vars.top().top().first.typeFlag == SyntaxInfo::INT){
            			formattedOutput("WORD\n", output, 2);
            		}
            		else if(vars.top().top().first.typeFlag == SyntaxInfo::FLOAT){
            			formattedOutput("RESW", output, 2);
            			formattedOutput("2\n", output, 3);
            		}
            		else{
            			formattedOutput("RESW", output, 2);
            			formattedOutput(vars.top().top().second + "\n", output, 3);
            		}
            		vars.top().pop();
            	}
            	break;
            case SyntaxInfo::VAR_DEC:
            	vars.top().push(VarAndSize(*it, ""));

                /*finalOutput << it->name << "";
                if(it->typeFlag == SyntaxInfo::INT){
                    finalOutput << "WORD" << std::endl;
                }
                else if(it->typeFlag == SyntaxInfo::INT_ARRAY){
                    stream << "RESW"; //next iteration will say how large the array is
                    waitingOnArraySize = true;
                    arrayDecIsInt = true;
                }
                else if(it->typeFlag == SyntaxInfo::FLOAT){
                    stream << "RESW2" << std::endl; //floats are twice the size of ints
                }
                else if(it->typeFlag == SyntaxInfo::FLOAT_ARRAY){
                    stream << "RESW"; //next iteration will say how large the array is
                    waitingOnArraySize = true;
                    arrayDecIsInt = false;
                }*/
                break;
            case SyntaxInfo::ARRAY_DEC_SIZE:
            	vars.top().top().second = it->name;
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
            	isFunc.push(true);
            	if(it->typeFlag == SyntaxInfo::VOID){
            		isFuncVoid = true;
            	}
            	else{
            		isFuncVoid = false;
            	}
                writeLater.top().push_back(std::pair<SyntaxInfo, bool>(*it, false)); //gotta write parameters first
                break;
            case SyntaxInfo::EXIT_FUN_DEC:
            	pointerVars.pop();
            	break;
            case SyntaxInfo::PARAMS:
            	pointerVars.push(std::vector<std::string>());
                break;
            case SyntaxInfo::EXIT_PARAMS:
            	formattedOutput(writeLater.top().back().first.name, output, 1);
            	formattedOutput("FDCL\n", output, 2); //seen all parameters now we can write function
                writeLater.top().pop_back();
                break;
            case SyntaxInfo::PARAM_DEC:
            	formattedOutput(it->name, output, 1);
            	formattedOutput("FPRM", output, 2);
            	if(it->typeFlag == SyntaxInfo::INT){
            		formattedOutput("1\n", output, 3); //ints are one word big
            	}
            	else if(it->typeFlag == SyntaxInfo::INT_ARRAY || it->typeFlag == SyntaxInfo::FLOAT_ARRAY){
            		formattedOutput("1\n", output, 3); //arrays are pass by reference and addresses are one word big
            		pointerVars.top().push_back(it->name);
            	}
            	else{
            		formattedOutput("2\n", output, 3); //floats are two words big
            	}
            	break;
            case SyntaxInfo::COMPOUND_STMT:
            	++depth;
            	formattedOutput("BLOC\n", output, 2);
            	vars.push(VarList());
            	break;
            case SyntaxInfo::EXIT_COMPOUND_STMT:
            {
            	std::stringstream jbloc;
            	if(depth > 1){
					jbloc << "__JBLOC" << writeBlockJumpLater++;
					formattedOutput("+J", output, 2);
					formattedOutput(jbloc.str() + "\n", output, 3);
            	}
            	if(depth == 1){
            		formattedOutput("FJBK\n", output, 2);
            	}
            	while(!vars.top().empty()){
            		formattedOutput(vars.top().top().first.name, output, 1);
            		if(vars.top().top().first.typeFlag == SyntaxInfo::INT){
            			formattedOutput("WORD\n", output, 2);
            		}
            		else if(vars.top().top().first.typeFlag == SyntaxInfo::FLOAT){
            			formattedOutput("RESW", output, 2);
            			formattedOutput("2\n", output, 3);
            		}
            		else{
            			formattedOutput("RESW", output, 2);
            			formattedOutput(vars.top().top().second + "\n", output, 3);
            		}
            		vars.top().pop();
            	}
            	formattedOutput("END", output, 2);
            	formattedOutput("BLOC\n", output, 3);
            	if(depth > 1){
            		formattedOutput(jbloc.str(), output, 1);
            	}
            	vars.pop();
            	--depth;
                break;
            }
            case SyntaxInfo::LOCAL_DECS:
            		/*finalOutput << it->name << "";
            		if(it->typeFlag == SyntaxInfo::INT){
            			finalOutput << "WORD" << std::endl;
            		}
            		else{
            			finalOutput << "RESW";
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
            	writeIfLabelLater.push(ifCounter++);
                break;
            case SyntaxInfo::BEGIN_IF_STMT:
            	formattedOutput("+LDA", output, 2);
            	formattedOutput(operandPrefix(writeLater.top().back()) + writeLater.top().back().first.name + "\n", output, 3);
                writeLater.top().pop_back();
                formattedOutput("+COMP", output, 2);
                formattedOutput("#0\n", output, 3);
                formattedOutput("+JEQ", output, 2);
                {
                	std::stringstream temp;
                	temp << "__ELSE" << writeIfLabelLater.top() << "\n";
                	formattedOutput(temp.str(), output, 3);
                }
                //writeIfLabelLater.push(ifCounter++);
                break;
            case SyntaxInfo::BEGIN_ELSE_STMT:
				{
					std::stringstream temp;
					temp << "__ELSE" << writeIfLabelLater.top();
					formattedOutput(temp.str(), output, 1);
				}
                writeIfLabelLater.pop();
                break;
            case SyntaxInfo::WHILE:
                formattedOutput("__LOOP", output, 1);
                writeWhileLableLater.push(whileCounter++);
                break;
            case SyntaxInfo::BEGIN_WHILE_STMT:
            	formattedOutput("+LDA", output, 2);
            	formattedOutput(operandPrefix(writeLater.top().back()) + writeLater.top().back().first.name + "\n", output, 3);
                formattedOutput("+COMP", output, 2);
                formattedOutput("#0\n", output, 3);
                formattedOutput("+JEQ", output, 2);
                {
                	std::stringstream temp;
                	temp << "__NWHILE" << writeWhileLableLater.top() << "\n";
                	formattedOutput(temp.str(), output, 3);
                }
                writeLater.top().pop_back();
                break;
            case SyntaxInfo::EXIT_WHILE_STMT:
            	formattedOutput("+J", output, 2);
            	{
            		std::stringstream temp;
            		temp << "__LOOP" << writeWhileLableLater.top() << "\n";
            		formattedOutput(temp.str(), output, 3);
            	}
            	{
            		std::stringstream temp;
            		temp << "__NWHILE" << writeWhileLableLater.top();
            		formattedOutput(temp.str(), output, 1);
            	}
                writeWhileLableLater.pop();
                break;
            case SyntaxInfo::RETURN:
                break;
            case SyntaxInfo::EXIT_RETURN_STMT:
            	if(!isFuncVoid){
					formattedOutput("FRET", output, 2);
					formattedOutput(operandPrefix(writeLater.top().back()) + writeLater.top().back().first.name + "\n", output, 3);
					writeLater.top().pop_back();
            	}
                break;
            case SyntaxInfo::INT_LITERAL:
            case SyntaxInfo::FLOAT_LITERAL:
            case SyntaxInfo::VAR:
            	if(std::find(pointerVars.top().begin(), pointerVars.top().end(), it->name) != pointerVars.top().end()){
            		writeLater.top().push_back(SyntaxAndPointer(*it, true)); //is a reference to an array parameter
            	}
            	else{
            		writeLater.top().push_back(SyntaxAndPointer(*it, false));
            	}
            	handleExpression(varCounter, compCounter, writeLater.top(), output);
                break;
            case SyntaxInfo::INDEX:
            	writeLater.push(SyntaxAndPointerStack());
            	break;
            case SyntaxInfo::EXIT_INDEX:
            {
            	std::string array = operandPrefix(writeLater.top().back()) + writeLater.top().back().first.name;
            	writeLater.top().pop_back();
            	formattedOutput("+LDX", output, 2);
            	formattedOutput(operandPrefix(writeLater.top().back()) + writeLater.top().back().first.name + "\n", output, 3);
                writeLater.pop();
                formattedOutput("+LDA", output, 2);
                formattedOutput(array + ",X" + "\n", output, 3);
                writeVarLabelLater.push(varCounter);
                {
                    SyntaxInfo temp;
                    temp.syntaxFlag = SyntaxInfo::VAR;
                    std::stringstream I_Miss_CPP_11;
                    I_Miss_CPP_11 << "__t" << varCounter;
                    temp.name = I_Miss_CPP_11.str();
                    writeLater.top().push_back(SyntaxAndPointer(temp, false));
                }
                formattedOutput("+STA", output, 2);
                {
                	std::stringstream temp;
                	temp << "__t" << varCounter++ << "\n";
                	formattedOutput(temp.str(), output, 3);
                	handleExpression(varCounter, compCounter, writeLater.top(), output);
                }
                break;
            }
            case SyntaxInfo::CALL:
            	writeLater.top().push_back(SyntaxAndPointer(*it, false)); //need to see args before calling
            	break;
            case SyntaxInfo::EXIT_CALL:
            	formattedOutput("FCLL", output, 2);
            	formattedOutput(writeLater.top().back().first.name + "\n", output, 3);
            	writeLater.top().pop_back();
            	formattedOutput("SDR", output, 2);
            	{
            		std::stringstream temp;
            		temp << "__t" << varCounter++;
            		writeLater.top().push_back(SyntaxAndPointer(SyntaxInfo(SyntaxInfo::VAR, -1, temp.str()), true));
            		formattedOutput(temp.str() + "\n", output, 3);
            		handleExpression(varCounter, compCounter, writeLater.top(), output);
            	}
            	break;
            case SyntaxInfo::ARG:
            	writeLater.push(SyntaxAndPointerStack());
            	break;
            case SyntaxInfo::EXIT_ARG:
            	formattedOutput("FARG", output, 2);
            	formattedOutput(operandPrefix(writeLater.top().back()) + writeLater.top().back().first.name + "\n", output, 3);
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
            	formattedOutput("ERROR", output, 2);
            	break;
            }
		}
        finalOutput << output.str();
    }
}
