all: config.o bitboard.o main.o
	g++ -Ofast config.o bitboard.o main.o -o main

main.o: main.cpp pattern.h bitboard.h
	g++ -Ofast -c main.cpp -o main.o

bitboard.o: bitboard.cpp bitboard.h
	g++ -Ofast -c bitboard.cpp -o bitboard.o

config.o: config.cpp config.h
	g++ -Ofast -c config.cpp -o config.o