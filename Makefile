CC:=gcc
CFLAGS:=-lncurses -lmenu -Wall -g -O3
DEPS:=utils.h linked_list_proc.h

%.o: %.c $(DEPS)
	$(CC) -c $< -o $@ $(CFLAGS)

prochelper: prochelper.o utils.o linked_list_proc.o
	$(CC) -o $@ $^ $(CFLAGS)
	rm -f *.o

clean:
	rm -f *.o
	rm -f prochelper
