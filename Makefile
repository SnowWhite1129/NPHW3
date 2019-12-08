all:
	g++ httpserver.cpp -o httpserver -std=c++11 -lboost_system -pthread
	g++ console.cpp -o console.cgi -std=c++11 -lboost_system -pthread
