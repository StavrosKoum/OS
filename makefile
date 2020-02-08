all:	run	Peers

run:	Coordinator.o
	gcc -ansi -Wall -pedantic -o run Coordinator.o -lm

Coordinator.o:	Coordinator.c
		gcc -c Coordinator.c -lm

Peers:	Peers.o
	gcc -ansi -Wall -pedantic -o Peers Peers.o -lm

Peers.o:	Peers.c
		gcc -c Peers.c -lm



clean:
	rm -f *.o Coordinator
	rm -f *.o Peers
