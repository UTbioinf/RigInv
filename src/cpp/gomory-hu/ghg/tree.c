
#include "tree.h"


/************************ definitions ********************/

/*** Function findNodes counts the total number of edges in the subgraph.
It is used for statistical reasons */
 
int findNodes(graph *g, node *nextnode)
 
{
 
  int nodenum = 0;
 
  while ((nextnode != g->sentinelVertex) && (nextnode != NULL))
  {
    nodenum++;
    nextnode = nextnode->next;
  }
 
  return nodenum;
}
 
 
/*** Function findEdges counts the total number of edges in the subgraph.
It is used for statistical reasons */
 
int findEdges(graph *g, node *nextnode)
{
 
  int edgenum = 0;
  arc *nextarc;
 
  while ((nextnode != g->sentinelVertex) && (nextnode != NULL))
  {
    /* there should be at least one edge */
    for (nextarc = nextnode->first; nextarc != NULL; nextarc = nextarc->next)
      edgenum++;
    edgenum++; /* for the last edge */
    nextnode = nextnode->next;
  }
 
  return edgenum;
}



/*** Function createTree creates a Tree given the problem
instance graph g. The tree initially is a star having the
first node in the center. The root is set to that node. ***/

tree *createTree (int n)
{

  tree *t;
  int i;

  memassert(t = (tree *)calloc(1, sizeof(tree)));

  t->n = n;

  memassert(t->vertices = (treenode *)calloc(n, sizeof(treenode)));
  memassert(t->edges = (treearc *)calloc(2*(n-1), sizeof(treearc)));

  for (i=0; i<n; i++)
  {
    t->vertices[i].first = NULL;
    t->vertices[i].last = NULL;
    t->vertices[i].outtree = NULL;
    t->vertices[i].depthmaxmax = 0;
    t->vertices[i].depthminmax = 0;
  }
  for (i=0; i<2*(n-1); i++)
  {
    t->edges[i].CUTFIELD = MAXWEIGHT;
    t->edges[i].tail = NULL;
    t->edges[i].rev = NULL;
    t->edges[i].next = NULL;
  }

  /* set the root: */

  t->root = t->vertices;
  t->freenode = t->vertices + 1;
  t->freearc = t->edges;
  t->root->first = NULL;
  t->root->last = NULL;

  return t;

}


/*************************************************************************/

/* expandTree is always called after splitgraph */
/* it adds one more node to the existing tree   */

void expandTree (tree *t, node *head_node_list1, node *head_node_list2,
                 node *tail_node_list1, node *tail_node_list2, weight_t mincutvalue)
{
  treenode *tn1, *tn2;
  treearc *temparc, *next_arc;
  node *next_node;


  assert (tail_node_list1->intree == NULL);          /* v is a left child */


  if (tail_node_list1->up->intree == NULL)
  {
    tail_node_list1->intree = t->root;
  }
  else
  {
    tail_node_list1->intree = tail_node_list1->up->intree;
  }
  tn1 = tail_node_list1->intree;

  tail_node_list1->up->downright->intree = t->freenode++;
  tn2 = tail_node_list1->up->downright->intree;

  /* the following loop makes the contracted nodes of list2 to point to tn2 */

  for ( next_node = head_node_list2;
        next_node != tail_node_list2;
        next_node = next_node->next )
  {
    if ( next_node->contracted == 1)
      next_node->intree = tn2;
  }


  /* the following loop moves adjacent nodes from tn1 to tn2 */


  for ( next_node = head_node_list2;
        next_node != tail_node_list2;
        next_node = next_node->next )
  {

    for ( next_arc = tn1->first; next_arc != NULL; next_arc = next_arc->next )
      if (next_node->up != NULL)

      if (((next_node == next_node->up->downleft) &&
                     ( next_arc->tail == next_node->up->downright->intree ))
         ||
         ((next_node == next_node->up->downright) &&
                     ( next_arc->tail == next_node->up->downleft->intree )))

      {

        if ((next_arc->prev != NULL) && (next_arc->next != NULL))
        {
          next_arc->next->prev = next_arc->prev;
          next_arc->prev->next = next_arc->next;
        }
        else if ((next_arc->prev == NULL) && (next_arc->next != NULL))
        {
          tn1->first = next_arc->next;
          next_arc->next->prev = NULL;
        }
        else if ((next_arc->prev != NULL) && (next_arc->next == NULL))
        {
          tn1->last = next_arc->prev;
          next_arc->prev->next = NULL;
        }
        else /* next_arc->prev = next_arc->next = NULL) */
        {
          tn1->first = NULL;
          tn1->last = NULL;
        }


        next_arc->rev->tail = tn2;

        if (tn2->first == NULL)
        {
          tn2->first = next_arc;
          tn2->last = tn2->first;
          next_arc->prev = NULL;
          next_arc->next = NULL;
        }
        else    /* put at head of list */
        {
          tn2->first->prev = next_arc; 
          next_arc->next = tn2->first;
          next_arc->prev = NULL;
          tn2->first = next_arc;
        }

      }
  }


  temparc = tn1->last;
  tn1->last = t->freearc++;
  if (temparc == NULL)
    tn1->first = tn1->last;
  else
  {
    temparc->next = tn1->last;
    tn1->last->prev = temparc;
  }


  if (tn2->last == NULL)
    tn2->last = t->freearc++;
  else
  {
    tn2->last->next = t->freearc++;
    tn2->last->next->prev = tn2->last;
    tn2->last = tn2->last->next;
  }

  if (tn2->first == NULL)
    tn2->first = tn2->last;

  tn1->last->CUTFIELD = mincutvalue;
  tn1->last->tail = tn2;
  tn1->last->rev = tn2->last;
  tn1->last->next = NULL;
  tn2->last->CUTFIELD = mincutvalue;
  tn2->last->tail = tn1;
  tn2->last->rev = tn1->last;
  tn2->last->next = NULL;

}


