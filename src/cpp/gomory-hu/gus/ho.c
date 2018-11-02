/* Hao-Orlin min cut algorithm 
 *
 * Reference: J. Hao and J. B. Orlin, "A Faster Algorithm for Finding
 * the Minimum Cut in a Directed Graph". Journal of Algorithms, vol
 * 17, pp 424-446, 1994.
 *
 * applied to
 * Gusfield's all-pairs min-cut algorithm
 *
 * Reference: D. Gusfield, "Very Simple Methods for All Pairs Network
 * Flow Analysis". SIAM Journal on Computing, vol 19, pp 143-155, 1990.
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __CHECKER__
#include "memassert.h"
#else
#include <assert.h>
#endif

#define HO

/************************** constants ********************************/

#define GLOB_UPDATE_FREQ 2.0
#define INT_PR_FREQ_12  18.0  /* this should be played with some more */
#define INT_PR_FREQ_34   9.0  /* this should be played with some more */

/************************** includes *******************************/

#include "graph.h"
#include "timer.h"
#include "ho.h"
#define HO_INTERNAL
#include "heap.h"
#include "tree.h"

/**************************** global variables **********************/

#ifdef PR_34
heap h;
#endif

/************************************** prototypes *******************/

static void insertActive(graph *g, node *j, bucket *l);
static void insertNode(node *i, bucket *l);
static void deleteNode(node *i, bucket *l);
static node *FirstRealActive(bucket *l);
static node *NextRealActive(node *i);

static void pushFlow(graph *g, arc *a, node *i, node *j, int jD,
		     weight_t delta);
static int relabel(graph *g, node *i);
static int discharge(graph *g, node *i);
static void saturateOutgoing(graph *g, node *w);

static void gap(graph *g, bucket *gapB);
static void globalUpdate(graph *g);
static void saveFlowCut(graph *g);
static int STCut(graph *g);

static void oneNodeLayer (graph *g, node *i);

static void initCutComp(graph *g, node *source, node *sink);

static void mainInit(graph *g);
static void mainCleanup(graph *g);
static void resetVertices(graph *g);
static void resetNodes(graph *g);
static void resetArcs(graph *g);
static void initContracted(graph *g);

static void copyArcs(graph *g);
static void moveVertices(graph *g);
static void moveVerticesBack(graph *g);

void computeCuts(graph *g);

/************************************** function definitions *********/

/* ================================================================ */

#ifdef CONTRACT
void fixFlowForContraction(graph *g, node *v, node *w)
{
  bucket *bb, *l;

  assert(v->status != FROZEN);
  assert(v->leader == v);
  assert(w->leader == w);
  assert(w->status != SOURCE);
  
  w->toContract = 1;
  
  if (v->status == SOURCE)
    saturateOutgoing(g, w);

  assert(w->leader == w);  

  if (g->currentN > 2) 
    {
      if (w->status == REGULAR)
	g->regN--;

      if ((w->status != FROZEN) && (w != g->sink)) 
	{
	  bb = g->buckets + w->d;
	  deleteNode(w,bb);
	}
      
      if (( v != g->sink) && (v != g->source) && 
	  (v->excess == 0) && (w->excess > 0)) 
	{
	  l = g->buckets + v->d;
	  insertActive(g,v,l);
	}
      
      v->excess += w->excess;
    }

  w->status = CONTRACTED;
}
#endif

/* ================================================================ */
/* insert node in bucket's active list */
static void insertActive(graph *g, node *j, bucket *l) 
{
  j->nextA = l->firstActive;
  l->firstActive  = j;
  if (j->d > g->aMax)
    g->aMax = j->d;
}

/* ================================================================ */
/* insert node in bucket */
static void insertNode(node *i, bucket *l)
{
  node *tmpNode; 

  tmpNode = l->first;
  i->bNext = tmpNode;
  if ( tmpNode != NULL ) tmpNode->bPrev = i;
  l->first = i;
}

/* ================================================================ */
/* delete node from bucket */
static void deleteNode(node *i, bucket *l)
{
  node *tmpNode;

  if ( (l)->first == (i) )
    { 
       (l)->first = (i)->bNext;
	if ((i)->bNext != NULL) (i)->bNext->bPrev = NULL; 
    } 
  else 
    {
      tmpNode = (i)->bNext;
      (i)->bPrev->bNext = tmpNode;
      if ( tmpNode != NULL ) tmpNode->bPrev = (i)->bPrev;
    }
}

