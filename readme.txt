/* 
	GROUP 33
	Aryan Mehra 2017A7PS0077P
	Akshit Khanna 2017A7PS0023P
   	Vipin Baswan 2017A7PS0429P
   	Swadesh Vaibhav 2017A7PS0030P
*/

1) Since our code is 32-bit code, to run it on 64-bit PCs, multilib library of gcc must installed. Use the following commands:
	On ubuntu based system, run:
	sudo apt-get install gcc-multilib g++-multilib

2) To run the compiler:
	make
	./compiler <Name_of_the_program_file> code.asm (name of the file that contains asm code, can be anything)
	
	Eg:
		make
		./compiler c11.txt code.asm
	
	Now, choose appropriate options from the driver menu by entering a number from 0 to 9 (inclusive)

3) After generating code.asm, do the following to run the code

	nasm -f elf -F dwarf -g code.asm
	gcc -m32 code.o -o code
	./code
	
	NOTE: For running (3), user need to install multilib library using command shown in (1)