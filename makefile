GIT_COMMIT_HASH = $(shell git rev-parse HEAD)

CXX = g++
UNAME := $(shell uname)
INC=-I/usr/include -I/usr/include/python3.8/ -I ./cpp/include/ `python3-config --includes`
LIBSERVER=-lboost_system -lboost_filesystem `python3-config --ldflags` -lpython3.8
LIBLAUNCHER=-lboost_system -lsfml-graphics -lsfml-system -lboost_filesystem
ifeq ($(UNAME), Darwin)
CXXFLAGS = -g -Wall -std=c++17 -no-pie -arch x86_64 -DGIT_COMMIT_HASH='"$(GIT_COMMIT_HASH)"'
LIBCLIENT=-lboost_system -lsfml-graphics -lsfml-system -lsfml-window -framework OpenGL
else
# ifeq($(UNAME), Linux)
CXXFLAGS = -g -Wall -std=c++17 -pthread -no-pie -DGIT_COMMIT_HASH='"$(GIT_COMMIT_HASH)"'
LIBCLIENT=-lboost_system -lsfml-graphics -lsfml-system -lsfml-window -lGL -lGLU
endif

launcher: pre-build launcher-build

launcher-build: bin/launcher

pre-build:
	mkdir -p cpp/obj
	mkdir -p bin/
	mkdir -p dist/

cpp/obj/launcher/launcher.o: cpp/src/launcher/launcher.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(INC)

%.o: %.cpp %.h
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(INC)

bin/launcher: cpp/obj/launcher/launcher.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBLAUNCHER)