CC = gcc
CFLAGS = -Wall -Wextra -I.
DEPS = myshell.h
OBJS = myshell.o myfunc.o
EXE = myshell

.PHONY: run clean

myshell: $(OBJS) 
	$(CC) -o $(EXE) $(OBJS) $(CFLAGS)

myshell.o: myshell.c  $(DEPS)
	$(CC) -c myshell.c $(CFLAGS)
	
myfunc.o: myfunc.c $(DEPS)
	$(CC) -c myfunc.c $(CFLAGS)

run:
	@./$(EXE)

debug: $(OBJS)
	$(CC) -g -o $(EXE) $(OBJS) $(CFLAGS)

clean:
	rm -f $(OBJS) $(EXE)
