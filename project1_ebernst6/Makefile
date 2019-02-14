CFLAGS=-g -Wall

all: driver

my_malloc: my_malloc.h my_malloc.c
	gcc $(CFLAGS) -c --std=c11 my_malloc.c -o my_malloc.o

driver: my_malloc driver.c
	gcc $(CFLAGS) --std=c11 my_malloc.o driver.c -lpthread -o driver

.PHONY: clean

clean:
	rm -f *.o driver




