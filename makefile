.PHONY:all
all:forker
forker.o:forker.c
	gcc -c forker.c -o forker.o 
procfork.o:procfork.c procfork.h
	gcc -c procfork.c -o procfork.o
worker.o:worker.c worker.h
	gcc -c worker.c -o worker.o
gdef.o:gdef.c gdef.h
	gcc -c gdef.c -o gdef.o
forker:forker.o procfork.o worker.o gdef.o
	gcc forker.o procfork.o worker.o gdef.o -o forker

.PHONY:clean
clean:
	-rm *.o forker
