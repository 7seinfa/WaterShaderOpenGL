CXX	:= g++
CXX_FLAGS	:= -g -std=c++17 #-Wextra -Wall

SRC	:= src
INCLUDE	:= C:/msys64/mingw64/include
INCLUDE2 := lib
LIB	:= C:/msys64/mingw64/lib

LIBRARIES	:= -lglfw3 -lfreeglut -lglew32 -lopengl32
EXECUTABLE	:= main

all:./$(EXECUTABLE)

run: all
	./$(EXECUTABLE)

$(EXECUTABLE):$(SRC)/*
	$(CXX) $(CXX_FLAGS) -I$(INCLUDE) -I$(INCLUDE2) -L$(LIB) $^ -o $@ $(LIBRARIES)