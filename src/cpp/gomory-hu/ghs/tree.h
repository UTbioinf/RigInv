
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
  treearc *first;                /* first outgoing edge */
  treearc *last;                  /* last outgoing edge */
  node /* maybe later: outtree */ *outtree;       /* connection to the input graph */
  int name;              /* corrosponding vertex number */
  int depthmaxmax;  /* stores the max depth of the node */
  int depthminmax;  /* stores the 2nd max depth of the node */
}
treenode;


/******************* tree data type *********************/

typedef struct treeSt
{

   treenode *root;   /* The root of the tree. We will later(?)
                 maintain a leaf to be the root. */

   int n;        /* The number of nodes in tree */

   treenode *vertices;  /* All the nodes of the tree */
   treenode *freenode;  /* Pointer to the next empty node */

   treearc *edges;      /* The array of edges */
   treearc *freearc;   /* Pointer to the next empty node */
}
tree;



/************************* prototypes *******************/

tree *createTree (int n);
int findEdges(graph *g, node *nextnode);
int findNodes(graph *g, node *nextnode);
void expandTree (tree *t, node *head_node_list1, node *head_node_list2,
                 node *tail_node_list1, node *tail_node_list2, weight_t mincutvalue);
void backTree (graph *g, node *v);
void traverseTree(tree *t, treenode *startNode, treenode *traverseNode,
                         weight_t mincut);
void writeTree (tree *t, int treesize);
void scanTree (treenode *treeNode, treenode *parentNode);
void calcCut (tree *t);
void computeTree (tree *t);
int depthTree (treenode *treeNode, treenode *parentNode);
int findmaxTree(treenode *treeNode, treenode *parentNode, int *numleaves);

void scanCut (treenode *treeNode, treenode *parentNode, int *sum);

#endif



