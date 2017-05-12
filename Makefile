# makefile for server.c client.c

# standard gcc compiler
CC = gcc

# for debugging
CFLAGS = -g -Wall

# compile all executable
all: server.c client.c 
	$(CC) $(CFLAGS) server.c -o server
	$(CC) $(CFLAGS) client.c -o client

# memory leak dection
valgrind: server
	valgrind --leak-check=full --show-leak-kinds=all ./server 

runserver:
	./server

runclient:
	./client	

# remove all object oject files and executables
clean: 
	rm server client

# <target>: <dependicies>
# [tab]<system command>