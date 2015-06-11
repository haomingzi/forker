.PHONY:all
all:forker
forker.o:forker.c
	gcc -c forker.c -o forker.o 
forker:forker.o
	gcc forker.o -o forker

.PHONY:clean
clean:
	-rm forker.o forker
