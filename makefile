CXX = g++
CXXFLAGS = -g -Wall -std=c++17 -pthread -no-pie `pkg-config --cflags glfw3`

INC=-I/usr/include -I../common -I./include/ -I../../git-external/glfw/include/
LIB=-lboost_system -lsfml-graphics -lsfml-system -lsfml-window -lGL -lGLU -lGLEW `pkg-config --static --libs glfw3`

all: bin/client bin/server bin/test bin/gltest

obj/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@ $(INC)

bin/server: obj/server.o obj/engine.o obj/vchpack.o obj/myvectors.o obj/cmds.o obj/common.o obj/coins.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIB)

bin/client: obj/client.o obj/engine.o obj/vchpack.o obj/myvectors.o obj/cmds.o obj/common.o obj/coins.o obj/graphics.o obj/input.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIB)

bin/test: obj/test.o obj/engine.o obj/vchpack.o obj/myvectors.o obj/cmds.o obj/common.o obj/graphics.o obj/coins.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIB)

obj/vchpack.o:
	$(CXX) $(CXXFLAGS) -c ../common/vchpack.cpp $^ $(LIB) -o $@

obj/myvectors.o:
	$(CXX) $(CXXFLAGS) -c ../common/myvectors.cpp $^ $(LIB) -o $@