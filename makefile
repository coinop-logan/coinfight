CXX = g++
CXXFLAGS = -g -Wall -std=c++17 -pthread -no-pie
UNAME := $(shell uname)
INC=-I/usr/include -I/usr/include/python3.8/ -I ./cpp/include/ `python3-config --includes`
LIBSERVER=-lboost_system -lsfml-graphics -lsfml-system -lboost_filesystem `python3-config --ldflags` -lpython3.8
ifeq ($(UNAME), Darwin)
LIBCLIENT=-lboost_system -lsfml-graphics -lsfml-system -lsfml-window -framework OpenGL
else
# ifeq($(UNAME), Linux)
LIBCLIENT=-lboost_system -lsfml-graphics -lsfml-system -lsfml-window -lGL -lGLU
endif

all: pre-build client-build server-build

release: pre-build client-build package-client server-build

clean:
	rm -f cpp/obj/*
	rm -rf bin/*
	rm -rf dist/*

prep-server:
	mkdir -p bin/events_in
	mkdir -p bin/events_in/deposits
	mkdir -p bin/events_out/withdrawals
	cp py/* bin/
	cp secret.txt bin/secret.txt
	cp web3-api-key bin/web3-api-key
	cp assets/server/* bin/

client: pre-build client-build

server: pre-build server-build prep-server

pre-build:
	mkdir -p cpp/obj
	mkdir -p bin/
	mkdir -p dist/

all: server-build client-build prep-server

server-build: bin/server

client-build: bin/coinfight
	cp assets/client/* bin/

package-client:
	cd package-assets/client && ./package.sh && mv coinfight-*.zip ../../dist/ && cd ../..

cpp/obj/%.o: cpp/src/%.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@ $(INC)

bin/coinfight: cpp/obj/coinfight.o cpp/obj/engine.o cpp/obj/myvectors.o cpp/obj/cmds.o cpp/obj/common.o cpp/obj/coins.o cpp/obj/graphics.o cpp/obj/input.o cpp/obj/packets.o cpp/obj/events.o cpp/obj/unit_interface_cmds.o cpp/obj/entities.o cpp/obj/interface.o cpp/obj/collision.o cpp/obj/netpack.o cpp/obj/tutorial.o cpp/obj/graphics_helpers.o cpp/obj/ui_elements.o cpp/obj/particles.o cpp/obj/client_networking.o cpp/obj/address.o cpp/obj/sigil.o cpp/obj/algorithm.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBCLIENT)

bin/server: cpp/obj/server.o cpp/obj/engine.o cpp/obj/myvectors.o cpp/obj/cmds.o cpp/obj/common.o cpp/obj/coins.o cpp/obj/packets.o cpp/obj/sigWrapper.o cpp/obj/events.o cpp/obj/entities.o cpp/obj/collision.o cpp/obj/netpack.o cpp/obj/address.o cpp/obj/sigil.o cpp/obj/algorithm.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBSERVER)

bin/test: cpp/obj/test.o cpp/obj/graphics_helpers.o cpp/obj/engine.o cpp/obj/myvectors.o cpp/obj/cmds.o cpp/obj/common.o cpp/obj/coins.o cpp/obj/graphics.o cpp/obj/input.o cpp/obj/packets.o cpp/obj/events.o cpp/obj/unit_interface_cmds.o cpp/obj/entities.o cpp/obj/interface.o cpp/obj/collision.o cpp/obj/netpack.o cpp/obj/tutorial.o cpp/obj/address.o cpp/obj/entroPic.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBCLIENT) $(LIBSERVER)
