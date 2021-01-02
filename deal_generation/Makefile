CC=gcc
CXX=g++
RM=rm -f

all: dddriver

dddriver.o: dddriver.cpp
	g++ -c -std=c++11 dddriver.cpp -fopenmp 

dddriver: dddriver.o
	g++ -o dddriver dddriver.o -L. -ldds -fopenmp 

clean:
	$(RM) dddriver dddriver.o

run: 
	./dddriver deals.txt