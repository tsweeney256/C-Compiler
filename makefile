p1: main.o DFA.o LexicalAnalyzer.o
	g++ -o p1 main.o DFA.o LexicalAnalyzer.o
main.o: main.cpp LexicalAnalyzer.hpp
	g++ -c main.cpp
DFA.o: DFA.cpp DFA.hpp
	g++ -c DFA.cpp
LexicalAnalyzer.o: LexicalAnalyzer.cpp LexicalAnalyzer.hpp DFA.hpp
	g++ -c LexicalAnalyzer.cpp
