all:
	g++ http_server.cpp -o http_server -std=c++11 -lboost_system -pthread
	g++ console.cpp -o console.cgi -std=c++11 -lboost_system -pthread
