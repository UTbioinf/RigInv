/* Hao-Orlin min cut algorithm 
 *
 * Reference: J. Hao and J. B. Orlin, "A Faster Algorithm for Finding
 * the Minimum Cut in a Directed Graph". Journal of Algorithms, vol
 * 17, pp 424-446, 1994.
 *
 * applied to
 * Gomory-Hu all-pairs min-cut algorithm
 *
 * Reference: R. E. Gomory and T. C. Hu, "Multi-terminal network flows".
 * J. SIAM, vol 9, pp 551-570, 1961.
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

/*#define HO*/

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
#include "compact.h"
#include "tree.h"

/**************************** global variables **********************/

#ifdef PR_34
heap h;
#endif

/************************************** prototypes *******************/

static void insertActive(graph *g, node *j, bucket *l);
static int notInActive(graph *g, node *j, bucket *l);
static int deleteActive(graph *g, node *j, bucket *l);
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
static int saveFlowCut(graph *g);
static int STCut(graph *g);

static void oneNodeLayer (graph *g, node *i);
#ifdef EXCESS_DETECTION
static void excessGap (graph *g, node *i);
static int excessCheck(graph *g, node *j);
#endif

static int restartCutComp(graph *g);
static void initCutComp(graph *g, node *source, node *sink);

static void mainInit(graph *g);
static void mainCleanup(graph *g);
static void resetVertices(graph *g);
static void resetNodes(graph *g);
static void resetArcs(graph *g);
static void initContracted(graph *g);
tree *buildTree(graph *g);

static void moveVertices(graph *g);
static void moveVerticesBack(graph *g);

void computeCuts(graph *g, FILE* fp);

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

  assert(notInActive(g, j, l));
  j->nextA = l->firstActive;
  l->firstActive  = j;
  if (j->d > g->aMax)
    g->aMax = j->d;
}

/* ================================================================ */
/* delete node from bucket's active list */
static int notInActive(graph *g, node *j, bucket *l)
{
  node *tmpNode = (l)->firstActive;

  while ( tmpNode != NULL )
  {
    if (tmpNode == (j))
      return 0;
    tmpNode = tmpNode->nextA;
  }
  return 1;
}


