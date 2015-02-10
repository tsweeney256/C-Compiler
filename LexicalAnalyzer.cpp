#include <string>
#include <iterator>
#include <istream>
#include <sstream>
#include "DFA.hpp"
#include "LexicalAnalyzer.hpp"

LexicalAnalyzer::LexicalAnalyzer()
	: m_lastTokenFlag(-1), m_eof(false)
{
    //these functions used purely for organizational convenience
    buildCppCommentDFA(m_dfa[DFA_CPPCOMMENT]);
	buildCCommentDFA(m_dfa[DFA_CCOMMENT]);
	buildWhitespaceDFA(m_dfa[DFA_WHITESPACE]);
	buildKeywordDFA(m_dfa[DFA_KEYWORD]);
	buildOperatorsDFA(m_dfa[DFA_OPERATOR]);
	buildIntLiteralDFA(m_dfa[DFA_INT_LITERAL]);
	buildFloatLiteralDFA(m_dfa[DFA_FLOAT_LITERAL]);
	buildIDDFA(m_dfa[DFA_ID]);

	m_strInput = new std::istringstream;
}

LexicalAnalyzer::~LexicalAnalyzer()
{
	delete m_strInput;
}

void LexicalAnalyzer::setInput(const std::string& input)
{
	m_strInput->str(input);
    setInputCommon(m_strInput);
}

void LexicalAnalyzer::setInput(std::istream& input)
{
    m_input = &input;
    setInputCommon(m_input);
}

std::string LexicalAnalyzer::getNextToken()
{
    std::string token;
    int DFACount[NUM_DFA];
    int DFAFlag[NUM_DFA];
    for(int i=0; i<NUM_DFA; ++i){
        DFACount[i] = 0;
        DFAFlag[i] = DFA::NOT_DONE;
        m_dfa[i].reset();
    }
    int maxCount = 0;
    int prevFlag = DFA::NOT_DONE;

    std::istreambuf_iterator<char> eof;

    int lastSawStartCComment = 0;
    int lastSawEndCComment = 2; //just needs to be greater than 1

    while(m_inputIter != eof){
        char prevC = 0;
        char prevprevC = 0;
		int ccommentCounter = 0;

        bool somethingDidntReject = true; //I hate do-while loops
        while(somethingDidntReject && m_inputIter != eof){
            somethingDidntReject = false;

            for(int i=0; i<NUM_DFA; ++i){
                if(DFAFlag[i] != DFA::REJECTED){ //no sense in wasting cycles just to have an already rejected DFA tell us it's still rejected
                    prevFlag = DFAFlag[i];
                    if((DFAFlag[i] = m_dfa[i].feedNextCharacter(*m_inputIter)) != DFA::REJECTED){
                        somethingDidntReject = true;
                        ++DFACount[i];
                    }
                    //string rejected if the previous state was non-accepting and the current state rejects
                    else if(prevFlag == DFA::NOT_DONE){
                        DFACount[i] = 0;
                    }
                    //string accepted if the previous state was accepting and the current state rejects
                    //else if(prevFlag == ACCEPTING){}
                }
            }
            if(somethingDidntReject || maxCount == 0){
				//to allow for nested C comments
            	if(prevC == '/' && *m_inputIter == '*' && lastSawEndCComment > 1){
            		++ccommentCounter;
            		lastSawStartCComment = 0;
            	}

                token.append(1, *m_inputIter);
                ++maxCount;
                ++m_inputIter;
                prevprevC = prevC;
                prevC = *m_inputIter;
                ++lastSawStartCComment;
                ++lastSawEndCComment;
            }
            //to allow for nested C comments
            else if(prevprevC == '*' && prevC == '/' && ccommentCounter > 0 && lastSawStartCComment > 1){
				--ccommentCounter;
				lastSawEndCComment = 0;
				if(ccommentCounter > 0){
					m_dfa[DFA_CCOMMENT].reset();
					m_dfa[DFA_CCOMMENT].feedNextCharacter('/');
					m_dfa[DFA_CCOMMENT].feedNextCharacter('*');
					somethingDidntReject = true;
					DFAFlag[DFA_CCOMMENT] = DFA::NOT_DONE;
				}
			}
        }
        //don't report back any whitespace or comments and just do the whole thing over again
        if(DFACount[DFA_CPPCOMMENT] == maxCount ||
           DFACount[DFA_CCOMMENT] == maxCount ||
           DFACount[DFA_WHITESPACE] == maxCount){

            for(int i=0; i<NUM_DFA; ++i){
                m_dfa[i].reset();
                DFAFlag[i] = DFA::NOT_DONE;
                DFACount[i] = 0;
            }
            maxCount = 0;
            prevFlag = DFA::NOT_DONE;
            token = "";
        }
        else {
            if(DFACount[DFA_KEYWORD] == maxCount){
                m_lastTokenFlag = KEYWORD;
            }
            else if(DFACount[DFA_OPERATOR] == maxCount){
                m_lastTokenFlag = OPERATOR;
            }
            else if(DFACount[DFA_INT_LITERAL] == maxCount){
                m_lastTokenFlag = INT_LITERAL;
            }
            else if(DFACount[DFA_FLOAT_LITERAL] == maxCount){
                m_lastTokenFlag = FLOAT_LITERAL;
            }
            else if(DFACount[DFA_ID] == maxCount){
                m_lastTokenFlag = ID;
            }
            else{
                m_lastTokenFlag = ERROR;
            }
            return token;
        }
    }
    m_eof = true;
    m_lastTokenFlag = END_OF_INPUT;
    return token;
}

