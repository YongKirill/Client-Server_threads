all: server client

server:
	g++ Program_2/infotex_2.cpp -o server -w

client:
	g++ Program_1/infotex_1.cpp -o client -w
