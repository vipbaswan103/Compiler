compile: driver.c parser.c lexer.c
	gcc -o stage1exe driver.c parser.c lexer.c -lm -g
