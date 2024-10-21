all: clean task0 loader start startup

loader: loader.o startup start
	ld -o loader loader.o startup.o start.o -L/usr/lib32 -lc -T linking_script -dynamic-linker /lib32/ld-linux.so.2

loader.o: loader.c
	gcc -m32 -c loader.c -o loader.o

task0: task0.o
	gcc -g -m32 -Wall -o task0 task0.o

task0.o: task0.c
	gcc -g -m32 -Wall -c -o task0.o task0.c

start: start.s
	nasm -f elf32 start.s -o start.o
	
startup: startup.s
	nasm -f elf32 startup.s -o startup.o
	
.PHONY: clean

clean:
	rm -f task0.o loader.o task0 loader