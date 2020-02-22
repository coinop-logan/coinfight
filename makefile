CXX = g++
CXXFLAGS = -g -Wall -std=c++0x

INC=-I/usr/include -I/home/oglog/cpp/common -I./
LIB=-lboost_system -lsfml-graphics -lsfml-system -lsfml-window -lGL -lGLU


%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@ $(INC)

all: client server

server: server.o engine.o vchpack.o myvectors.o cmds.o common.o
	$(CXX) $(CXXFLAGS) -o server $^ $(LIB)

client: client.o engine.o vchpack.o myvectors.o cmds.o common.o graphics.o
	$(CXX) $(CXXFLAGS) -o client $^ $(LIB)

test: test.o vchpack.o cmds.o
	$(CXX) $(CXXFLAGS) -o test $^ $(LIB)

vchpack.o:
	$(CXX) $(CXXFLAGS) -c /home/oglog/cpp/common/vchpack.cpp $^ $(LIB)

myvectors.o:
	$(CXX) $(CXXFLAGS) -c /home/oglog/cpp/common/myvectors.cpp $^ $(LIB)