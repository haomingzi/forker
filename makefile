.PHONY:all
all:forker
forker.o:forker.c
	gcc -c forker.c -o forker.o 
procfork.o:procfork.c
	gcc -c procfork.c -o procfork.o
forker:forker.o procfork.o
	gcc forker.o procfork.o -o forker

.PHONY:clean
clean:
	-rm *.o forker