/* ================================================================ */
/* delete node from bucket's active list */
static int deleteActive(graph *g, node *j, bucket *l)
{
  node *tmpNode1;
  node *tmpNode2;
  int retval = 0;

  if ( (l)->firstActive == (j) )
  {
    tmpNode1 = (l)->firstActive->nextA;
    (l)->firstActive->nextA = NULL;
    (l)->firstActive = tmpNode1; 
    retval = 1;
  }
  else
  {
    for (tmpNode1 = (l)->firstActive; tmpNode1 != NULL;
         tmpNode1 = tmpNode1->nextA)
    {
      if (tmpNode1->nextA == (j))
      {
        tmpNode2 = tmpNode1->nextA->nextA;
        tmpNode1->nextA->nextA = NULL;
        tmpNode1->nextA = tmpNode2;
        retval = 1;
        break;
      }
    } 
  }
  return retval;
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
        if (i->toContract != 0)
          i->toContract = 0;

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
        if (j->toContract != 0)
          j->toContract = 0;

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
/* doubleSplit, splits the graph according to two min-cuts */
 
static void doubleSplit (graph *g, tree *t)
{
 
  int counter1 = 0;
  int counter2 = 0;
  int quarter1 = 0;
  int quarter2 = 0;
  int tmp = 0;
  weight_t tmp_minCap;
  weight_t cut01Cap = 0, cut13Cap = 0, cut23Cap = 0, cut02Cap = 0;
  weight_t cut03Cap = 0, cut12Cap = 0;
  node *v;
  arc *a;


  if (g->min2Cap == MAXWEIGHT)
  {
    splitgraph(g, -1);
    expandTree (t, g->Active, g->Last->up->downright->firstN,
                   g->Last, g->Last->up->downright, g->minCap);
    return;
  }
 

  if (g->minCap == g->min2Cap)
  {
    ForAllNodes(g, v)
    {
      if (v->in_cut)
        counter1++;
      if (v->in_2cut)
        counter2++;
      tmp++;
    }
    counter1 = tmp - (counter1 << 1);
    if (counter1 < 0)
      counter1 = 0 - counter1;
    counter2 = tmp - (counter2 << 1);
    if (counter2 < 0)
      counter2 = 0 - counter2;
 
    if (counter2 < counter1)   /* counters contain differences of sides */
    {
      tmp_minCap = g->min2Cap;
      ForAllNodes(g, v)
      {
        tmp = v->in_cut;
        v->in_cut = v->in_2cut;
        v->in_2cut = tmp;
      }
    }
 
    splitgraph(g, -1);
    expandTree (t, g->Active, g->Last->up->downright->firstN,
                   g->Last, g->Last->up->downright, g->minCap);
    return;
  }
 
 
  if (g->minCap > g->min2Cap)
  {
    tmp_minCap = g->minCap;
    g->minCap = g->min2Cap;
    g->min2Cap = tmp_minCap;
    ForAllNodes(g, v)
    {
      tmp = v->in_cut;
      v->in_cut = v->in_2cut;
      v->in_2cut = tmp;
    }
  }
 
  ForAllNodes(g, v)
  {
    quarter1 = ((v->in_cut << 1) + v->in_2cut ) <<  2;
 
    ForAllIncidentArcs(v, a)
    {
      quarter2 = quarter1 + (a->head->in_cut << 1) + a->head->in_2cut;
 
      switch (quarter2)
      {
        case 0:
        case 5:
        case 10:
        case 15:
          break;    /* nodes in same quarter */
        case 3:
        case 12:
          cut03Cap += a->ORIGCAP;
          break;
        case 6:
        case 9:
          cut12Cap += a->ORIGCAP;
          break;
        case 1:
        case 4:
          cut01Cap += a->ORIGCAP;
          break;
        case 7:
        case 13:
          cut13Cap += a->ORIGCAP;
          break;
        case 11:
        case 14:
          cut23Cap += a->ORIGCAP;
          break;
        case 2:
        case 8:
          cut02Cap += a->ORIGCAP;
          break;
      }
    }
  }
 
  if ((g->min2Cap << 1) == cut01Cap + cut13Cap + cut12Cap)
                   /* Second cut between quarters 0 and 1 */
                   /* First splitgraph puts 0 to the left */
                   /* Second cut puts g->Last->in_cut = 0 */
  {
    splitgraph(g,0);
    expandTree (t, g->Active, g->Last->up->downright->firstN,
                   g->Last, g->Last->up->downright, g->minCap);
 
    ForAllNodes(g, v)
      v->in_cut = v->in_2cut;
 
    g->Last->in_cut = 0;
  }
 
  else if ((g->min2Cap << 1) == cut02Cap + cut01Cap + cut03Cap)
                   /* Second cut between quarters 0 and 1 */
                   /* First splitgraph puts 0 to the left */
                   /* Second cut puts g->Last->in_cut = 1 */
  {
    splitgraph(g, 0);
    expandTree (t, g->Active, g->Last->up->downright->firstN,
                   g->Last, g->Last->up->downright, g->minCap);
 
    ForAllNodes(g, v)
      v->in_cut = v->in_2cut;
 
    g->Last->in_cut = 1;
  }
 
  else if ((g->min2Cap << 1) == cut13Cap + cut23Cap + cut03Cap)
                   /* Second cut between quarters 2 and 3 */
                   /* First splitgraph puts 1 to the left */
                   /* Second cut puts g->Last->in_cut = 0 */
  {
    splitgraph(g, 1);
    expandTree (t, g->Active, g->Last->up->downright->firstN,
                   g->Last, g->Last->up->downright, g->minCap);
 
    ForAllNodes(g, v)
      v->in_cut = v->in_2cut;
 
    g->Last->in_cut = 0;
  }
 
  else if ((g->min2Cap << 1) == cut02Cap + cut12Cap + cut23Cap)
                   /* Second cut between quarters 2 and 3 */
                   /* First splitgraph puts 1 to the left */
                   /* Second cut puts g->Last->in_cut = 1 */
  {
    splitgraph(g, 1);
    expandTree (t, g->Active, g->Last->up->downright->firstN,
                   g->Last, g->Last->up->downright, g->minCap);
 
    ForAllNodes(g, v)
      v->in_cut = v->in_2cut;
 
    g->Last->in_cut = 1;
  }
 
  else
    assert(0);
 
 
  splitgraph(g, -1);
  expandTree (t, g->Active, g->Last->up->downright->firstN,
                   g->Last, g->Last->up->downright, g->min2Cap);
 
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
#ifdef EXCESS_DETECTION
  int zeroExcess=0;
#endif 

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

	if (j->excess >= g->minCap)
        {
	  if (j == g->sink ) {
	    i->current = a;
	    return (2);
	  }
#ifdef EXCESS_DETECTION
	/* we want to excessCheck(j) and, if excessCheck succeeds,
	   return appropriate value.
	   It is possible to continue if i was not reinserted into the
	   active list, but it is possible that aMax increased and
	   i no longer has max label among active nodes
	   */
	  else {
	    zeroExcess = (i->excess == 0);
	    if (excessCheck(g, j)) {
#ifndef NO_PR
	      if (!isLeader(i))
#else
	      if (i->status == SOURCE)
#endif
		return(4); /* i got contacted */
	      if (!zeroExcess) {
		assert(i->excess > 0);
		insertActive(g,i, (g->buckets+i->d));
		return(3); /* i just got reinserted */
	      }
	      else if (i->excess > 0)
		return(3); /* i got reinserted during excessCheck(j) */
	    } 
	  } 
        }

#endif

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

#ifdef EXCESS_DETECTION
	excessCheck(g, j);       
#endif

	break;
      }
      case FROZEN: {
	a->resCap = 0;
	Reverse ( a )->resCap += delta;
	      
	w->excess -= delta;
	j->excess += delta;

#ifdef EXCESS_DETECTION
	excessCheck(g, j);       
#endif

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

  assert(!emptyLayer);

  g->dMax = r;
  g->aMax = r;

}


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
static int saveFlowCut(graph *g) 
{

#ifdef SAVECUT
  node *v;
  int zero_side, one_side;
  float prop;
#endif

  g->totalcutcount++;

  g->bestminCap = g->sink->excess;

g->decr++;

  g->cutCount++;

  g->dtime = timer();

#ifdef SAVECUT

  zero_side = one_side = 0;
  ForAllNodes(g, v)
  {
    if ((v->status == REGULAR) || (v == g->sink))
      one_side++;
    else
      zero_side++;
  }

  if (zero_side <= one_side)
    prop = (1.0 * zero_side)/(1.0 * one_side);
  else
    prop = (1.0 * one_side)/(1.0 * zero_side);

  if ((prop > g->ratio) || 
      ((g->min2Cap == MAXWEIGHT) && (g->minCap != MAXWEIGHT)))
  {
    g->ratio = prop;
    g->bestcutcount++;


    if ((g->min2Cap == MAXWEIGHT) && (g->minCap != MAXWEIGHT))
    {
      g->min2Cap = g->minCap;
      ForAllNodes(g, v)
        v->in_2cut = v->in_cut;
    }


    g->minCap = g->bestminCap;

    ForAllNodes(g, v) 
      v->in_cut = (v->status == REGULAR);
    g->sink->in_cut = 1;

  }

#ifdef CONTRACT
  ForAllVertices(g, v)
    v->in_cut = findLeader(v)->in_cut;
#endif
#endif

if (g->mincutcount < 1)
  {
    g->mincutcount += 1;
    return 0;
  }

  else
    return 1;
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
	case 4: 
	  {
	    /* i was contracted into source */
	    break;
	  }
	  
	case 3: {
	  /* i got reinserted into the active list */
	  break;
	}
	case 2: {
	  /* current preflow value at least minCapValue */
	  /* early termination */
	  if (i->excess > 0) {
	    insertActive(g,i,l);
	  }
	  return (1); 
	}
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

#ifdef EXCESS_DETECTION
      excessCheck(g, j);
#endif
    }
  }
