
all: submit

submit: submit.o send_recv_utility.o
	g++ -o submit submit.o send_recv_utility.o 

submit.o: submit.cpp send_recv_utility.h
	g++ -c submit.cpp

send_recv_utility.o: send_recv_utility.cpp send_recv_utility.h
	g++ -c send_recv_utility.cpp

clean:
	rm -f server *.o
