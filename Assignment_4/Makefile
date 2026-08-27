CC = gcc
CFLAGS = -Wall -Wextra -std=gnu99

OBJS = main.o compiler.o processor.o memory.o os.o
TARGET = simulator

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

main.o: main.c os.h
	$(CC) $(CFLAGS) -c main.c

compiler.o: compiler.c compiler.h opcodes.h
	$(CC) $(CFLAGS) -c compiler.c

processor.o: processor.c processor.h memory.h opcodes.h common.h
	$(CC) $(CFLAGS) -c processor.c

memory.o: memory.c memory.h common.h
	$(CC) $(CFLAGS) -c memory.c

os.o: os.c os.h common.h compiler.h processor.h memory.h
	$(CC) $(CFLAGS) -c os.c

clean:
	rm -f $(OBJS) $(TARGET) log.txt tests/*.byte

.PHONY: all clean