#endif

}
/* ================================================================ */

#ifdef EXCESS_DETECTION

/* ================================================================ */
/*
   When node with large excess is contrracted into the source
   we check if this created a gap
*/
static void excessGap (graph *g, node *i)
{
  bucket *b;
  node  *j;
  int  r;           /* index of the bucket before the gap */
  int fCnt = 0;     /* number of frozen nodes */

  g->gapCnt++;
  b = g->buckets + (i->d + 1);
  r = i->d;

  sPush (NULL, g->freezer);
  for ( ; b <= g->buckets + g->dMax; b++ ) {
    for (j = b->first; j != NULL; j = j->bNext) {
      assert(j != g->sink);
      j->status = FROZEN;
      sPush (j, g->freezer);
      fCnt++;
      g->regN--;
      g->gNodeCnt++;
    }
    b->firstActive = b->first = NULL;
  }

  if (fCnt == 1) {
    sPop(j, g->freezer);
    if (j->toContract) {
      sPush(j, g->freezer);  /* WHY?? */
    }
    else {
      fCnt--;
      j->status = REGULAR;
      g->regN++;
      oneNodeLayer(g, j);
    }
  }

  if (fCnt == 0) sPop(i, g->freezer);

  g->dMax = r;
  g->aMax = r;

}

/* ================================================================ */

