CC:=gcc
CFLAGS:=-lpthread -lncurses -Wall -g -O3
DEPS:=window.h linked_list_proc.h

%.o: %.c $(DEPS)
	$(CC) -c $< -o $@ $(CFLAGS)

main: main.o window.o linked_list_proc.o
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f *.o
	rm -f main