/*************************************************************************/

/* backTree puts the name of the vertex in g corresponding to the node in t */


void backTree (graph *g, node *v)
{

  node *nextnode;

  for (nextnode = v->firstN; nextnode != g->sentinelVertex; nextnode = nextnode->next)
  {
    if (nextnode->contracted == 0)
    {
      v->intree->name = nextnode->name;
      return;
    }
  }

  assert(0); /* it shouldn't reach this point */
}


/*************************************************************************/

/* alterTree moves a leaf from one place to another.
   only leaves are moved, and they remain leaves... */


void alterTree (tree *t, treenode *moveNode, treenode *fromNode, treenode *toNode)
{

  treearc *searchArc;
  treearc *revArc;
  treenode *tempNode;

  for (searchArc = moveNode->first;
       (searchArc != NULL) && (searchArc->tail != fromNode);
       searchArc = searchArc->next)
  ;

  if (searchArc == NULL)
    fprintf(stderr, "error\n");
  else /* found arc: moveNode <-> fromNode,
          I think this should happen always with the first trial */
  {
    /* update pointers, put edge moveNode <-> toNode at the
       beginning of the list of moveNode
       and at the end of the list of toNode */
    
    revArc = searchArc->rev;

    /* searchArc->CUTFIELD = revArc->CUTFIELD = cut; */

    /* fix edges adjacent to revArc: */
    if ((revArc->prev == NULL) && (revArc->next == NULL))
    ; /* do nothing */
    else if (revArc->prev == revArc->next)
    {
      tempNode = revArc->prev->head;
      tempNode->first = revArc->prev;
      tempNode->last = revArc->prev;
      revArc->prev->next = NULL;
      revArc->prev->prev = NULL;
    }
    else if (revArc->prev == NULL)
    {
      tempNode = revArc->head;
      tempNode->first = revArc->next;
      revArc->next->prev = NULL;
    }
    else if (revArc->next == NULL)
    {
      tempNode = revArc->head;
      tempNode->last = revArc->prev;
      revArc->prev->next = NULL;
    }
    else /* revArc in the middle of a list */
    {
      revArc->prev->next = revArc->next;
      revArc->next->prev = revArc->prev;
    }

    /* fix revArc: (we assume fromNode and toNode
                    have at least another edge, which must be true) */
    revArc->head = toNode;                /* it was set to fromNode */
    toNode->last->next = revArc;
    revArc->prev = toNode->last;
    revArc->next = NULL;
    toNode->last = revArc;



    /*fix searchArc: */ 

    searchArc->tail = toNode;               /* it was set to fromNode */

  }
}




void fixTree (tree *t, graph *g)
{
  treearc *traverseEdge;
  treearc *traverseEdgenext;
  treenode *treesource;
  treenode *treesink;
  unsigned int source_in_cut;

#ifdef HO
  treesource = g->source->intree;
  treesink = g->sink->intree;
  source_in_cut = g->source->in_cut;
#else
  fprintf(stderr, "HO is not defined!!!\n");
#endif

  traverseEdge = treesink->first;
  traverseEdgenext = treesink->first->next;

  while (traverseEdge != NULL)
  {
    if (traverseEdge->tail == treesource)
      traverseEdge->CUTFIELD = traverseEdge->rev->CUTFIELD = g->minCap;

    else if ((traverseEdge->tail > treesource) &&
        (traverseEdge->tail->outtree->in_cut == source_in_cut))
      alterTree (t, traverseEdge->tail, treesink, treesource);

    /* We have to keep traverseEdge->next in a separate variable,
       because traverseEdge gets removed from treesink's list
       in alterTree and then traverseEdge->next is wrong */

    traverseEdge = traverseEdgenext;
    if (traverseEdge == NULL)
      traverseEdgenext = NULL; /* this will never be used */
    else
      traverseEdgenext = traverseEdge->next;
  }
}


/*************************************************************************/