/* perform the excess heuristic test
 
   returns 0 if nothing happened
   1 if j was contracted
*/
static int excessCheck(graph *g, node *j)
{
  node *v;
  int retv = 0;
  static int doing_excess_contraction = 0;

  if (j->excess < g->bestminCap || j->toContract ||
      g->currentN <= 2 /*|| g->uncontractedN <= 1*/ || j == g->sink )
    return 0;

  j->toContract = 1;
  sPush(j, g->exStack);
  
  if (doing_excess_contraction == 0)  /* don't allow this while 
					 loop to happen recursively */
    {
      doing_excess_contraction = 1;
      while (!sEmpty(g->exStack)) 
	{
	  sPop(v, g->exStack);
#ifndef NO_PR
	  if (isLeader(v))
#else
	  if (v->status != SOURCE)
#endif
	    {
	      if (g->currentN > 2)
		{
		  /* node v is now safe to contract into source */
		  
		  g->excessCnt++;
#ifndef NO_PR
		  if ((v->status != FROZEN) && (v != g->sink)) {
		    if ((v->bNext == NULL) && (v->bPrev == NULL)) {
		      excessGap(g, v);
		    }
                  }

		  compactContract(g, g->source, v, MERGE_INTO_W);
#else
		  saturateOutgoing(g, v);
		  if ((v->status != FROZEN) && (v != g->sink)) {
		    if ((v->bNext == NULL) && (v->bPrev == NULL))
		      excessGap(g, v);
		    deleteNode(v, g->buckets + v->d);
		  }
		  if (v->status == REGULAR)
		    g->regN--;
		  g->currentN--;
		  v->status = SOURCE;
		  v->d = g->sourceD;
#endif
		  retv = 1;
		}
	      else break;
	    }
	} 
      doing_excess_contraction = 0;
    }
  
  return(retv);
  
}
#endif



