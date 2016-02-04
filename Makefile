CCClient=g++ -Wall -std=c++11 -pthread
CCServer=g++ -Wall

all: Server Client

Server: Server.o SendReceive.o ServerChild.o
	$(CCServer) -o Server Server.o SendReceive.o ServerChild.o

Client: Client.o SendReceive.o
	$(CCClient) -o Client Client.o SendReceive.o

clean:
	rm -f *.o core.* Client Server

Server.o: Server.cpp Server.h SendReceive.h
	$(CCServer) -c Server.cpp

Client.o: Client.cpp Client.h SendReceive.h
	$(CCClient) -c Client.cpp

ServerChild.o: ServerChild.cpp ServerChild.h
	$(CCServer) -c ServerChild.cpp

SendReceive.o: SendReceive.cpp SendReceive.h
	$(CCServer) -c SendReceive.cpp