/* ================================================================ */
/* finds the first non-frozen active node */
static node *FirstRealActive(bucket *l)
{
  node *i;

  for (i = l->firstActive; i != NULL; i = i->nextA) 
    if (i->status == REGULAR)
      {
	assert(i->toContract == 0);
	l->firstActive = i;
	return i;
      }
  l->firstActive = NULL;
  return NULL;
}

/* ================================================================ */
/* finds the next non-frozen active node */
static node *NextRealActive(node *i)
{
  node *j;

  for (j = i->nextA; j != NULL; j = j-> nextA) 
    if (j->status == REGULAR)
      {
	assert(j->toContract == 0);
	return j;
      }
  return NULL;
}


/* ================================================================ */
/* push flow and take care of bucket updates */

static void pushFlow(graph *g, arc *a, node *i, node *j, int jD,
		     weight_t delta)
{
  a->resCap -= delta;
  Reverse(a)->resCap += delta;

  if ( j != g->sink && j->excess == 0 )  
    insertActive (g, j, g->buckets+jD );
  
  j->excess += delta;
  i->excess -= delta;
}

/* ================================================================ */
/*--- relabelling node i */
/* assumes i->excess > 0 */
static int relabel(graph *g, node *i)
{
  node  *j;
  int  minD;       /* minimum d of a node reachable from i */
  arc   *minA=NULL; /* an arc which leads to the node with minimal d */
  arc   *a;
  bucket *l;        /* bucket containing node i */

  g->relabelCnt++; 
  assert(i->status == REGULAR);

  l = g->buckets + i->d;
  deleteNode ( i, l );

  minD = g->sourceD;

  /* find the minimum */
  ForAllIncidentArcs(i,a) 
    {
      g->edgeScanCnt[MAIN]++;
      if ( a->resCap > 0 ) 
	{
	  j = a->head;
	  if ((j->status == REGULAR ) && ( j->d < minD )) 
	    {
	      minD = j->d;
	      minA = a;
	    }
	}
    }
  
  minD++;
  
  if ( minD < g->sourceD ) 
    {
      i->d = minD;
      i->current = minA;
      
      l = g->buckets + minD;
      
      insertNode(i, l);
      if ( minD > g->dMax ) g->dMax = minD;
      
      insertActive(g,i , l);
    }
  else
    oneNodeLayer(g, i);

  return(minD);

} /* end of relabel */

/* ================================================================ */
/* discharge:  push flow out of i until i becomes inactive or i is relabeled */
static int discharge (graph *g, node  *i)
{
  node  *j;                /* successor of i */
  int  jD;                /* d of the next bucket */
  arc   *a;                /* current arc (i,j) */
  weight_t  delta;           /* flow to push through the arc */
  int flag;

  jD = (i->d) - 1;

  /* scanning arcs out of  i  */
  assert(Reverse(i->current)->head == i);
  a = i->current;
  while (a != NULL) {

    assert(Reverse(a)->head == i);
    flag = 0;

    g->edgeScanCnt[g->context]++;
    if ( a->resCap > 0 ) {
      j = a->head;

      if (( j->status == REGULAR ) && ( j->d == jD )) {
 	g->pushCnt ++; 
	delta = MIN ( i->excess, a->resCap );

	pushFlow (g, a, i, j, jD, delta );

	if (i->excess == 0) break;

      } /* j belongs to the next bucket */
    } /* a  is not saturated */

    a = a->next;
    
  } /* end of scanning arcs from  i */


  if (a == NULL) {
    return (1);
  }
  else {
#ifdef CONTRACT
    if (a->head != NULL)
      i->current = a;
    else { /* a was deleted */
      if (a->next != NULL)
	i->current = a->next;
      else
	i->current = a->prev; 
    }
#else
    i->current = a;   /* a never deleted if no contractions */
#endif
    return (0);
  }
}



/* ================================================================ */
/* saturateOutgoing is called only before contracting w
   into the source */
