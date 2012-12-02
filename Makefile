all:		netw

netw:		main.o
	gcc -o netw main.o


main.o:		main.c
	gcc -c main.c -Wall

clean:
	rm *.o netw
