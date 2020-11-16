CC=gcc
CFLAGS=-I.

client: client.c requests.c helpers.c parson.c buffer.c
	$(CC) -g -o client client.c requests.c helpers.c buffer.c parson.c -Wall

run: client
	./client

run-debug: client
	./client debug

clean:
	rm -f *.o client
