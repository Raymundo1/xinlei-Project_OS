all: a3chat clean

a3chat: a3chat.o client.o server.o
	gcc -pthread -o a3chat a3chat.o client.o server.o

a3chat.o: a3chat.c client.h server.h
	gcc -c a3chat.c

client.o: client.c client.h
	gcc -c client.c

server.o: server.c server.h
	gcc -c server.c

clean:
	rm -rf *.o

tar:
	tar -cvzf submit.tar README Makefile a3chat.c server.c server.h client.c client.h project_report.pdf