/* ================================================================ */
/* do another max-flow */
static int restartCutComp(graph *g)
{
  node *i, *j;
  int newD = 0, iD;
  int foundD = 0, regN_count, lowestD, lowestregD, highestD;
  int there_exist_contracted_regular_nodes;
  node *newSink=NULL;
  node *candSink=NULL;
  node *moveNode, *nextNode;
  bucket *b;
  int returnval = 1;

  /* aMax = 0; can have left-over ones from early termination! */

  b = g->buckets + g->sink->d;
  assert(b->first != NULL);
  deleteNode ( g->sink, b );

  assert(g->sink->toContract == 0);

#ifdef NO_PR
  /* saturate all arcs out of the sink */
  /* which will become a source */
  g->sink->toContract = 1;
  saturateOutgoing(g, g->sink);
  g->source = g->sink;
  g->source->status = SOURCE;
  g->source->d = g->sourceD;
  g->regN--;
#else
  if (g->currentN <= 2) return 0;

  compactContract(g,  g->source, g->sink, MERGE_INTO_W); 

  if (g->currentN <= 2) return(0);     /* catch case when excess contracts 
					  bring currentN down to 2 */
#endif


  qReset(g->BFSqueue);
  newSink = NULL;
  there_exist_contracted_regular_nodes = 0;
  highestD = 0; /* for use only if there_exist_contracted_regular_nodes = 1 */

  if ( g->regN > 0 )
  {
    newD = g->sinkD;          /* newD will be the lowest non-empty bucket */
    foundD = g->sinkD;        /* foundD will be the bucket of the sink found */
    regN_count = g->regN;
    for ( b = g->buckets + newD; b->first == NULL; b++)
    {
      newD++;
      foundD++;
      highestD = foundD;
    } 
    for (candSink = b->first; (candSink != g->sentinelVertex) && (candSink != NULL); candSink = candSink->bNext)
    {
      if (!(candSink->contracted))
      {
        newSink = candSink;
        break;
      }
      else
      {
        candSink->toTransfer = 1;
        qEnqueue(candSink, g->BFSqueue);
      }
      regN_count--;
    }
    while ((newSink == NULL) && (regN_count > 0))
    {
      foundD++;
      for ( b = g->buckets + foundD; b->first == NULL; b++)
      {
        foundD++;
        highestD = foundD;
      }
      for (candSink = b->first; (candSink != g->sentinelVertex) && (candSink != NULL); candSink = candSink->bNext)
      {
        if (!(candSink->contracted))
        {
          newSink = candSink;
          break;
        }
        else
        {
          candSink->toTransfer = 1;
          qEnqueue(candSink, g->BFSqueue);
        }
        regN_count--;
      }
    }

    if (newSink != NULL)               /* found new sink */
    {
      assert(newD <= foundD);
      if (newD <= foundD) /*(regN_count < g->regN)*/        /* it skipped over contracted nodes before
                                          finding an uncontracted sink            */
      {
        while ( !qEmpty(g->BFSqueue))
        {
          qDequeue(moveNode, g->BFSqueue);
          if (moveNode->d != foundD)
          {
            deleteNode(moveNode, g->buckets + moveNode->d);
            if (deleteActive (g, moveNode, g->buckets + moveNode->d))
              /*assert(moveNode->excess <= 0)*/;
            moveNode->d = foundD;
            moveNode->current = moveNode->first;
            insertNode(moveNode, g->buckets + foundD);

            if ( moveNode->excess > 0 )
            {
              assert ( notInActive(g, moveNode, g->buckets + foundD) );
              insertActive (g, moveNode, g->buckets + foundD);
            }
          }
          moveNode->toTransfer = 0;
        }
        newD = foundD;

      }
    }
    else
    {
      assert(regN_count == 0);
      there_exist_contracted_regular_nodes = 1;
      lowestregD = newD;              /* lowest d of regular nodes
                                      all regular nodes are contracted */
      if (highestD < g->dMax)
        highestD = g->dMax;

    }


  }
  /*else*/ if (newSink == NULL) {
    /* if no regular nodes, unfreeze the last layer */
    newD = g->infD;
    g->dMax = 0; /* need to compute new value */
    lowestD = g->infD;

    do {
      do {
	if (sEmpty(g->freezer))
	  return ( 0 );
	sPop ( i, g->freezer );
      } while (i == NULL);

      /* now i is not NULL */
      do {
	if (i->status == FROZEN) {

	  if (g->currentN <= 2 /*|| g->uncontractedN <= 1*/) return(0);

/* avg: this breaks. Does not seem to help much in any case 
   but it would be nice to have it fixed */
	  /*	 if (!excessCheck(i)) */ {
	    i->status = REGULAR;
	    g->regN++;

	    iD = i->d;

            if (iD < lowestD)
              lowestD = iD;

	    b = g->buckets + iD;
	    insertNode ( i, b );
	    if ( iD > g->dMax ) g->dMax = iD;
	    if (( i->excess > 0 )/* && ( iD < sourceD )*/) {
	      insertActive (g,i, b);
	    }

            if (newSink == NULL)
            {
              if (!(i->contracted))
              {
                newSink = i;
                foundD = iD;
              }
            }

            if ( iD < newD )
            {
              newD = iD;
              if (!(i->contracted))
              {
                newSink = i;
                foundD = iD;
              }
            }

	  }


	}
	sPop ( i, g->freezer );
      } while ( i != NULL );
    } while ((g->regN == 0) || ( newSink == NULL));

  if (g->currentN <= 2 /*|| g->uncontractedN <= 1*/) return(0);

  assert(g->regN > 0);

if (there_exist_contracted_regular_nodes)
{
  sReset(g->regNodes);

  while (!qEmpty(g->BFSqueue))
  {
    qDequeue(moveNode, g->BFSqueue);
    if (moveNode->d < foundD)
    {
      deleteNode(moveNode, g->buckets + moveNode->d);
      if (deleteActive (g, moveNode, g->buckets + moveNode->d))
        /*assert(moveNode->excess <= 0)*/;

      moveNode->d = foundD;
      moveNode->current = moveNode->first;
      insertNode(moveNode, g->buckets + foundD);

      if ( moveNode->excess > 0 )
      {
        assert ( notInActive(g, moveNode, g->buckets + foundD) );
        insertActive (g, moveNode, g->buckets + foundD);
      }

    }
    sPush(moveNode, g->regNodes);  /* Transfering all regNodes to the
                      stack, so that we can unset toTransfer later
                      and avoid the double transfer of the following
                      block for unfrozen nodes */
  }
  if (highestD > g->dMax)
    g->dMax = highestD;

}



assert(newD <= foundD);
if (newD < foundD)        /* it skipped over contracted nodes before
                             finding an uncontracted sink            */
{
  qReset(g->BFSqueue);

  for (b = g->buckets + lowestD; b < g->buckets + foundD; b++)
  {
    for (moveNode = b->first;
         (moveNode != g->sentinelVertex) && (moveNode != NULL);)
    {
      nextNode = moveNode->bNext;
      if ((moveNode->status == REGULAR) && (moveNode->toTransfer == 0))
      {
        qEnqueue(moveNode, g->BFSqueue);
        deleteNode(moveNode, g->buckets + moveNode->d);
        if (deleteActive (g, moveNode, g->buckets + moveNode->d))
        {
          /*assert(moveNode->excess <= 0)*/;
        }
      }
      else
        assert (0);
      moveNode = nextNode; 
    }
  }

  assert ( !qEmpty(g->BFSqueue));

  while ( !qEmpty(g->BFSqueue))
  {
    qDequeue(moveNode, g->BFSqueue);
    moveNode->d = foundD;
    moveNode->current = moveNode->first;
    insertNode(moveNode, g->buckets + foundD);
    if ( moveNode->excess > 0 )
    {
      assert ( notInActive(g, moveNode, g->buckets + foundD) );
      insertActive (g, moveNode, g->buckets + foundD);
    }
  }

  newD = foundD;
 

}

if (there_exist_contracted_regular_nodes)
{
  while (!sEmpty(g->regNodes))
  {
    sPop(moveNode, g->regNodes);
    moveNode->toTransfer = 0;
  }
}


}

assert(qEmpty(g->BFSqueue));


  g->sink = newSink;
  g->sinkD = newD;

  if (g->sink->excess > 0) {
    /* delete sink from active list */
    b = g->buckets + g->sink->d;
    i = FirstRealActive(b);
    if (i != NULL) {
    if (i == g->sink)
      b->firstActive = NextRealActive(g->sink);
    else {
      while ( 1 ) {
	j = NextRealActive(i);
	if ( j == g->sink ) break;
        if (j == NULL) break;
	else i = j;
      }
      /* now i points to sink */
      i->nextA = NextRealActive(g->sink);
    }
    }
  }

  g->source->d = g->sourceD;

#ifndef NO_PR
  if (g->edgeScanCnt[PR12] * INT_PR_FREQ_12  < g->edgeScanCnt[MAIN]) 
    {
#ifndef DONT_DO_INTERNAL
      sourcePR12(g, g->source);
#endif
      if (g->currentN <= 2) return 0;

    }
  
  if (g->edgeScanCnt[PR34] * INT_PR_FREQ_34  < g->edgeScanCnt[MAIN]) 
    {
#ifndef DONT_DO_INTERNAL
      sourcePR34(g, g->source);
#endif

      if (g->currentN <= 2) return 0;
    }
#endif
  g->context = MAIN;

  return ( returnval );  /* default 1 */
}