/* printTree prints for all pairs of nodes the mincut */


void printTree (tree *t)
{
    traverseTree(t, t->root, t->root, MAXWEIGHT);
}



void traverseTree(tree *t, treenode *startNode, treenode *traverseNode, weight_t mincut)
{

  treearc *nextEdge;

  for (nextEdge = traverseNode->first; nextEdge != NULL; nextEdge = nextEdge->next)
  {
    if (nextEdge->rev->tail < nextEdge->tail)
    {
      if (nextEdge->CUTFIELD < mincut)
        traverseTree(t, startNode, nextEdge->tail, nextEdge->CUTFIELD);
      else
        traverseTree(t, startNode, nextEdge->tail, mincut); 
    }
  }

  if (startNode < traverseNode)
    printf("Mincut for %d - %d is %d\n", startNode->name, traverseNode->name,
            (int) mincut); 
}


/*************************************************************************/
 
/* writeTree appends the tree into file 'tree' in DIMACS format */ 
 

void writeTree (tree *t, int treesize)
{

  printf("p cut %d %d\n", treesize, treesize-1);
  scanTree(t->root, t->root);

}

void scanTree (treenode *treeNode, treenode *parentNode)
{
  treearc *nextEdge;

  for (nextEdge = treeNode->first; nextEdge != NULL; nextEdge = nextEdge->next)
  {
    if (nextEdge->tail != parentNode)
    {
      printf("a %d %d %d\n", treeNode->name, nextEdge->tail->name,
             (int)nextEdge->CUTFIELD);
      scanTree(nextEdge->tail, treeNode);
    }
  }
}

void writeSimpleTree(FILE* fp, tree* t, int treesize)
{
    fprintf(fp, "%d\n", treesize - 1);
    scanSaveTree(fp, t->root, t->root);
}

void scanSaveTree (FILE* fp, treenode *treeNode, treenode *parentNode)
{
  treearc *nextEdge;

  for (nextEdge = treeNode->first; nextEdge != NULL; nextEdge = nextEdge->next)
  {
    if (nextEdge->tail != parentNode)
    {
      fprintf(fp, "%d %d %d\n", treeNode->name, nextEdge->tail->name,
             (int)nextEdge->CUTFIELD);
      scanSaveTree(fp, nextEdge->tail, treeNode);
    }
  }
}



/*************************************************************************/

/* calcCut prints out the sum of the cuts of the mincut tree */

void calcCut (tree *t)
{

  int sum[1];
 
  sum[0] = 0;

  scanCut(t->root, t->root, sum);

  printf("Cut-values Sum: %d\n", *sum);
 
}
 
void scanCut (treenode *treeNode, treenode *parentNode, int *sum)
{
  treearc *nextEdge;
 
  for (nextEdge = treeNode->first; nextEdge != NULL; nextEdge = nextEdge->next)
  {
    if (nextEdge->tail != parentNode)
    {
      *sum += (int)nextEdge->CUTFIELD;
      scanCut(nextEdge->tail, treeNode, sum);
    }
  }
}



void computeTree (tree *t)
{

  int *numleaves = malloc(sizeof(int));

  *numleaves = 0;

  printf("root returned %d\n", depthTree(t->root, t->root));

  printf("root has %d and %d\n", t->root->depthmaxmax, t->root->depthminmax);

  printf("Maximum distance is: %d\n", findmaxTree(t->root, t->root, numleaves));

  printf("Total number of leaves: %d\n", *numleaves);
}


int depthTree (treenode *treeNode, treenode *parentNode)
{
  treearc *nextEdge;
  int depth_of_subtree;

  for (nextEdge = treeNode->first; nextEdge != NULL; nextEdge = nextEdge->next)
  {
    if (nextEdge->tail != parentNode)
    {
      depth_of_subtree = depthTree(nextEdge->tail, treeNode); 
      if (depth_of_subtree > treeNode->depthmaxmax)
      {
        treeNode->depthminmax = treeNode->depthmaxmax;
        treeNode->depthmaxmax = depth_of_subtree;
      }
      else if (depth_of_subtree > treeNode->depthminmax)
      {
        treeNode->depthminmax = depth_of_subtree;
      }
    }
  }

  return treeNode->depthmaxmax + 1;
}


int findmaxTree(treenode *treeNode, treenode *parentNode, int *numleaves)
{
  treearc *nextEdge;
  int max_dept_so_far = 0;

  if (treeNode->first->next == NULL)
    (*numleaves)++;

  for (nextEdge = treeNode->first; nextEdge != NULL; nextEdge = nextEdge->next)
  {
    if (nextEdge->tail != parentNode)
      max_dept_so_far = findmaxTree(nextEdge->tail, treeNode, numleaves);
  }
    if ((treeNode->depthminmax + treeNode->depthmaxmax) > max_dept_so_far)
      return (treeNode->depthminmax + treeNode->depthmaxmax);
    else
      return max_dept_so_far;
}








