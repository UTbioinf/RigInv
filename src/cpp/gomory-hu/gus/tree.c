
#include "tree.h"


/************************ definitions ********************/


/*** Function createTree creates a Tree given the problem
instance graph g. The tree initially is a star having the
first node in the center. The root is set to that node. ***/

treefield *createTree (int n)
{

  treefield *t;
  int i;

  memassert(t = (treefield *)calloc(n, sizeof(treefield)));


  for (i=0; i<n; i++)
  {
    t[i].node_index = i;
    t[i].alias_index = i;
    t[i].parent = 0;
    t[i].fl = MAXWEIGHT;
  }

  return t;

}



void printTree (treefield *tree, int n)
{

  int i;

  printf("p cut %d %d\n", n, n-1);

  for (i=0; i<n; i++)
  {

    if (tree[i].parent != tree[i].node_index)
      printf("a %d %d %d\n", tree[i].node_index +1, tree[i].parent +1,
      (int)tree[i].fl); 

  }
}


/*************************************************************************/

/* calcCut prints out the sum of the cuts of the mincut tree */

void calcCut (treefield *t, int n)
{

  int i, sum;

  sum = 0;

  for (i=0; i<n; i++)
  {
    if (t[i].parent != t[i].node_index)
      sum += (int)t[i].fl;
  }

  printf("Cut-values Sum: %d\n", sum);
 
}
 


