# C- Compiler
###COP4620 Construction of Language Translators Class Project

###Program Description:
Fully functional compiler for the C- Language as described in Kenneth C. Louden's book, Compiler Construction: Principles and Practice, except now with support for the float type. 
The program first checks to make sure the inputted C- source code file is lexically, syntactically, and semantically correct. It uses deterministic finite automata for the lexical analyzer and a handwritten recursive descent parser for the syntax and semantic checking. Then at the same time the program is verifying the correctness of the C- code, it builds a syntax tree (different from a parse tree) of the C- Code. Using this syntax tree, the program then outputs SIC/XE assembly code.

###Program Usage: 

p4 filename [-e] [-t]

"filename" denotes the C- source code file you wish to compile. The "-e" argument will make the program output any semantic errors it finds. The "-t" argument will make the program output the syntax tree as a text file called "inputfile.tree.txt", where "inputfile" denotes the actual name of the C- file you inputted.
