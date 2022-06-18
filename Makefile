CC:=gcc
CFLAGS:=-lpthread -lncurses -lmenu -Wall -g -O3
DEPS:=utils.h linked_list_proc.h

%.o: %.c $(DEPS)
	$(CC) -c $< -o $@ $(CFLAGS)

main: main.o utils.o window.o linked_list_proc.o
	$(CC) -o $@ $^ $(CFLAGS)
	rm -f *.o

clean:
	rm -f *.o
	rm -f main
