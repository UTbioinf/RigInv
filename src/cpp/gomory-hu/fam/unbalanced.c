/* min cut problem generator 
   creates a tree with heavy edges and "random noise" of light edges
*/
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <values.h>

#include "random.h"

#define RANGE 100
#define DASH '-'
#define LIGHTCAP Random(l,u)
#define HEAVYCAP Random(L,U)
#define CAPACITY(i,j) (( color[i] == color[j] ) ? \
Random(L,U) : Random(l,u))

void error (int error_no);

void error (int error_no)
{
  switch ( error_no ) {

  case 1: {
    fprintf ( stderr, "\nUsage: treegen n d k P [-sS]\n");
    fprintf ( stderr, "where n is the number of nodes\n");
    fprintf ( stderr, "      b is the balance (int > 1)\n");
    fprintf ( stderr, "      S is a seed\n");
    break;
  }

  case 2: {
    fprintf ( stderr, "\nError: number_of_nodes must be int > 1\n");
    break;
  }

  case 3: {
    fprintf ( stderr, "\nError: density out of range\n");
    break;
  }

  case 4: {
    fprintf ( stderr, "\nError: decomposition number k out of range\n");
    break;
  }

  }

  exit(error_no);
}

void createTree(long lower, long upper, long size, long prop, long depth, long n)
{

  int leftsize, rightsize;

  leftsize = ((prop - 1)*size)/prop;
/*
  printf("Left side: %d\n", leftsize);
*/
  rightsize = size - leftsize - 1;
/*
  printf("Right side: %d\n", rightsize);
*/

  if (lower + 1 <= upper)
    printf ("a %ld %ld %ld\n", lower, lower + 1, depth + 1); 
  if (lower + leftsize + 1 <= upper)
    printf ("a %ld %ld %ld\n", lower, lower + leftsize + 1, depth + 1); 

  if ((lower + 1 <= upper) && (leftsize > 0))
    createTree(lower + 1, lower + leftsize, leftsize, prop, depth + 1, n);
  if ((lower + leftsize + 1 <= upper) && (rightsize > 0))
    createTree(lower + leftsize + 1, upper, rightsize, prop, depth + 1, n);
}


void main ( int argc, char* argv[])
{

  char   args[30];
  long   n, m;
  long   b;
  long   t;
  long   i, j;
  long   seed;
  long   P, k;
  double d;
  long   *color;
  long u, l, U, L;

  if (( argc < 3 ) || ( argc > 4 )) error (1);

  strcpy ( args, argv[1] );

  /* first parameter - number of nodes */
  if (( n = atoi ( argv[1] ) )  <  2  ) error (2);
 
  /* second parameter - balance */
  b = atof ( argv[2] );

  /* optional parameters */
  /* set default values */
  seed = 214365;

  if ( argc == 4 ) {
    strcpy ( args, argv[3]);
    if (( args[0] != DASH ) || ( args[1] != 's')) error (1);
    seed  =  atoi ( &args[2] );
  }

  SetRandom(seed);

  printf ("c UNBALANCED min-cut problem\n");
  printf ("p cut %8ld %8ld\n", n, n-1 );

  createTree(1, n, n, b, 0, n);

  exit (0);
}



