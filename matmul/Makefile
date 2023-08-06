all: matmul.cpp.x matmul.class

matmul.cpp.x: matmul.cpp
	g++ -std=c++17 -Wall -Werror -Wextra -Wshadow -O3 -DNDEBUG -march=native -mtune=native matmul.cpp -o matmul.cpp.x

matmul.class: MatMul.java
	javac -Xlint:all -Xdiags:verbose -deprecation -source 17 -target 17 --add-modules jdk.incubator.vector -d . MatMul.java

clean:
	rm -f matmul.cpp.x matmul.class
