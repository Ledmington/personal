CXX=g++
CXXFLAGS=-std=c++17 -Wall -Wextra -Wpedantic -Werror -Wshadow -fno-exceptions -fno-rtti -fopenmp
DEBUG_FLAGS=-O0 -g
OPTFLAGS=-O3 -DNDEBUG -march=native -mtune=native
EXE_NAME=escaping_the_black_hole.x

all: debug

debug:
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) main.cpp -o $(EXE_NAME)

release:
	$(CXX) $(CXXFLAGS) $(OPTFLAGS) main.cpp -o $(EXE_NAME)

format:
	clang-format --style=file -i *.hpp *.cpp

clean:
	rm -f $(EXE_NAME)