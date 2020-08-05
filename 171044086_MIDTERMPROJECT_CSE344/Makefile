
CC = gcc
CFLAGS = -pedantic -errors -Wall -c -std=gnu99 

all: program

program: main.o supplier.o cooks.o studentandgraduate.o
	 $(CC) -o program main.o supplier.o cooks.o studentandgraduate.o -lm -lpthread -lrt

main.o: main.c
	 $(CC) $(CFLAGS) main.c supplier.h cooks.h studentandgraduate.h

supplier.o: supplier.c
	 $(CC) $(CFLAGS) supplier.c supplier.h


cooks.o: cooks.c
	 $(CC) $(CFLAGS) cooks.c cooks.h

studentandgraduate.o: studentandgraduate.c
	 $(CC) $(CFLAGS) studentandgraduate.c studentandgraduate.h


clean: 
	rm *.o program
