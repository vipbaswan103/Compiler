compile: driver.c parser.c lexer.c ast.c symbolTable.c
	gcc -o stage1exe driver.c parser.c lexer.c ast.c symbolTable.c -lm -g
