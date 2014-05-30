CXXFLAGS=-std=c++11
LIBS=-lboost_system

default: all

main.o: main.cpp
	$(CXX) -o $@ -c $< $(CXXFLAGS)

zeroize.o: zeroize.c
	$(CXX) -o $@ -c $< $(CXXFLAGS)

sha256.o: sha256.c
	$(CXX) -o $@ -c $< $(CXXFLAGS)

hashchains: main.o zeroize.o sha256.o
	$(CXX) -o $@ main.o zeroize.o sha256.o $(LIBS)

all: hashchains

clean:
	rm -f hashchains
	rm -f *.o

