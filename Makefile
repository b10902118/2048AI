CXX=clang++
CXXFLAGS := -Ofast -c -g
LDFLAGS := -Ofast -g

all: config.o bitboard.o main.o
	$(CXX) $(LDFLAGS) config.o bitboard.o main.o -o main

main.o: main.cpp pattern.h bitboard.h
	$(CXX) $(CXXFLAGS) main.cpp -o main.o

bitboard.o: bitboard.cpp bitboard.h
	$(CXX) $(CXXFLAGS) bitboard.cpp -o bitboard.o

config.o: config.cpp config.h
	$(CXX) $(CXXFLAGS) config.cpp -o config.o

clean:
	rm main *.o


