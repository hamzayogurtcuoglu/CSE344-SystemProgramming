
CC = gcc
CFLAGS = -pedantic -errors -Wall -c -std=gnu99 

all: program

program: main.o 
	 $(CC) -o program main.o  -lm -lpthread -lrt 

main.o: main.c
	 $(CC) $(CFLAGS) main.c 

clean: 
	rm *.o program
