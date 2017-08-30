CC = gcc -Wall
LINK = -lgdbm -lcrypt -pthread

OBJS = bin/main.o bin/posts.o bin/login.o bin/client.o

bbs: $(OBJS)
	$(CC) $(OBJS) -o bbs $(LINK)

bin/%.o: src/%.c | bin
	$(CC) $< -c -o $@

bin:
	mkdir bin

.PHONY: clean

clean:
	rm bbs
	rm -r bin