static void saturateOutgoing(graph *g, node * w)
{
  arc *a;
  weight_t delta;
  int jD;
  node *j;

  ForAllIncidentArcs ( w, a ) {
    g->edgeScanCnt[g->context]++;
    j = a->head;
    /* some nodes may get contracted by excess detection */
#ifdef EXCESS_DETECTION
    if (j == NULL) continue;
    if (j->toContract) continue;
#endif
    
    delta = a->resCap;
    if ( delta > 0 ) {
      switch ( j->status ) {
	
      case REGULAR: {
	jD = j->d;
	pushFlow (g, a, w, j, jD, delta );

/*
#ifdef EXCESS_DETECTION
	excessCheck(g, j);       
#endif
*/

	break;
      }
      case FROZEN: {
	a->resCap = 0;
	Reverse ( a )->resCap += delta;
	      
	w->excess -= delta;
	j->excess += delta;

/*
#ifdef EXCESS_DETECTION
	excessCheck(g, j);       
#endif
*/

	break;
      }
      case SOURCE: 
	{ 
	  break; 
	}
      } /* switch */
    }
  }
}

/* ================================================================ */
/* gap relabeling */
static void gap (graph *g, bucket *gapB)
{
  bucket *b;
  node  *i;
  int emptyLayer = 1;
  int  r;           /* index of the bucket before the gap */

  g->gapCnt++;

  r = ( gapB - g->buckets ) - 1;

  /* freeze nodes beyond the gap */

  if (( gapB + 1 )->first == NULL ) {
    i = gapB->first;
    gapB->firstActive = gapB->first = NULL;
    oneNodeLayer(g, i);
    emptyLayer = 0;
  }
  else  { 
    sPush ( NULL, g->freezer );
    for ( b = gapB; b <= g->buckets + g->dMax; b++ ) {
      for ( i = b->first; i != NULL; i = i->bNext ) {
	i->status = FROZEN;
	sPush ( i, g->freezer );
	emptyLayer = 0;
	g->regN--;
	g->gNodeCnt++;
      }
      b->firstActive = b->first = NULL;
    }
  }

  assert(!emptyLayer);

  g->dMax = r;
  g->aMax = r;

}
/* ================================================================ */


/* ================================================================ */
/* global update via backward breadth first search from the sink */
/* update time proportional to the number of arcs adjacent to regular nodes */

static void globalUpdate(graph *g)
{
  node  *i, *j;
  arc   *a;
  bucket *b, *bS;
  int  dist;
  int  fCnt = 0; /* count of freezed nodes */

  g->updateCnt++;
  g->relsSinceUpdate = 0;

  /* initialization */

  bS = g->buckets + g->dMax;
  for ( b = g->buckets + g->sinkD; b <= bS; b++ ) {
    for ( i = b->first; i != NULL; i = i->bNext ) {
      i->d += g->infD; /* this way we can reconstruct i->d */
      sPush ( i, g->regNodes );
    }
    b->first = b->firstActive = NULL;
  }

  g->dMax = g->aMax = g->sinkD;

  /* backward breadth first search from the sink */

  qEnqueue ( g->sink, g->BFSqueue );  
  g->sink->d = g->sinkD;
  do {
    qDequeue ( i, g->BFSqueue );

    dist = i->d;
    b = g->buckets + dist;
    insertNode( i, b );
    if ( dist > g->dMax ) g->dMax = dist;

    if (( i->excess  > 0 ) && ( i != g->sink )) {
      insertActive (g,i, b);
    }

    /* scanning arcs incident to node i */
    dist++;
    ForAllIncidentArcs(i, a ) { 
      g->edgeScanCnt[GLUPDATE]++;
      if ( Reverse ( a )->resCap > 0 ) {
	j = a->head;
	
	if (( j->status == REGULAR ) && ( j->d >= g->infD )) {
	  j->d = dist;
	  j->current = j->first;
    	  qEnqueue ( j, g->BFSqueue );
    	}
      }
    }
  } while ( !qEmpty ( g->BFSqueue ) );

  /* freeze gapped nodes */
  sPush ( NULL, g->freezer );
  do {
    sPop (i, g->regNodes);
    if (i->d >= g->infD) {
      i->d -= g->infD; /* restore label */
      i->status = FROZEN;
      g->regN--;
      sPush ( i, g->freezer );
      fCnt++;
    }
  } while ( !sEmpty ( g->regNodes ));

  if (fCnt == 1) 
    {
      sPop(i, g->freezer);
      fCnt--;
      i->status = REGULAR;
      g->regN++;
      oneNodeLayer(g, i);
    }

  if (fCnt == 0) sPop(i, g->freezer);
}

