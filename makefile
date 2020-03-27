compile: driver.c parser.c lexer.c ast.c
	gcc -o stage1exe driver.c parser.c lexer.c ast.c -lm -g
