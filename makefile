.PHONY:all
all:forker
forker.o:forker.c
	gcc -c forker.c -o forker.o 
procfork.o:procfork.c
	gcc -c procfork.c -o procfork.o
worker.o:worker.c
	gcc -c worker.c -o worker.o
forker:forker.o procfork.o worker.o
	gcc forker.o procfork.o worker.o -o forker

.PHONY:clean
clean:
	-rm *.o forker
