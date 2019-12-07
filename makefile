P2.out : Project2.o SetLimits.o LexicalAnalyzer.o SyntacticalAnalyzer.o Object.o CodeGenerator.o
	g++ -g -o P2.out Project2.o SetLimits.o LexicalAnalyzer.o SyntacticalAnalyzer.o Object.o CodeGenerator.o

Project2.o : Project2.cpp SetLimits.h SyntacticalAnalyzer.h
	g++ -g -c Project2.cpp

SetLimits.o : SetLimits.cpp SetLimits.h
	g++ -g -c SetLimits.cpp

Object.o : Object.cpp Object.h
	g++ -g -c Object.cpp

CodeGenerator.o : CodeGenerator.cpp CodeGenerator.h
	g++ -g -c CodeGenerator.cpp

SyntacticalAnalyzer.o : SyntacticalAnalyzer.cpp SyntacticalAnalyzer.h LexicalAnalyzer.h
	g++ -g -c SyntacticalAnalyzer.cpp

clean : 
	rm [SP]*.o P2.out *.gch

submit : Project2.cpp LexicalAnalyzer.h SyntacticalAnalyzer.h SyntacticalAnalyzer.cpp makefile README.txt
	rm -rf JadeP2
	mkdir JadeP2
	cp Project2.cpp JadeP2
	cp SyntacticalAnalyzer.h JadeP2
	cp SyntacticalAnalyzer.cpp JadeP2
	cp makefile JadeP2
	cp README.txt JadeP2
	tar cfvz JadeP2.tgz JadeP2
	cp JadeP2.tgz ~tiawatts/cs460drop
