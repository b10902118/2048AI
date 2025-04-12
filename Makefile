all:
	g++ -shared -std=c++11 -fPIC -I/home/bill/micromamba/envs/hw2/include/python3.8 -I/home/bill/micromamba/envs/hw2/lib/python3.8/site-packages/pybind11/include config.cpp bitboard.cpp -o board.cpython-38-x86_64-linux-gnu.so

#all: config.o bitboard.o main.o
#	g++ -Ofast config.o bitboard.o main.o -o main

main.o: main.cpp pattern.h bitboard.h
	g++ -Ofast -c main.cpp -o main.o

bitboard.o: bitboard.cpp bitboard.h
	g++ -Ofast -c bitboard.cpp -o bitboard.o

config.o: config.cpp config.h
	g++ -Ofast -c config.cpp -o config.o

clean:
	rm main *.o
