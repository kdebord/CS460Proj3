P3.out : Project3.o SetLimits.o LexicalAnalyzer.o SyntacticalAnalyzer.o Object.o CodeGenerator.o
	g++ -g -o P3.out Project3.o SetLimits.o LexicalAnalyzer.o SyntacticalAnalyzer.o Object.o CodeGenerator.o

Project3.o : Project3.cpp SetLimits.h SyntacticalAnalyzer.h
	g++ -g -c Project3.cpp

SetLimits.o : SetLimits.cpp SetLimits.h
	g++ -g -c SetLimits.cpp

Object.o : Object.cpp Object.h
	g++ -g -c Object.cpp

CodeGenerator.o : CodeGenerator.cpp CodeGenerator.h
	g++ -g -c CodeGenerator.cpp

SyntacticalAnalyzer.o : SyntacticalAnalyzer.cpp SyntacticalAnalyzer.h LexicalAnalyzer.h
	g++ -g -c SyntacticalAnalyzer.cpp

clean : 
	rm [SP]*.o P3.out *.gch

submit : Project3.cpp LexicalAnalyzer.h SyntacticalAnalyzer.h SyntacticalAnalyzer.cpp makefile README.txt
	rm -rf Team12P3
	mkdir Team12P3
	cp Project3.cpp Team12P3
	cp SyntacticalAnalyzer.h Team12P3
	cp SyntacticalAnalyzer.cpp Team12P3
	cp makefile Team12P3
	cp README.txt Team12P3
	tar cfvz Team12P3.tgz Team12P3
	cp Team12P3.tgz ~tiawatts/cs460drop
