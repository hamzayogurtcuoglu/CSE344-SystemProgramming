
CC = gcc
CFLAGS = -pedantic -errors -Wall -c -std=gnu99 

all: programA programB programC

programA: mainA.o 
	 $(CC) -o programA mainA.o -lm


programB: mainB.o 
	 $(CC) -o programB mainB.o -lm


programC: mainC.o 
	 $(CC) -o programC mainC.o -lm

clean: 
	rm *.o programA programB programC
