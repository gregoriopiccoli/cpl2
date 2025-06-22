all: interpreter compiler asm

interpreter: interpreter.cpp pcodes.c pcodes.h gc.cpp
	c++ -g -O2 interpreter.cpp -o interpreter

compiler: parser.c cpl2.c cpl2.h pcodes.h pcodes.c emit.c
	cc parser.c -o compiler
	
cpl2.c: cpl2.gram
	lemon cpl2.gram

asm: asm.c
	cc asm.c -o asm
