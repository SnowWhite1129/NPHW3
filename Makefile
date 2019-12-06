CXX=clang++
CXXFLAGS=-std=c++11 -Wall -pedantic -pthread -lboost_system
CXX_INCLUDE_DIRS=/usr/local/include
CXX_INCLUDE_PARAMS=$(addprefix -I , $(CXX_INCLUDE_DIRS))
CXX_LIB_DIRS=/usr/local/lib
CXX_LIB_PARAMS=$(addprefix -L , $(CXX_LIB_DIRS))
CPP_CGI_SRC=$(wildcard *.cpp)
CPP_CGI=$(patsubst %.cpp,%.cgi,$(CPP_CGI_SRC))

all: httpserver | $(CPP_CGI)

%: %.cpp
	$(CXX) $< -o $@ $(CXX_INCLUDE_PARAMS) $(CXX_LIB_PARAMS) $(CXXFLAGS)

install: all | ~/public_html
	cp *.cgi ~/public_html

~/public_html:
	mkdir -p ~/public_html

clean:
	rm -f *.cgi $(CPP_CGI)