int LexicalAnalyzer::lastTokenFlag()
{
    return m_lastTokenFlag;
}

bool LexicalAnalyzer::eof()
{
    return m_eof;
}

void LexicalAnalyzer::setInputCommon(std::istream* input)
{
    m_inputIter = input->rdbuf();
    m_lastTokenFlag = -1;
    m_eof = false;
}

void LexicalAnalyzer::buildCppCommentDFA(DFA& cppcomment)
{
	cppcomment.start(1);

	cppcomment.add(1, 2, '/');
	cppcomment.add(2, 3, '/');
	cppcomment.add(3, 3, DFA::ANY);
	cppcomment.add(3, 5, '\r');
	cppcomment.add(3, 6, '\n');
	cppcomment.add(5, 6, '\n');

	cppcomment.accept(5); //\r for old macs
	cppcomment.accept(6); //\n for unix and \r\n for windows
}

void LexicalAnalyzer::buildCCommentDFA(DFA& ccomment)
{
	ccomment.start(1);

	ccomment.add(1, 2, '/');
	ccomment.add(2, 3, '*');
	ccomment.add(3, 3, DFA::ANY);
	ccomment.add(3, 5, '*');
	ccomment.add(5, 3, DFA::ANY); //needed because '*' takes precedence over ANY
	ccomment.add(5, 5, '*');
	ccomment.add(5, 6, '/');
	ccomment.add(3, 7, '/'); //needed for the nested comment hack so it doesn't terminate on "/*/*/"
	ccomment.add(7, 3, DFA::ANY); //also needed for the hack

	ccomment.accept(6);
}

void LexicalAnalyzer::buildWhitespaceDFA(DFA& whitespace)
{
	whitespace.start(1);

	whitespace.add(1, 2, DFA::BLANK);
	whitespace.add(2, 2, DFA::BLANK);
	whitespace.accept(2);
}

