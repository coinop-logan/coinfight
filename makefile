CXX = g++
CXXFLAGS = -g -Wall -std=c++17 -pthread -no-pie `pkg-config --cflags glfw3`

INC=-I/usr/include -I../common -I./include/ -I../../git-external/glfw/include/
LIB=-lboost_system -lsfml-graphics -lsfml-system -lsfml-window -lGL -lGLU -lGLEW `pkg-config --static --libs glfw3`

all: bin/client bin/server bin/test bin/gltest
	cp py/sig_to_address.py bin/sig_to_address.py
	cp secret.txt bin/secret.txt
	cp cpp/src/shaders/* bin/

cpp/obj/%.o: cpp/src/%.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@ $(INC)

bin/server: cpp/obj/server.o cpp/obj/engine.o cpp/obj/vchpack.o cpp/obj/myvectors.o cpp/obj/cmds.o cpp/obj/common.o cpp/obj/coins.o cpp/obj/packets.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIB)

bin/client: cpp/obj/client.o cpp/obj/engine.o cpp/obj/vchpack.o cpp/obj/myvectors.o cpp/obj/cmds.o cpp/obj/common.o cpp/obj/coins.o cpp/obj/graphics.o cpp/obj/input.o cpp/obj/packets.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIB)

bin/test: cpp/obj/test.o cpp/obj/engine.o cpp/obj/vchpack.o cpp/obj/myvectors.o cpp/obj/cmds.o cpp/obj/common.o cpp/obj/graphics.o cpp/obj/coins.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIB)

cpp/obj/vchpack.o:
	$(CXX) $(CXXFLAGS) -c ../common/vchpack.cpp $^ $(LIB) -o $@

cpp/obj/myvectors.o:
	$(CXX) $(CXXFLAGS) -c ../common/myvectors.cpp $^ $(LIB) -o $@