all: mrefgen a4vmsim clean

mrefgen:
	gcc -o mrefgen mrefgen.c -lm

a4vmsim: a4vmsim.o none.o mrand.o lru.o sec.o
	gcc -o a4vmsim a4vmsim.o none.o mrand.o lru.o sec.o -lm

a4vmsim.o: a4vmsim.c none.h mrand.h lru.h sec.h
	gcc -c a4vmsim.c

none.o: none.c none.h
	gcc -c none.c

mrand.o: mrand.c mrand.h
	gcc -c mrand.c

lru.o: lru.c lru.h
	gcc -c lru.c

sec.o: sec.c sec.h
	gcc -c sec.c

clean:
	rm -rf *.o

tar:
	tar -cvzf submit.tar README Makefile project_report.pdf none.c none.h mrand.c mrand.h lru.c lru.h sec.c sec.h a4vmsim.c mrefgen.c run.sh