void LexicalAnalyzer::buildKeywordDFA(DFA& keywords)
{
	//I feel so silly doing this
	keywords.start(1);

	keywords.add(1, 2, 'e');
	keywords.add(2, 3, 'l');
	keywords.add(3, 4, 's');
	keywords.add(4, 5, 'e');
	keywords.accept(5); //else

	keywords.add(1, 6, 'i');
	keywords.add(6, 7, 'f');
	keywords.accept(7); //if

	keywords.add(6, 8, 'n');
	keywords.add(8, 9, 't');
	keywords.accept(9); //int

	keywords.add(1, 10, 'r');
	keywords.add(10, 11, 'e');
	keywords.add(11, 12, 't');
	keywords.add(12, 13, 'u');
	keywords.add(13, 14, 'r');
	keywords.add(14, 15, 'n');
	keywords.accept(15); //return

	keywords.add(1, 16, 'v');
	keywords.add(16, 17, 'o');
	keywords.add(17, 18, 'i');
	keywords.add(18, 19, 'd');
	keywords.accept(19); //void

	keywords.add(1, 20, 'w');
	keywords.add(20, 21, 'h');
	keywords.add(21, 22, 'i');
	keywords.add(22, 23, 'l');
	keywords.add(23, 24, 'e');
	keywords.accept(24); //while

	keywords.add(1, 25, 'f');
	keywords.add(25, 26, 'l');
	keywords.add(26, 27, 'o');
	keywords.add(27, 28, 'a');
	keywords.add(28, 29, 't');
	keywords.accept(29); //float
}

void LexicalAnalyzer::buildOperatorsDFA(DFA& operators)
{
	operators.start(1);

	operators.add(1, 2, '+');
	operators.accept(2);

	operators.add(1, 3, '-');
	operators.accept(3);

	operators.add(1, 4, '<');
	operators.accept(4);

	operators.add(1, 5, '>');
	operators.accept(5);

	operators.add(1, 6, '=');
	operators.accept(6);

	operators.add(1, 7, '!');
	operators.add(4, 8, '=');
	operators.add(5, 8, '=');
	operators.add(6, 8, '=');
	operators.add(7, 8, '=');
	operators.accept(8); //<=, >=, ==, !=

	operators.add(1, 9, ';');
	operators.accept(9);

	operators.add(1, 10, '(');
	operators.accept(10);

	operators.add(1, 11, ')');
	operators.accept(11);

	operators.add(1, 12, '[');
	operators.accept(12);

	operators.add(1, 13, ']');
	operators.accept(13);

	operators.add(1, 14, '{');
	operators.accept(14);

	operators.add(1, 15, '}');
	operators.accept(15);

	operators.add(1, 16, '*');
	operators.accept(16);

	operators.add(1, 17, '/');
	operators.accept(17);

	operators.add(1, 18, ',');
	operators.accept(18);
}

void LexicalAnalyzer::buildIntLiteralDFA(DFA& intLiteral)
{
	intLiteral.start(1);

	intLiteral.add(1, 2, DFA::DIGIT);
	intLiteral.add(2, 2, DFA::DIGIT);
	intLiteral.accept(2);
}

void LexicalAnalyzer::buildFloatLiteralDFA(DFA& floatLiteral)
{
	floatLiteral.start(1);
	floatLiteral.accept(2);
	floatLiteral.accept(4);
	floatLiteral.accept(8);

	floatLiteral.add(1, 2, DFA::DIGIT);
	floatLiteral.add(1, 3, '.');
	floatLiteral.add(2, 2, DFA::DIGIT);
	floatLiteral.add(2, 3, '.');
	floatLiteral.add(2, 5, 'E');
	floatLiteral.add(3, 4, DFA::DIGIT);
	floatLiteral.add(4, 4, DFA::DIGIT);
	floatLiteral.add(4, 5, 'E');
	floatLiteral.add(5, 6, '+');
	floatLiteral.add(5, 7, '-');
	floatLiteral.add(5, 8, DFA::DIGIT);
	floatLiteral.add(6, 8, DFA::DIGIT);
	floatLiteral.add(7, 8, DFA::DIGIT);
	floatLiteral.add(8, 8, DFA::DIGIT);
}

void LexicalAnalyzer::buildIDDFA(DFA& ID)
{
	ID.start(1);

	ID.add(1, 2, DFA::ALPHA);
	ID.add(2, 2, DFA::ALPHA);
	ID.accept(2);
}


