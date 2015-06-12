.PHONY:all
all:forker
forker.o:forker.c
	g++ -c forker.c -o forker.o 
procfork.o:procfork.c procfork.h
	g++ -c procfork.c -o procfork.o
worker.o:worker.c worker.h
	g++ -c worker.c -o worker.o
gdef.o:gdef.c gdef.h
	g++ -c gdef.c -o gdef.o
forker:forker.o procfork.o worker.o gdef.o
	g++ forker.o procfork.o worker.o gdef.o -o forker

.PHONY:clean
clean:
	-rm *.o forker
