all: interpreter compiler asm runtest

interpreter: interpreter.cpp pcodes.c pcodes.h cli.c gc.h gccommon.h basetypes.cpp
	c++ -g -O2 interpreter.cpp -o interpreter

compiler: parser.c cpl2.c cpl2.h pcodes.h pcodes.c emit.c cli.c
	cc parser.c -o compiler
	
cpl2.c: cpl2.gram
	lemon cpl2.gram

asm: asm.c pcodes.h pcodes.c emit.c cli.c
	cc asm.c -o asm

runtest: runtest.c cli.c
	cc runtest.c -o runtest
