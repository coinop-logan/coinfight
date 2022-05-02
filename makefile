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

all: pre-build main-build

release: pre-build main-build package-client

clean:
	rm -f cpp/obj/*
	rm -rf bin/*
	rm -rf dist/*

prep-server:
	mkdir -p bin/accounting
	mkdir -p bin/accounting/pending_deposits
	mkdir -p bin/accounting/pending_withdrawals
	cp py/* bin/
	cp secret.txt bin/secret.txt
	cp web3-api-key bin/web3-api-key
	cp package-assets/server/* bin/

client: pre-build client-build bin/coinfight_local

server: pre-build server-build prep-server

pre-build:
	mkdir -p cpp/obj
	mkdir -p bin/
	mkdir -p dist/

main-build: server-build client-build bin/coinfight_local prep-server

server-build: bin/server

client-build: bin/client bin/coinfight_local
	cp assets/Andale_Mono.ttf bin/

package-client:
	cd package-assets/client && ./package.sh && mv coinfight-client-linux.zip ../../dist/ && cd ../..

cpp/obj/%.o: cpp/src/%.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@ $(INC)

bin/coinfight_local: cpp/obj/coinfight_local.o cpp/obj/engine.o cpp/obj/vchpack.o cpp/obj/myvectors.o cpp/obj/cmds.o cpp/obj/common.o cpp/obj/coins.o cpp/obj/packets.o cpp/obj/events.o cpp/obj/input.o cpp/obj/graphics.o cpp/obj/unit_interface_cmds.o cpp/obj/entities.o cpp/obj/interface.o cpp/obj/collision.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBCLIENT)

bin/client: cpp/obj/client.o cpp/obj/engine.o cpp/obj/vchpack.o cpp/obj/myvectors.o cpp/obj/cmds.o cpp/obj/common.o cpp/obj/coins.o cpp/obj/graphics.o cpp/obj/input.o cpp/obj/packets.o cpp/obj/events.o cpp/obj/unit_interface_cmds.o cpp/obj/entities.o cpp/obj/interface.o cpp/obj/collision.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBCLIENT)

bin/server: cpp/obj/server.o cpp/obj/engine.o cpp/obj/vchpack.o cpp/obj/myvectors.o cpp/obj/cmds.o cpp/obj/common.o cpp/obj/coins.o cpp/obj/packets.o cpp/obj/sigWrapper.o cpp/obj/events.o cpp/obj/entities.o cpp/obj/collision.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBSERVER)

bin/test: cpp/obj/test.o cpp/obj/netpack.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBCLIENT) $(LIBSERVER)
