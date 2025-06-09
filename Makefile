CC=gcc
CFLAGS=-I. -Wall -pthread
HEADERS=main.h
OBJS=main.o relayThread.o 

main.out: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

%.o : %.c $(HEADERS)
	$(CC) -c -o $@ $< $(CFLAGS)
	
all:
	main.out

clean: 
	rm *.out *.o
