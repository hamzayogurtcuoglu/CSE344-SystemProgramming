
CC = gcc
CFLAGS = -pedantic -errors -Wall -c -std=gnu99 

all: client server 

server: server.o 
	 $(CC) -o server server.o -lm  -lpthread -lrt 


client: client.o 
	 $(CC) -o client client.o -lm


clean: 
	rm *.o client server
