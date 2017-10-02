CFLAGS = -std=c++11

all: client server

client : client.cpp
	g++ $(CFLAGS) client.cpp -o $@

server: server.cpp
	g++ $(CFLAGS) server.cpp -o $@