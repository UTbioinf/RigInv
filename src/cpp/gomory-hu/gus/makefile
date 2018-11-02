
CC = gcc
# uncomment -DBROKEN_HDRS is missing getopt.h
#CFLAGS = -g -Wall -DSAVECUT #-DBROKEN_HDRS
CFLAGS = -O4 -Wall -DNDEBUG -DSAVECUT #-DBROKEN_HDRS
LDFLAGS = -O4 
LOADLIBES = -lm


PROGS=gus

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

graph_HO_nopr.o: graph.c graph.h memassert.h fprintfll.h timer.h 
	$(CC) $(CFLAGS) -DHO -DNO_PR -c graph.c -o graph_HO_nopr.o

# programs
progs: $(PROGS)

###### GUS

gus: ho_nopr.o random.o timer.o tree.o graph_HO_nopr.o fprintfll.o
	$(CC) $(LDFLAGS)  ho_nopr.o random.o timer.o tree.o graph_HO_nopr.o fprintfll.o $(LOADLIBES) -o gus

ho_nopr.o: ho.c graph.h memassert.h fprintfll.h timer.h ho.h qs.h  
	$(CC) $(CFLAGS) -DNO_PR -c ho.c -o ho_nopr.o

