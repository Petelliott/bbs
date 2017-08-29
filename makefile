CC = gcc -Wall -lgdbm -lcrypt

OBJS = bin/main.o bin/posts.o bin/login.o

bbs: $(OBJS)
	$(CC) $(OBJS) -o bbs

bin/%.o: src/%.c | bin
	$(CC) $< -c -o $@

bin:
	mkdir bin

.PHONY: clean

clean:
	rm -r bin
