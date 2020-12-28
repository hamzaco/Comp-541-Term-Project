CC=gcc
CXX=g++
RM=rm -f

all: dddriver

dddriver: dddriver.o
	g++ -o dddriver dddriver.o -L. -ldds -fopenmp 

dddriver.o: dddriver.cpp
	g++ -c -std=c++11 dddriver.cpp -fopenmp 

clean:
	$(RM) dddriver dddriver.o