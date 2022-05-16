CC=gcc
CFLAGS=-Wall
EXE_SERVER=server

all:  $(EXE_SERVER)

server: $(EXE_SERVER).c
	gcc -o $(EXE_SERVER) $(EXE_SERVER).c
