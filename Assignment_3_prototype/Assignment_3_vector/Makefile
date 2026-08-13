#compiler used
CC = gcc 

#flags : Wall -> show warnings
CFLAGS = -Wall 

#name of final program built
TARGET = simulator 

#object files needed to build the program
OBJS = main.o compiler.o processor.o memory.o #

# target : dependencies
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)	#command to build target

#building each object file
main.o: main.c compiler.h processor.h memory.h
	$(CC) $(CFLAGS) -c main.c

compiler.o: compiler.c compiler.h
	$(CC) $(CFLAGS) -c compiler.c

processor.o: processor.c processor.h memory.h
	$(CC) $(CFLAGS) -c processor.c

memory.o: memory.c memory.h
	$(CC) $(CFLAGS) -c memory.c

#deletes generated files
clean:
	rm -f $(OBJS) $(TARGET) program.byte 