/* ================================================================ */
/* save minimum cut value found from a flow computation. */
static void saveFlowCut(graph *g) 
{
#ifdef SAVECUT
  node *v;
#endif

if (g->minCap <= g->sink->excess)
{
  if (g->mincuts_on == 0) g->mincuts++;
  return;
}
  g->minCap = g->sink->excess;

  g->cutCount++;
#ifdef VERBOSE
  printf("c New mc valu = ");
  fprintf_wt(stdout, "%", g->minCap);
  printf("\n");
#endif
  g->dtime = timer();

#ifdef SAVECUT
  ForAllNodes(g, v) 
    v->in_cut = (v->status == REGULAR);
  g->sink->in_cut = 1;
#ifdef CONTRACT
  ForAllVertices(g, v)
    v->in_cut = findLeader(v)->in_cut;
#endif
#endif
}

/* ================================================================ */
/* find a minimum s-t cut */
/* based on the push-relabel method */
static int STCut(graph *g)
{
  node   *i;
  bucket  *l;             /* current bucket */

  /* main loop */
  g->STCutCnt++;
  g->totalSize += g->regN + 1;
  g->relsSinceUpdate = 0;
  while (g->aMax >= g->sinkD) {
    l = g->buckets + g->aMax;

    i = FirstRealActive(l);   /* skip over contracted vertices */
    if ( i == NULL ) {
      g->aMax--;
    }
    else {
      l->firstActive = NextRealActive(i); /* delete i */

      assert(i->status == REGULAR);

      switch (discharge(g, i))
	{
	case 1:
	  {
	    /* i must be relabeled */

	    if ( l->first->bNext == NULL ) {
	      /* relabeling would create a gap */
	      gap (g, l);
	    }
	    else {
	      relabel(g, i);
	      g->relsSinceUpdate ++;

	      /* is it time for global update? */
#ifdef GLOBAL_UPDATES
	      if ( g->relsSinceUpdate > GLOB_UPDATE_FREQ * (double)g->regN ) {
		globalUpdate(g);
	      }
#endif

	    }
	    break;
	  }
	case 0:
	  {
	    break;
	  }
	}
    }
  }
  return (0);
}

/* ================================================================ */
/* 
   a common one-node layer case
   handle in a special way for efficiency

   relabeled node with no residual arcs to regular nodes
   must be handled in a special way in any case
   */
static void oneNodeLayer(graph *g, node *i)
{
#ifndef CONTRACT
  weight_t delta;
  node *j;
#endif
  arc *a;
  weight_t cutCap;

  g->oneCnt++;
  /* 
     when i would have become sink, all currently regular nodes
     would be sources and would push to i
     when i becomes source, it pushes flow to the currently 
     frozen nodes
     */

#ifdef CONTRACT
  g->regN--;
  i->status = FROZEN;  /* so that we don't try to delete it from buckets */
  cutCap = i->excess;
  ForAllIncidentArcs(i, a ) 
    {
      g->edgeScanCnt[g->context]++;
      if (a->head->status == REGULAR)
	cutCap += Reverse (a)->resCap;
    }
  if (safetoContract(g, g->source, i))
  {
    compactContract(g, g->source, i, MERGE_INTO_W);
  }
#else
  assert(i->status == REGULAR);
  i->status = SOURCE;
  i->d = g->sourceD;
  g->regN--;
  g->currentN--;
  g->gNodeCnt++;

  cutCap = i->excess;
  ForAllIncidentArcs(i, a ) {
    g->edgeScanCnt[g->context]++;
    j = a->head;

    /* sum capacity from regular nodes */
    if (j->status == REGULAR)
      {
	/*	assert((j->toContract != 0) || (a->resCap == 0)); */
	cutCap += Reverse ( a )->resCap;
      }

    /* saturate outgoing to frozen nodes */
    if ( j->status == FROZEN ) {
      delta = a->resCap;
      a->resCap = 0;
      Reverse ( a )->resCap += delta;
      j->excess += delta;

    }
  }
#endif

}


