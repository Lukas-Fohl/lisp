CC = g++
FLAGS = -Wall -Wextra -pedantic -std=c++20
SRC = src/*.cpp
DEST = build/main
TEST_FILE = example/ass.clisp

build:
	if [ ! -d "build" ]; then  mkdir ../build/; fi
	$(CC) $(SRC) -o $(DEST) $(FLAGS)

run: build
	./$(DEST) $(TEST_FILE)
