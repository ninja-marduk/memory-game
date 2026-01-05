# Makefile para Juego de Memoria con SDL2
# Compilador: g++ (MinGW/MSYS2)

CXX = g++
CXXFLAGS = -Wall -std=c++17 -O2
SDL_FLAGS = -lmingw32 -lSDL2main -lSDL2

TARGET = memory-game.exe
SRC = main.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(SDL_FLAGS)

clean:
	del $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