/* ================================================================ */
/* initialize maxflow computation */
static void initCutComp(graph *g, node *source, node *sink)
{
  node *v;
  bucket *b;
  weight_t maxmaxV; /* the largest vCap */
  weight_t minmaxV; /* the second largest vCap */

  for ( b = g->buckets + g->sourceD; b >= g->buckets; b-- )
    b->first = b->firstActive = NULL;
  
  maxmaxV = 0;
  minmaxV = 0;
  ForAllNodes (g, v) 
    {
      v->excess = 0;
      v->status = REGULAR;
      v->current = v->first;
    }

  g->source = source;
  g->sink = sink;
  g->source->status = SOURCE;


  /* initialize labels */
  g->source->d = g->sourceD;
  g->sink->d = 0;
  g->sinkD = 0;

  insertNode(g->sink, g->buckets);

  b = g->buckets + 1;
  ForAllNodes (g, v)
    if ((v != g->source) && (v != g->sink)) 
      {
	v->d = 1;
	insertNode(v, b);
      }

  g->dMax = 1;
  g->aMax = 0; /* no active nodes at this point */

  saturateOutgoing(g, g->source);
  
#ifdef INIT_UPDATES
  globalUpdate(g);
#endif
}


/* ================================================================ */
/* initialization procedure */

static void mainInit (graph *g)
{
  g->sourceD = 2*g->currentN - 1;
  g->infD = g->sourceD + 1;

#if defined(GLOBAL_UPDATES) || defined(INIT_UPDATES)
  qReset ( g->BFSqueue );
#endif

  sReset ( g->freezer );
  sReset ( g->regNodes );

#ifdef EXCESS_DETECTION
  sReset ( g->exStack );
#endif

  (g->buckets + g->infD)->first = NULL;


}


/* ================================================================ */

static void initContracted(graph *g)
{

  node *nextvertex;
  node *lastvertex;
  arc *nextedge;

  lastvertex = NULL;
  for ( nextvertex = g->vertices;
        nextvertex + 1 != g->sentinelVertex; /* there should be at least 1 node */
        nextvertex++ )
  {
    nextvertex->next = nextvertex + 1; 
    nextvertex->contracted = 0;
  }
  g->EmptyFirst = g->sentinelVertex;
  g->sentinelVertex = g->vertices + 10 * g->n;
  g->EmptyLast = g->vertices + 10 * g->n - 1;
  nextvertex->next = g->sentinelVertex;
  for ( nextvertex = g->EmptyFirst;
        nextvertex != g->EmptyLast;
        nextvertex++ )
  {
    nextvertex->next = nextvertex + 1;
    nextvertex->contracted = 1;
  }
  g->EmptyLast->next = NULL;
  g->Last = g->EmptyFirst;
  g->Last->up = NULL;
  g->Last->downleft = NULL;
  g->Last->downright = NULL;
  g->EmptyFirst = g->EmptyFirst->next;
  g->Active = g->vertices;

  g->FirstEmptyArc = g->sentinelArc + 1;
  g->FirstEmptyOrigArc = g->origarcs + (g->FirstEmptyArc - g->arcs) 
           /* which is = g->sentinelOrigArc + 1 */;
  for ( nextedge = g->FirstEmptyArc;
        nextedge < g->arcs + 2*g->m + 8*g->n;
        nextedge++ )
  {
    nextedge->next = nextedge + 1;
  } 
  g->LastEmptyArc = nextedge;

  for ( nextedge = g->FirstEmptyOrigArc;
        nextedge < g->origarcs + 2*g->m + 8*g->n;
        nextedge++ )
  {
    nextedge->next = nextedge + 1;
  }
  g->LastEmptyOrigArc = nextedge;

}


/* ================================================================ */

static void mainCleanup(graph *g)
{
#ifdef PR_34
  freeHeap(h);
#endif
  free(g->buckets); 
  qFree(g->BFSqueue); 
  sFree(g->freezer);
  sFree(g->regNodes);

  g->buckets = NULL;
  g->BFSqueue = NULL;
  g->freezer = NULL;
  g->regNodes = NULL;

#ifdef EXCESS_DETECTION
  sFree(g->exStack);
  g->exStack = NULL;
#endif
}


/* ================================================================ */
/* reseting compact and name values for all vertices */

static void resetVertices(graph *g)
{
  node *v;

  g->regN = 0; /* this must also be reset */
  sReset(g->regNodes);
  sReset(g->freezer);

  for (v = g->vertices; v != g->sentinelVertex; v++)
  {
    v->compact = 0;
    v->name = v - g->vertexalloc;
    v->current = NULL;
    v->toContract = 0;
  }
}


/* ================================================================ */
/* reseting values for all nodes */

