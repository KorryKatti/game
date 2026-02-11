CC = g++
CFLAGS = -Wall -Wextra -std=c++17
LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

SRC = src/main.cpp
OUT = build/game

all: $(OUT)

$(OUT): $(SRC)
	@mkdir -p build
	$(CC) $(CFLAGS) $(SRC) -o $(OUT) $(LIBS)

run: all
	./$(OUT)

clean:
	rm -rf build

.PHONY: all run clean