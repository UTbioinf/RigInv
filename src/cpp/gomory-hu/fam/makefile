# -*-makefile-*- for programs and generators
# $Id: makefile,v 1.18 1997/05/16 15:30:32 mslevine Exp $
######################################

CC = gcc
# uncomment -DBROKEN_HDRS is missing getopt.h
CFLAGS = -g -Wall -DSAVECUT #-DBROKEN_HDRS
#CFLAGS = -O4 -Wall -DNDEBUG -DSAVECUT #-DBROKEN_HDRS
LDFLAGS = -O4 
LOADLIBES = -lm

# place to copy binaries to
BINDIR=../bin

GENS=cyclegen noigen treegen unbalanced pathgen randomgen regulargen wheelgen \
     bikewheelgen prgen irregulargen dblcyclegen

all: gens

clean:
	rm -f $(GENS) *.o

# utilities
random.o: random.c random.h
	$(CC) $(CFLAGS) -c -o random.o random.c

# problem generators
gens: $(GENS)

sgbgen: sgbgen.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(SGBINC) -o sgbgen  sgbgen.c $(LOADLIBES) $(SGBLIB) -lgb

noigen: noigen.c random.o
	$(CC) $(CFLAGS) -o noigen  noigen.c random.o

treegen: treegen.c random.o
	$(CC) $(CFLAGS) -o treegen  treegen.c random.o

unbalanced: unbalanced.c random.o
	$(CC) $(CFLAGS) -o unbalanced unbalanced.c random.o

pathgen: pathgen.c random.o
	$(CC) $(CFLAGS) -o pathgen  pathgen.c random.o

cyclegen: cyclegen.c random.o
	$(CC) $(CFLAGS) -o cyclegen  cyclegen.c random.o

randomgen: randomgen.c random.o
	$(CC) $(CFLAGS) -o randomgen  randomgen.c random.o

regulargen: regulargen.c random.o
	$(CC) $(CFLAGS) -o regulargen  regulargen.c random.o

irregulargen: irregulargen.c random.o
	$(CC) $(CFLAGS) -o irregulargen  irregulargen.c random.o

wheelgen: wheelgen.c random.o
	$(CC) $(CFLAGS) -o wheelgen  wheelgen.c random.o

bikewheelgen: bikewheelgen.c random.o
	$(CC) $(CFLAGS) -o bikewheelgen  bikewheelgen.c random.o

prgen: prgen.c random.o
	$(CC) $(CFLAGS) -o prgen  prgen.c random.o

dblcyclegen: dblcyclegen.c random.o
	$(CC) $(CFLAGS) -o dblcyclegen  dblcyclegen.c random.o


