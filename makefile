CC = gcc -Wall -lgdbm

OBJS = bin/main.o

bbs: $(OBJS)
	$(CC) $(OBJS) -o bbs

bin/%.o: src/%.c | bin
	$(CC) $< -c -o $@

bin:
	mkdir bin

.PHONY: clean

clean:
	rm -r bin
