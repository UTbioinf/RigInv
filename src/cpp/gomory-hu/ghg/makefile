
CC = gcc
# uncomment -DBROKEN_HDRS is missing getopt.h
#CFLAGS = -pg -g -Wall -DSAVECUT #-DBROKEN_HDRS
CFLAGS = -O4 -Wall -DNDEBUG -DSAVECUT #-DBROKEN_HDRS
#LDFLAGS = -pg
LDFLAGS = -O4 
LOADLIBES = -lm


PROGS=ho

all: progs

clean:
	rm -f $(PROGS) *.o

# utilities
random.o: random.c random.h
	$(CC) $(CFLAGS) -c -o random.o random.c

fprintfll.o: fprintfll.c fprintfll.h
	$(CC) $(CFLAGS) -c -o fprintfll.o fprintfll.c

timer.o: timer.c timer.h
	$(CC) $(CFLAGS) -c -o timer.o timer.c

tree.o: graph.h tree.c tree.h
	$(CC) $(CFLAGS) -DHO -DNO_PR -c -o tree.o tree.c
 
compact.o: graph.h compact.c compact.h
	$(CC) $(CFLAGS) -DHO -DNO_PR -c -o compact.o compact.c
 
graph_HO.o: graph.c graph.h memassert.h fprintfll.h timer.h 
	$(CC) $(CFLAGS) -DHO -DNO_PR -c graph.c -o graph_HO.o

# programs
progs: $(PROGS)

###### HO

ho: ho.o random.o timer.o compact.o tree.o graph_HO.o fprintfll.o 
	$(CC) $(LDFLAGS) -DNO_PR ho.o random.o timer.o compact.o tree.o graph_HO.o fprintfll.o $(LOADLIBES) -o ho

ho.o: ho.c graph.h memassert.h fprintfll.h timer.h ho.h qs.h  
	$(CC) $(CFLAGS) -c -DNO_PR ho.c -o ho.o

