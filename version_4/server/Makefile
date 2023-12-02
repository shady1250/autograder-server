
all: server

server: server.o send_recv_utility.o FileQueue.o
	g++ -o server server.o send_recv_utility.o FileQueue.o

server.o: server.cpp send_recv_utility.h FileQueue.h
	g++ -c server.cpp

FileQueue.o: FileQueue.cpp FileQueue.h
	g++ -c FileQueue.cpp

send_recv_utility.o: send_recv_utility.cpp send_recv_utility.h
	g++ -c send_recv_utility.cpp

clean:
	rm -f server *.o
