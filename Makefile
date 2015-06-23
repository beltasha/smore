CC=gcc

all: tar

tar: smore.o util.o
            $(CC) smore.o util.o -o smore

smore.o: smore.c
            $(CC) -c smore.c

util.o: util.c
            $(CC) -c util.c
clean:
        rm *.o
