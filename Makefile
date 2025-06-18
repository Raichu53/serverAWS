CC=gcc
CFLAGS=-IInc -Wextra -pthread -lsystemd
HEADERS=main.h queue.h parser.h relayThread.h
SRC=Src/main.c Src/relayThread.c Src/parser.c Src/queue.c
OBJS=$(SRC:.c=.o)
TARGET=main.out

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

%.o : %.c $(HEADERS)
	$(CC) -c -o $@ $< $(CFLAGS)
	
all: $(TARGET)

clean: 
	rm $(TARGET) $(OBJS)