/* ================================================================ */
/* initialize maxflow computation */
static void initCutComp(graph *g, node *source, node *sink)
{
  node *v;
  bucket *b;
#ifdef NO_PR
  arc *a;
#endif
  weight_t vCap;
  weight_t maxmaxV; /* the largest vCap */
  weight_t minmaxV; /* the second largest vCap */


  for ( b = g->buckets + g->sourceD; b >= g->buckets; b-- )
    b->first = b->firstActive = NULL;
  
  /* make the largest capacity node initial sink 
     - this should still be fine even when having contracted vertices
       which we obviously don't want as a sink */
  /* we also make the second largest capacity node initially the source
     - this shouldn't contradict either with the contracted vertices */
  maxmaxV = 0;
  minmaxV = 0;
  ForAllNodes (g, v) 
    {
#ifdef NO_PR
      vCap = 0;
      ForAllIncidentArcs(v, a )
	vCap += a->resCap;
#else
      vCap = v->cap;
#endif

      if (!(v->contracted))
      {
        if ( vCap > minmaxV ) 
        {
          if ( vCap > maxmaxV )
          {
            minmaxV = maxmaxV;
            maxmaxV = vCap;
            if ( g->sink != NULL)
              g->source = g->sink;
            g->sink = v;
            /** g->sink = v; **/
          }
          else
          {
            minmaxV = vCap;
            g->source = v;
          }
        }
      }
      v->excess = 0;
      v->status = REGULAR;
      v->current = v->first;
    }

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


  /* initialize timers */
  g->calctimetotal = 0.0;
  g->cuttimetotal = 0.0;

  g->bestcutcount = 0;

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

/* The next line initializes g->Active->uncontracted_count,
which is later assigned to g->uncontractedN in resetNodes.
This is done just for the first iteration of the algorithm. */
  g->Active->uncontracted_count = g->n;

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
  g->sourceD = 2*g->currentN - 1;
  g->infD = g->sourceD + 1;

  sReset(g->regNodes);
  sReset(g->freezer);
  qReset(g->BFSqueue);


  for (v = g->vertices; v != g->sentinelVertex; v++)
  {
    v->compact = 0;
    v->name = v - g->vertexalloc;
    v->current = NULL;
    v->toContract = 0;
    v->toTransfer = 0;
    v->excess = 0;
  }
}