static void resetNodes(graph *g)
{

/* Kostas 6/13/98: added the #ifndef NO_PR */
#ifndef NO_PR

  node *v;
  int i;


  g->source = NULL;
  g->sink = NULL;
  
  /* buckets get reset by initCutComp... */


  i=0;
  ForAllVertices(g, v)
  {
    g->nodes[i] = v;
    v->leader = v;
    v->index = i;  /* Maybe here we need v - g->vertices instead of i */
    g->nodes[i]->contracted = v->contracted;
    i++;
  }
  g->nodes[i] = NULL;

  g->uncontractedN = g->Active->uncontracted_count;

#else

  g->currentN = g->n;

#endif

  g->minCap = MAXWEIGHT;

}


/* ================================================================ */
/* reseting arc capacities to ORIGCAP */

static void resetArcs(graph *g)
{
  node *v;
  arc *a;

  ForAllVertices(g, v)
    ForAllIncidentArcs(v, a)
      a->CAPFIELD = a->ORIGCAP; 
}


/* ================================================================ */
/* copying arcs to origarcs */ 

static void copyArcs(graph *g)
{
  arc *fromarc; /* arc in array arcs */
  arc *toarc;   /* arc in array origarcs */
  node *v;

  OldForAllVertices(g, v)
    ForAllIncidentArcs(v, fromarc)
    {
      toarc = g->origarcs + (fromarc - g->arcs);
  
      toarc->ORIGCAP = fromarc->ORIGCAP;
      toarc->CAPFIELD = fromarc->CAPFIELD;
      toarc->head = fromarc->head;
      toarc->rev = g->origarcs + (fromarc->rev - g->arcs);
      if (fromarc->next != NULL)
        toarc->next = g->origarcs + (fromarc->next - g->arcs);
      else
        toarc->next = NULL;
      if (fromarc->prev != NULL)
        toarc->prev = g->origarcs + (fromarc->prev - g->arcs);
      else
        toarc->prev = NULL;
  #ifdef ENABLE_GRAPH_BACKUPS
      toarc->index = fromarc->index;
  #endif
      /* lca, usage, cost : probably unused */
    }

  g->sentinelOrigArc = g->origarcs + (g->sentinelArc - g->arcs);

  OldForAllVertices(g, v)
  {
    v->origfirst = g->origarcs + (v->first - g->arcs);
    v->origlast = g->origarcs + (v->last - g->arcs);
    v->origafterlast = g->origarcs + (v->afterlast - g->arcs);
  }
}


/* ================================================================ */
/* move vertices to point from arcs to origarcs */

static void moveVertices(graph *g)
{
  node *v;

  ForAllVertices(g, v)
  {
    v->first = g->origarcs + (v->first - g->arcs);
    v->last = g->origarcs + (v->last - g->arcs);
    v->afterlast = g->origarcs + (v->afterlast - g->arcs);
  }
}


/* ================================================================ */
/* move vertices to point from origarcs to arcs */
 
static void moveVerticesBack(graph *g)
{
  node *v;
 
  ForAllVertices(g, v)
  {
    v->first = g->arcs + (v->first - g->origarcs);
    v->last = g->arcs + (v->last - g->origarcs);
    v->afterlast = g->arcs + (v->afterlast - g->origarcs);
  }
}


