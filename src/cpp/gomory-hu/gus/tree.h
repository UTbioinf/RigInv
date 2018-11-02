
#ifndef TREE_H
#define TREE_H

#include "graph.h"
#include <assert.h> 


/******************* treearc data type ******************/
 
typedef struct treearcSt
{
  weight_t CUTFIELD;              /* the capacity value */
  struct treenodeSt *head;       /* the head of the arc */
  struct treenodeSt *tail;       /* the tail of the arc */
  struct treearcSt *rev;            /* the reverse edge */
  struct treearcSt *next;     /* next edge of same node */
  struct treearcSt *prev;     /* prev edge of same node */
}
treearc;
 
/******************* outtree data type ******************/
/* the following data type is used by treenode, because
   some treenodes may point out to more than one nodes  */
 
typedef struct outtreeSt
{
  node *outnode;
  struct outtreeSt *next;
  struct outtreeSt *prev;
}
outtree;

/******************* treenode data type *****************/
 
typedef struct treenodeSt
{
  int index;                /* first outgoing edge */
  int parent;               /* last outgoing edge */
  weight_t fl;              /* the capacity value */
}
treenode;


/******************* treefield data type *****************/
 
typedef struct treefieldSt
{
  int node_index;                /* first outgoing edge */
  int alias_index;               /* first outgoing edge */
  int parent;                    /* last outgoing edge */
  weight_t fl;                   /* the capacity value */
}
treefield;

/******************* tree data type *********************/
 
typedef struct treeSt
{
 
   treenode *root;   /* The root of the tree. */
   int n;        /* The number of nodes in tree */
 
   treenode *vertices;  /* All the nodes of the tree */
   treenode *freenode;  /* Pointer to the next empty node */
 
   treearc *edges;      /* The array of edges */
   treearc *freearc;   /* Pointer to the next empty node */
}
tree;



/************************* prototypes *******************/

treefield *createTree (int n);
void printTree(treefield *t, int n);
void calcCut (treefield *t, int n);


#endif



