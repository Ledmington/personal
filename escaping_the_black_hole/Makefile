CXX=g++
CXXFLAGS=-std=c++17 -Wall -Wextra -Wpedantic -Werror -Wshadow -fno-exceptions -fno-rtti -fopenmp
DEBUG_FLAGS=-O0 -g
OPTFLAGS=-O3 -DNDEBUG -march=native -mtune=native

all: debug

debug:
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) main.cpp -o escaping_the_black_hole.x

release:
	$(CXX) $(CXXFLAGS) $(OPTFLAGS) main.cpp -o escaping_the_black_hole.x
