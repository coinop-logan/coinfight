CXX = g++
CXXFLAGS = -g -Wall -std=c++17 -pthread -no-pie `pkg-config --cflags glfw3`

INC=-I/usr/include -I/usr/include/python3.8/ -I../common -I./include/ -I../../git-external/glfw/include/ `python3-config --includes`
LIB=-lboost_system -lboost_filesystem -lsfml-graphics -lsfml-system -lsfml-window -lGL -lGLU -lGLEW `pkg-config --static --libs glfw3` `python3-config --ldflags` -lpython3.8

all: bin/client bin/server bin/test bin/gltest
	cp py/* bin/
	cp secret.txt bin/secret.txt
	cp cpp/src/shaders/* bin/

cpp/obj/%.o: cpp/src/%.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@ $(INC)

bin/server: cpp/obj/server.o cpp/obj/engine.o cpp/obj/vchpack.o cpp/obj/myvectors.o cpp/obj/cmds.o cpp/obj/common.o cpp/obj/coins.o cpp/obj/packets.o cpp/obj/sigWrapper.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIB)

bin/client: cpp/obj/client.o cpp/obj/engine.o cpp/obj/vchpack.o cpp/obj/myvectors.o cpp/obj/cmds.o cpp/obj/common.o cpp/obj/coins.o cpp/obj/graphics.o cpp/obj/input.o cpp/obj/packets.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIB)

bin/test: cpp/obj/test.o cpp/obj/engine.o cpp/obj/vchpack.o cpp/obj/myvectors.o cpp/obj/cmds.o cpp/obj/common.o cpp/obj/graphics.o cpp/obj/coins.o cpp/obj/sigWrapper.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIB)

cpp/obj/vchpack.o:
	$(CXX) $(CXXFLAGS) -c ../common/vchpack.cpp $^ $(LIB) -o $@

cpp/obj/myvectors.o:
	$(CXX) $(CXXFLAGS) -c ../common/myvectors.cpp $^ $(LIB) -o $@