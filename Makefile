CC=gcc
CFLAGS=-Wall
EXE_SERVER=server
EXE_UTILS=serverUtils

all:  $(EXE_SERVER)

$(EXE_SERVER): $(EXE_SERVER).o $(EXE_UTILS).o
	$(CC) -pthread $(CFLAGS) -o $(EXE_SERVER) $(EXE_SERVER).o $(EXE_UTILS).o

$(EXE_SERVER).o: $(EXE_SERVER).c $(EXE_UTILS).h
	$(CC) -pthread $(CFLAGS) -c $(EXE_SERVER).c

$(EXE_UTILS).o: $(EXE_UTILS).c $(EXE_UTILS).h
	$(CC) -pthread $(CFLAGS) -c $(EXE_UTILS).c


clean:
	rm -f *.o $(EXE_SERVER) $(EXE_UTILS)
