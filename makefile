COMPILER = gcc
CFLAGS = -Wall -Werror -g -fsanitize=address,undefined
COMPILE_OBJECT = $(COMPILER) $(CFLAGS) -c $< -o $@

main: main.o heap.o
	$(COMPILER) $(CFLAGS) -o $@ $^

main.o: main.c heap.h
	$(COMPILE_OBJECT)

heap.o: heap.c heap.h
	$(COMPILE_OBJECT)

clean:
	rm -f *.o main