CC=gcc
CFLAGS=-Wall
EXE_SERVER=server

all:  $(EXE_SERVER)

server: $(EXE_SERVER).c
	gcc -pthread -Wall -o $(EXE_SERVER) $(EXE_SERVER).c

clean:
	rm -f *.o $(EXE_SERVER)