/* ================================================================ */
/* main loop of Hao-Orlin */
void computeCuts(graph *g)
{
  node *v;
  treefield *tree;
  int s, t, i, node_s, rand_s, tmp1, tmp2;


  tree = createTree(g->n);
 
  qCreate ( g->n, g->BFSqueue );
  sCreate ( 2 * g->n, g->freezer );
  sCreate ( g->n, g->regNodes );
  sCreate ( g->n, g->exStack );
  memassert(g->buckets = (bucket*)calloc(2 * g->n + 1, sizeof(bucket)));

  g->calctimetotal = 0.0;
  g->cuttimetotal = 0.0;

for (s = 1; s < g->n; s++)
  {

    mainInit(g);

    g->iter = 0;
    g->maxiter = 0;
    g->mincuts = 0;
    g->mincuts_on = 1;

    g->Last->uncontracted_count = g->n;

    g->cuttimemarker = timer();

    resetVertices(g);
    resetArcs(g);
    resetNodes(g);

    ForAllNodes(g, v)
      v->status = REGULAR;


    g->mincuts_on = 1;
      compact(g);
    g->mincuts_on = 0;

    #ifndef NO_PR
    #ifdef PR_34
      clearHeap(h);
    #endif
      PRpreprocess(g, 0.75, 0.5, 0.5);
    #endif


    g->regN = g->currentN - 1;

    rand_s = (rand() % (g->n - s)) + s;

    tmp1 = tree[s].node_index;
    tmp2 = tree[rand_s].node_index;
    tree[s].node_index = tmp2;
    tree[rand_s].node_index = tmp1;

    tmp1 = tree[s].parent;
    tmp2 = tree[rand_s].parent;
    tree[s].parent = tmp2;
    tree[rand_s].parent = tmp1;

    tmp1 = tree[s].fl;
    tmp2 = tree[rand_s].fl;
    tree[s].fl = tmp2;
    tree[rand_s].fl = tmp1;

    tree[tree[s].node_index].alias_index = s;
    tree[tree[rand_s].node_index].alias_index = rand_s;

    node_s = tree[s].node_index;
    t = tree[s].parent;

    g->cuttimetotal += timer() - g->cuttimemarker;
    g->calctimemarker = timer();

    initCutComp(g, (g->vertices + node_s), (g->vertices + t));
    while (g->currentN > 1)
    {
      STCut(g);

      #ifndef NO_PR
        if (g->sink->excess < g->minCap)
      #endif
          saveFlowCut(g);

      #ifndef NO_PR
        if ((g->uncontractedN <= 2) || (!restartCutComp(g)))
      #endif
          break;

      g->iter++;
      assert(g->sink->status == REGULAR);
    }


    g->calctimetotal += timer() - g->calctimemarker;
    g->cuttimemarker = timer();

    tree[tree[node_s].alias_index].fl = g->minCap;

    for (i=1; i<g->n; i++)
    {
      if ((i != node_s) &&
         (g->vertices[i].in_cut == g->vertices[node_s].in_cut) &&
         (tree[tree[i].alias_index].parent == t))

        tree[tree[i].alias_index].parent = node_s;
    } 

    if (g->vertices[tree[tree[t].alias_index].parent].in_cut ==
                                          g->vertices[node_s].in_cut)
    {
      tree[tree[node_s].alias_index].parent = tree[tree[t].alias_index].parent;
      tree[tree[t].alias_index].parent = node_s;
      tree[tree[node_s].alias_index].fl = tree[tree[t].alias_index].fl;
      tree[tree[t].alias_index].fl = g->minCap;
    }

  g->cuttimetotal += timer() - g->cuttimemarker;
  }

  printf("AverageNodes = %f\n", (float)g->n);
  printf("AverageEdges = %f\n", (float)g->m);

  printf("Hao-Orlin comp. = 0\n");
  printf("AverageHaoOrlinNodes = 0.0\n");
  printf("AverageHaoOrlinEdges = 0.0\n");

  printf("Time for calculating the cuts: %f\n", g->calctimetotal);
  printf("Time for manipulating the cuts: %f\n", g->cuttimetotal);

}

/* ================================================================ */

int main (int argc, char *argv[])

{
  float t;
  graph *g;

  g = dimacsParse(stdin);

  copyArcs(g);

  initContracted(g);

  printf("c nodes: %14d    arcs: %15d\n", g->n, g->m);

  t = timer();

  computeCuts(g);

  g->dtime = g->dtime - t;
  t = timer() - t;

  printf("c relabels:  %10d    pushes:   %15d\n", g->relabelCnt, g->pushCnt); 

#ifndef NO_PR
  printf("c internal PR: %-6d PR 1: %-6d PR 2: %-6d PR 3: %-6d PR 4: %-6d\n", 
	 g->PR1Cnt+g->PR2Cnt+g->PR3Cnt+g->PR4Cnt, g->PR1Cnt, g->PR2Cnt, 
	 g->PR3Cnt, g->PR4Cnt);
  printf("c PR internal edge scans 1+2: %-8ld 3+4: %-8ld\n", 
	 g->edgeScanCnt[PR12], g->edgeScanCnt[PR34]);
#endif


#ifdef GLOBAL_UPDATES
#endif

#ifdef EXCESS_DETECTION
#endif

#ifndef NO_PR

#endif
  printf("c ttime: %16.2f", t);
  printf("\nc dtime: %16.2f\n", g->dtime);

  printf("\n");

  return 0;
}

/* ================================================================ */

