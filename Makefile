## 

SRCS=parallel.cpp serial.cpp dna_util.cpp
OBJS=$(SRCS:.cc=.o)

## OPT_FLAG = -O2

CPPFLAGS=-std=gnu++0x -Wall -g -fopenmp
LDFLAGS=-std=gnu++0x 

CC=mpicc
CXX=mpicxx

.SUFFIXES: .cc .o
.cc.o:
	$(CXX) $(CPPFLAGS) -c -o $@ $<

default: main
all: gen main

help:
	@echo "make           => compile everything"
	@echo "make clean     => clean compiled objs and executables"

main: main.o $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS) -lm

gen: gen.o
	$(CXX) -o $@ $^ $(LDFLAGS) -lm
	./gen entire_dna.txt 40000000

clean:
	/bin/rm -f *.o main gen

test:
	mpirun -np 3 main input_dna.txt entire_dna.txt

