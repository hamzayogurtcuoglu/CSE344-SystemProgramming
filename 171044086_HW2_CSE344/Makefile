
CC = gcc
CFLAGS = -pedantic -errors -Wall -c -std=gnu99 

all: program

program: main.o 
	 $(CC) -o program main.o -lm
clean: 
	rm *.o program
