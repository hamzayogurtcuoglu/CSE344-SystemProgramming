
CC = gcc
CFLAGS = -pedantic -errors -Wall -c -std=gnu99 

all: floristApp

floristApp: main.o 
	 $(CC) -o floristApp main.o  -lm -lpthread -lrt 

main.o: main.c
	 $(CC) $(CFLAGS) main.c 

clean: 
	rm *.o floristApp