/* ================================================================ */
/* reseting values for all nodes */

static void resetNodes(graph *g)
{

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
/* added for uncontractedN: */
    if (v->contracted)
    {
      v->contains_uncontracted = 0;
      g->nodes[i]->contains_uncontracted = 0;
    }
    else /* v->contracted == 0 */
    {
      v->contains_uncontracted = 1;
      g->nodes[i]->contains_uncontracted = 1;
    }
    i++;
  }
  g->nodes[i] = NULL;

#endif

  g->minCap = MAXWEIGHT;
  g->min2Cap = MAXWEIGHT;
  g->bestminCap = MAXWEIGHT;
  g->ratio = 0.0;
  g->mincutcount = 0;

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
void computeCuts(graph *g, FILE* fp)
{
  node *v;
  tree *t;
  int nextrand;
  node *nextsink;

  int totalnodes = 0;
  int totaledges = 0;

  int previter;

  int totalhaoorlin = 0;


  t = createTree(g->n);
 
  qCreate ( g->n, g->BFSqueue );
  sCreate ( 2 * g->n, g->freezer );
  sCreate ( g->n, g->regNodes );
  sCreate ( g->n, g->exStack );
  memassert(g->buckets = (bucket*)calloc(2 * g->n + 1, sizeof(bucket)));

  
    mainInit(g);

    g->iter = 0;
    g->maxiter = 0;
    g->mincuts = 0;
    g->mincuts_on = 1;


  g->Last->uncontracted_count = g->n;
#ifdef LOON_ALLOW_PRINT
g->cuttimemarker = timer();
#endif
  while ( g->Active != NULL )
  {
    if (ReachedLeaf(g))
    {
      backTree(g, g->Last);
      restoregraph(g);
    }
    else
    {
      resetVertices(g);
      resetArcs(g);

      resetNodes(g);

totalhaoorlin++;


ForAllNodes(g, v)
    v->status = REGULAR;

g->decr = 0;
 
g->mincuts_on = 1;
  compact(g);
g->mincuts_on = 0;

#ifndef NO_PR
#ifdef PR_34
  makeHeap(h, g->currentN);
#endif
  PRpreprocess(g, 0.75, 0.5, 0.5);
#endif


#ifdef LOON_ALLOW_PRINT
g->cuttimetotal += timer() - g->cuttimemarker;
g->calctimemarker = timer();
#endif
g->regN = g->currentN - 1;

previter = g->iter;


      nextrand = rand() % (g->Last->uncontracted_count - 1);
      nextsink = g->Active->next;
      for (; nextrand > 0; nextrand--)
        nextsink = nextsink->next;
      initCutComp(g, g->Active, nextsink);

          


      while ((g->currentN > 1) /*&& (g->uncontractedN > 1)*/)
      {
        STCut(g);

        if ((g->sink->excess <= g->bestminCap))
          if (saveFlowCut(g))
            break;

        if (!restartCutComp(g))
          break;

        g->iter++;
        assert(g->sink->status == REGULAR);
      }

#ifdef LOON_ALLOW_PRINT
g->calctimetotal += timer() - g->calctimemarker;
g->cuttimemarker = timer();
#endif


      totalnodes += ( findNodes(g, g->Active));
      totaledges += findEdges(g, g->Active);
 
      doubleSplit(g, t);
 
    }
  }
#ifdef LOON_ALLOW_PRINT
g->cuttimetotal += timer() - g->cuttimemarker;

  printf("AverageNodes = %f\n", (float)totalnodes/(float)(g->n - 1));
  printf("AverageEdges = %f\n", (float)totaledges/(2.0 * (float)(g->n - 1)));

  printf("Hao-Orlin comp. = %d\n", totalhaoorlin);
  printf("AverageHaoOrlinNodes = %f\n", (float)totalnodes/(float)(totalhaoorlin));
  printf("AverageHaoOrlinEdges = %f\n", (float)totaledges/(2.0 * (float)(totalhaoorlin)));

  printf("Time for calculating the cuts: %f\n", g->calctimetotal);
  printf("Time for manipulating the cuts: %f\n", g->cuttimetotal);
#endif
  
  writeSimpleTree(fp, t, t->n);
}

/* ================================================================ */

int main (int argc, char *argv[])

{
  float t;
  graph *g;

  FILE *fin, *fout;
  fin = fopen(argv[1], "r");
  g = dimacsParse(fin);
  fclose(fin);

  initContracted(g);

#ifdef LOON_ALLOW_PRINT
  printf("c nodes: %14d    arcs: %15d\n", g->n, g->m);
#endif

#ifdef LOON_ALLOW_PRINT
  t = timer();
#endif

  fout = fopen(argv[2], "w");
  computeCuts(g, fout);
  fclose(fout);

#ifdef LOON_ALLOW_PRINT
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

#ifndef NO_PR

#endif
  printf("c ttime: %16.2f", t);
  printf("\nc dtime: %16.2f\n", g->dtime);

  printf("\n");
#endif // of LOON_ALLOW_PRINT
  return 0;
}

/* ================================================================ */

