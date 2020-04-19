compile: driver.c parser.c lexer.c ast.c symbolTable.c semantics.c codegen.c nasmcode.c
	gcc -o stage1exe driver.c parser.c lexer.c ast.c symbolTable.c semantics.c codegen.c nasmcode.c -lm -g
