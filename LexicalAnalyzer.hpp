#ifndef LEXICALANALYZER_HPP
#define LEXICALANALYZER_HPP

#include <string>
#include <istream>
#include <sstream>
#include <iterator>
#include "DFA.hpp"

class LexicalAnalyzer
{
public:
	LexicalAnalyzer();
	~LexicalAnalyzer();
	void setInput(const std::string& input);
	void setInput(std::istream& input);
	std::string getNextToken();
	int lastTokenFlag(); //returns one of the enumerated token flags
	bool eof();

	//token flags
	enum
	{
		KEYWORD,
		OPERATOR,
		INT_LITERAL,
		FLOAT_LITERAL,
		ID,
		ERROR,
		END_OF_INPUT
	};

private:
    enum
    {
        DFA_CPPCOMMENT,
        DFA_CCOMMENT,
        DFA_WHITESPACE,
        DFA_KEYWORD,
        DFA_OPERATOR,
        DFA_INT_LITERAL,
        DFA_FLOAT_LITERAL,
        DFA_ID,
        NUM_DFA //always keep this one on the bottom
    };

    void buildCppCommentDFA(DFA& cppcomment);
    void buildCCommentDFA(DFA& ccomment);
    void buildWhitespaceDFA(DFA& whitespace);
    void buildKeywordDFA(DFA& keywords);
    void buildOperatorsDFA(DFA& operators);
    void buildIntLiteralDFA(DFA& intLiteral);
    void buildFloatLiteralDFA(DFA& floatLiteral);
    void buildIDDFA(DFA& ID);

    void setInputCommon(std::istream* input);

	DFA m_dfa[NUM_DFA];

	std::istream* m_input;
	std::istringstream* m_strInput;
	std::istreambuf_iterator<char> m_inputIter;

	int m_lastTokenFlag;

	bool m_eof;
};

#endif



