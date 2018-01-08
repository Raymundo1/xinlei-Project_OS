all: a1shell.o errors.o
	gcc -o a1shell a1shell.o errors.o

a1shell.o: a1shell.c errors.h
	gcc -c a1shell.c

errors.o: errors.c errors.h
	gcc -c errors.c

clean:
	rm -rf *.o
tar:
	tar -cvzf submit.tar Makefile a1shell.c errors.c errors.h project_report.pdf



