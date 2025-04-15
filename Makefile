CXXFLAGS := -Ofast -c
#-fsanitize=address -static-libasan -g
LDFLAGS := -Ofast
# -fsanitize=address -static-libasan -g

all: config.o bitboard.o main.o
	g++ $(LDFLAGS) config.o bitboard.o main.o -o main

main.o: main.cpp pattern.h bitboard.h
	g++ $(CXXFLAGS) main.cpp -o main.o

bitboard.o: bitboard.cpp bitboard.h
	g++ $(CXXFLAGS) bitboard.cpp -o bitboard.o

config.o: config.cpp config.h
	g++ $(CXXFLAGS) config.cpp -o config.o

clean:
	rm main *.o


