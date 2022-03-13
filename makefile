CXX = g++
CXXFLAGS = -g -Wall -std=c++17 -pthread -no-pie

INC=-I/usr/include -I/usr/include/python3.8/ -I../common -I./include/ `python3-config --includes`
LIBSERVER=-lboost_system -lsfml-graphics -lsfml-system -lboost_filesystem `python3-config --ldflags` -lpython3.8
LIBCLIENT=-lboost_system -lsfml-graphics -lsfml-system -lsfml-window -lGL -lGLU -lGLEW

all: pre-build main-build

clean:
	rm cpp/obj/* -f
	rm bin/* -rf

prep-server:
	mkdir -p bin/accounting
	mkdir -p bin/accounting/pending_deposits
	mkdir -p bin/accounting/pending_withdrawals
	cp py/* bin/
	cp secret.txt bin/secret.txt

install: all
	sudo apt install libsfml-dev

install-client: client
	sudo apt install libsfml-dev

client: pre-build client-build
	cp assets/Andale_Mono.ttf bin/

server: pre-build server-build prep-server

pre-build:
	mkdir -p cpp/obj
	mkdir -p bin/

main-build: pre-build server-build client-build bin/coinfight_local prep-server

server-build: server

client-build: client bin/coinfight_local

cpp/obj/%.o: cpp/src/%.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@ $(INC)

bin/coinfight_local: cpp/obj/coinfight_local.o cpp/obj/engine.o cpp/obj/vchpack.o cpp/obj/myvectors.o cpp/obj/cmds.o cpp/obj/common.o cpp/obj/coins.o cpp/obj/packets.o cpp/obj/events.o cpp/obj/input.o cpp/obj/graphics.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBCLIENT)

bin/server: cpp/obj/server.o cpp/obj/engine.o cpp/obj/vchpack.o cpp/obj/myvectors.o cpp/obj/cmds.o cpp/obj/common.o cpp/obj/coins.o cpp/obj/packets.o cpp/obj/sigWrapper.o cpp/obj/events.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBSERVER)

bin/client: cpp/obj/client.o cpp/obj/engine.o cpp/obj/vchpack.o cpp/obj/myvectors.o cpp/obj/cmds.o cpp/obj/common.o cpp/obj/coins.o cpp/obj/graphics.o cpp/obj/input.o cpp/obj/packets.o cpp/obj/events.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBCLIENT)

bin/test: cpp/obj/test.o cpp/obj/engine.o cpp/obj/vchpack.o cpp/obj/myvectors.o cpp/obj/cmds.o cpp/obj/common.o cpp/obj/graphics.o cpp/obj/coins.o cpp/obj/input.o cpp/obj/sigWrapper.o cpp/obj/graphics.o cpp/obj/events.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBCLIENT) $(LIBSERVER)

cpp/obj/vchpack.o:
	$(CXX) $(CXXFLAGS) -c ../common/vchpack.cpp $^ -o $@

cpp/obj/myvectors.o:
	$(CXX) $(CXXFLAGS) -c ../common/myvectors.cpp $^ -o $@