p3: main.o DFA.o LexicalAnalyzer.o Parser.o SymbolTable.o
	g++ -o p3 main.o DFA.o LexicalAnalyzer.o Parser.o SymbolTable.o
main.o: main.cpp LexicalAnalyzer.hpp Parser.hpp
	g++ -c main.cpp
DFA.o: DFA.cpp DFA.hpp
	g++ -c DFA.cpp
LexicalAnalyzer.o: LexicalAnalyzer.cpp LexicalAnalyzer.hpp DFA.hpp
	g++ -c LexicalAnalyzer.cpp
Parser.o: Parser.cpp LexicalAnalyzer.hpp SymbolTable.hpp Tree.hpp SyntaxInfo.hpp Parser.hpp
	g++ -c Parser.cpp
SymbolTable.o: SymbolTable.cpp SymbolTable.hpp
	g++ -c SymbolTable.cpp
