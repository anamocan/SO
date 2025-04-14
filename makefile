CC=gcc
CFLAGS=-Wall

all: treasure_manager

treasure_manager: main.o treasure.o
	$(CC) -o treasure_manager main.o treasure.o

main.o: main.c treasure.h
	$(CC) $(CFLAGS) -c main.c

treasure.o: treasure.c treasure.h
	$(CC) $(CFLAGS) -c treasure.c

clean:
	rm -f *.o treasure_manager
