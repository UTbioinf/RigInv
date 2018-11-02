/* min cut problem generator 
   creates a path with heavy edges and "random noise" of light edges
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
    fprintf ( stderr, "\nUsage: pathgen n d k P [-sS]\n");
    fprintf ( stderr, "where n is the number of nodes\n");
    fprintf ( stderr, "      d is the density (%%)\n");
    fprintf ( stderr, "      k is the number of central nodes\n");
    fprintf ( stderr, "      P is the component arc capacity\n");
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

void main ( int argc, char* argv[])
{

  char   args[30];
  long   n, m;
  long   t;
  long   i, j;
  long   seed;
  long   P, k;
  double d;
  long   *color;
  long u, l, U, L;

  if (( argc < 5 ) || ( argc > 6 )) error (1);

  strcpy ( args, argv[1] );

  /* first parameter - number of nodes */
  if (( n = atoi ( argv[1] ) )  <  2  ) error (2);
 
  /* second parameter - density */
  d = atof ( argv[2] );
  m = (long) ((double) n * ((double) n - 1.0 ) * d / 200.0);
  m = m + n;
/*
  if (( m <= n ) || ( m > ( n * ( n - 1 ) / 2 )))
    error (3);
*/

  /* third parameter - decomposition number */
  /* defines the number of "central nodes" in the path:
     1 -  makes the path to be a star
     n-1 - makes the path to be uniformly random */
  k = atoi ( argv[3] );
  if (( k < 1 ) || ( k > n ))
    error (4);
 
  /* fourth parameter - component arc capacity */
  P = atoi ( argv[4] );
  l = 1;
  L = 1;
  u = RANGE;
  U = RANGE * P;

  /* optional parameters */
  /* set default values */
  seed = 214365;

  if ( argc == 6 ) {
    strcpy ( args, argv[5]);
    if (( args[0] != DASH ) || ( args[1] != 's')) error (1);
    seed  =  atoi ( &args[2] );
  }

  SetRandom(seed);

  /* set colors */
  color = (long *) calloc (n+1, sizeof (long) );
  for ( i = 1; i <= n; i++ )
    color[i] = Random ( 1, k );

  printf ("c PATHGEN min-cut problem\n");
  printf ("p cut %8ld %8ld\n", n, m );

  /* generate path */
  for ( i = 2; i <= n; i++ ) {
    if ((i-1) <= k)
      printf ("a %ld %ld %ld\n", i, i-1, HEAVYCAP); 
    else
    {
      j = Random (1, k);
      printf ("a %ld %ld %ld\n", i, j, HEAVYCAP); 
    }
  }

  /* generate remaining arcs */
  for ( t = n; t <= m; t++ ) {
    do {
      i = Random ( 1, n );
      j = Random ( 1, n );
    } while ( i == j );
    
    printf ("a %ld %ld %ld\n", i, j, LIGHTCAP);
  }

  exit (0);
}



