GIT_COMMIT_HASH = $(shell git rev-parse HEAD)

CXX = g++
UNAME := $(shell uname)
INC=-I/usr/include -I/usr/include/python3.8/ -I ./cpp/include/ `python3-config --includes`
LIBSERVER=-lboost_system -lsfml-graphics -lsfml-system -lboost_filesystem `python3-config --ldflags` -lpython3.8
ifeq ($(UNAME), Darwin)
CXXFLAGS = -g -Wall -std=c++17 -no-pie -arch x86_64 -DGIT_COMMIT_HASH='"$(GIT_COMMIT_HASH)"'
LIBCLIENT=-lboost_system -lsfml-graphics -lsfml-system -lsfml-window -framework OpenGL
else
# ifeq($(UNAME), Linux)
CXXFLAGS = -g -Wall -std=c++17 -pthread -no-pie -DGIT_COMMIT_HASH='"$(GIT_COMMIT_HASH)"'
LIBCLIENT=-lboost_system -lsfml-graphics -lsfml-system -lsfml-window -lGL -lGLU
endif

all: pre-build client-build server-build

abort-if-git-not-clean:
	@if test -n "$$(git status --porcelain)"; then \
		echo "ERROR: git repo is not clean."; \
		exit 1; \
	fi

release: abort-if-git-not-clean pre-build client-build package-client

clean:
	rm -f cpp/obj/*
	rm -rf bin/*
	rm -rf dist/*

prep-server:
	mkdir -p bin/events_in
	mkdir -p bin/events_in/deposits
	mkdir -p bin/events_out/withdrawals
	mkdir -p bin/sessions
	cp assets/server/* bin/

client: pre-build client-build

server: pre-build server-build prep-server

pre-build:
	mkdir -p cpp/obj
	mkdir -p bin/
	mkdir -p dist/

all: server-build client-build prep-server

server-build: bin/server

server-install: server-build
	sudo cp py/*.py /usr/bin/coinfight/python/
	sudo cp bin/server /usr/bin/coinfight/server

client-build: bin/coinfight
	cp -r assets/client/* bin/

package-client:
	cd package-assets/client && ./package.sh && mv coinfight-*.zip ../../dist/ && cd ../..

cpp/obj/test.o: cpp/src/test.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(INC)

cpp/obj/coinfight.o: cpp/src/coinfight.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(INC)

cpp/obj/server.o: cpp/src/server.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(INC)

cpp/obj/%.o: cpp/src/%.cpp cpp/src/%.h
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(INC)

bin/coinfight: cpp/obj/coinfight.o cpp/obj/engine.o cpp/obj/myvectors.o cpp/obj/cmds.o cpp/obj/common.o cpp/obj/coins.o cpp/obj/graphics.o cpp/obj/input.o cpp/obj/packets.o cpp/obj/events.o cpp/obj/entities.o cpp/obj/collision.o cpp/obj/netpack.o cpp/obj/tutorial.o cpp/obj/graphics_helpers.o cpp/obj/ui_elements.o cpp/obj/particles.o cpp/obj/client_networking.o cpp/obj/address.o cpp/obj/algorithm.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBCLIENT)

bin/server: cpp/obj/server.o cpp/obj/engine.o cpp/obj/myvectors.o cpp/obj/cmds.o cpp/obj/common.o cpp/obj/coins.o cpp/obj/packets.o cpp/obj/sigWrapper.o cpp/obj/events.o cpp/obj/entities.o cpp/obj/collision.o cpp/obj/netpack.o cpp/obj/address.o cpp/obj/algorithm.o cpp/obj/exec.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBSERVER)

bin/test: cpp/obj/test.o cpp/obj/common.o cpp/obj/myvectors.o cpp/obj/coins.o cpp/obj/netpack.o cpp/obj/address.o cpp/obj/algorithm.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBCLIENT) $(LIBSERVER)
