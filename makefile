GIT_COMMIT_HASH = $(shell git rev-parse HEAD)

CXX = g++
UNAME := $(shell uname)
INC=-I/usr/local/include -I/usr/include -I./cpp/src/ -I/usr/include/python3.8/ -I ./cpp/include/ `python3-config --includes`
LIBSERVER=-lboost_system -lboost_filesystem `python3-config --ldflags` -lpython3.8
LIBLAUNCHER=-lboost_system -lsfml-graphics -lsfml-system -lsfml-window -lboost_filesystem -lcurl
ifeq ($(UNAME), Darwin)
CXXFLAGS = -g -Wall -std=c++17 -no-pie -arch x86_64 -DGIT_COMMIT_HASH='"$(GIT_COMMIT_HASH)"'
LIBCLIENT=-lboost_system -lsfml-graphics -lsfml-system -lsfml-window -framework OpenGL
else
# ifeq($(UNAME), Linux)
CXXFLAGS = -g -Wall -std=c++17 -pthread -no-pie -DGIT_COMMIT_HASH='"$(GIT_COMMIT_HASH)"'
LIBCLIENT=-lboost_system -lsfml-graphics -lsfml-system -lsfml-window
endif

SRC_DIR = cpp/src
OBJ_DIR = cpp/obj
SRC_FILES := $(wildcard $(SRC_DIR)/*/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC_FILES))

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(INC)

launcher: directories launcher-build

directories:
	@mkdir -p bin/
	@mkdir -p dist/

.PHONY: clean
clean:
	rm -rf $(OBJ_DIR)
	rm -rf bin/*
	rm -rf dist/*

launcher-build: bin/launcher
	cp -r assets/client/* bin/

bin/launcher: $(OBJ_DIR)/launcher/launcher.o cpp/obj/interface/graphics/common.o cpp/obj/interface/common.o cpp/obj/common/utils.o cpp/obj/common/myvectors.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBLAUNCHER)