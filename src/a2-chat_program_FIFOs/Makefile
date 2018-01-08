all: a2rchat clean

a2rchat: a2rchat.o client.o server.o
	gcc -o a2rchat a2rchat.o client.o server.o

a2rchat.o: a2rchat.c client.h server.h
	gcc -c a2rchat.c

client.o: client.c client.h
	gcc -c client.c

server.o: server.c server.h
	gcc -c server.c

clean:
	rm -rf *.o

tar:
	tar -cvzf submit.tar project_report.pdf README Makefile a2rchat.c client.c client.h fifo server.c server.h