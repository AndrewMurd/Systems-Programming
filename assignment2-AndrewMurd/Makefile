.PHONY : all clean
CFLAGS = -Wall -g -std=gnu99

all : server client

server : server.o lib.o
	cc -o server server.o lib.o -lpthread

client : client.o lib.o
	cc -o client client.o lib.o -lpthread

lib.o : lib.h

server.o : lib.h

client.o : lib.h

clean :
	rm client server client.o server.o lib.o
