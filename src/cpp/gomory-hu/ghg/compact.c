#include "compact.h"





int safetoContract(graph *g, node *v, node *w)
{
 
  arc *a;
 
  if (!(v->contracted))
    ForAllIncidentArcs(v, a)
    {
      if ((a->head != w) && (a->head->contracted))
          return 0;
    }
 
  if (!(w->contracted))
    ForAllIncidentArcs(w, a)
    {
      if ((a->head != w) && (a->head->contracted))
        return 0;
    }
 
 
  if (g->uncontractedN > 2) return 1;
 
  if (!(w->contracted))
  {
    w->toContract = 0;
    return 0;
  }
 
  return 1;
 
}


/* =========================================================================== */

/* split graph, splits the given graph and returns the two resulting graphs
   the first graph is the one which contains the most recently contracted node */

void splitgraph(graph *g, int left_side)
{

/*
    the new splitgraph will set a pointer to the first vertex which is
       active after the splitting and the contraction 
    each node will now have the following "additional" fields:
    next : indicates the current active/inactive list
           the splitting depends on the previous next-list (initiated by
           the active pointer) and the 0/1 values of in_cut
         : also the empty list of to-be-contracted vertices is
           connected via this list
         : end of both lists is the sentinelVertex, although in the
           case of contracted vertices it should never be reached
    contracted : pointer to the contracted vertex (all the contracted
                 vertices point to the contracted vertex)
    firstN, lastN : pointers to the first/last element of the list
                    (one level higher) which was contracted to this node
                    The list of nodes must be connected via the next pointer
    up, downleft, downright : pointers to contracted parent/children

    The graph g will also have the following additional fields:
    Active : pointer to the first active vertex
    Last : pointer to the most recent contracted vertex
           Initially it points to an empty root...
    EmptyFirst : pointer to the first empty vertex in the vertex-array
    EmptyLast : pointer to the last empty vertex in the vertex-array
    All next lists end at the sentinelVertex

    The newly contracted vertex will be last on the next-list

*/
 
  node * first_active_zero_node, * first_active_one_node;
  node * last_active_zero_node, * last_active_one_node, * next_active_node;
  node * contracted_zero, * contracted_one;
  arc * next_edge;
  arc * edge_to_contracted;
  arc * previous_edge;
  int side;  /* tells which one will be the left side */
  int max_name; /* counter */
  node * temp_node;
  int zero_count = 0;  /* counts the number of nodes on the zero side of the cut */
  int one_count = 0;   /* counts the number of nodes on the one side of the cut */
  int zero_uncontracted_count = 0;  /* counts the number of uncontracted nodes on the zero side of the cut */
  int one_uncontracted_count = 0;  /* counts the number of uncontracted nodes on the one side of the cut */

  /* The following loop splits all currently active vertices into
     two sets, according to the in_cut value */
  /* It also contracts all the one_nodes to the g->Empty node */
  /* It also concatenates the edge lists */

  first_active_zero_node = NULL;
  first_active_one_node = NULL;
  last_active_zero_node = NULL;
  last_active_one_node = NULL;
  for ( next_active_node = g->Active;
        next_active_node != g->sentinelVertex;
        next_active_node = next_active_node->next )
  {
    if (next_active_node->in_cut == 0)
    {
      zero_count++;
      if (first_active_zero_node == NULL)
      {
        first_active_zero_node = next_active_node; 
        last_active_zero_node = next_active_node; 
      }
      else
      {
        last_active_zero_node->next = next_active_node; 
        last_active_zero_node = next_active_node;
      }
      if (next_active_node->contracted == 0)
        zero_uncontracted_count++;
    }
    else   /* next_active_node->in_cut == 1 */
    {
      one_count++;
      if (first_active_one_node == NULL)
      {
        first_active_one_node = next_active_node;
        last_active_one_node = next_active_node;
      }
      else
      {
        last_active_one_node->next = next_active_node;
        last_active_one_node = next_active_node;
      }
      if (next_active_node->contracted == 0)
        one_uncontracted_count++;
    }
  }

  contracted_zero = g->EmptyFirst;
  g->EmptyFirst = g->EmptyFirst->next;
  contracted_one = g->EmptyFirst;
  g->EmptyFirst = g->EmptyFirst->next;

  last_active_zero_node->next = contracted_zero;
  contracted_zero->next = g->sentinelVertex;
  contracted_zero->uncontracted_count = zero_uncontracted_count;
  last_active_one_node->next = contracted_one;
  contracted_one->next = g->sentinelVertex;
  contracted_one->uncontracted_count = one_uncontracted_count;

  /* The next two lines maintain the currentN value */
  first_active_zero_node->currentNinlist = zero_count + 1; 
  first_active_one_node->currentNinlist = one_count + 1; 




  /* The following loop finds which side should be directly connected
     to the rest of the tree */
 
  max_name = 0;
  side = 0;
  for (temp_node = first_active_zero_node; 
       temp_node != last_active_zero_node->next; /* last_node excluded */
       temp_node = temp_node->next)
  {

    if (max_name < temp_node->name)
      max_name = temp_node->name;

  }

  for (temp_node = first_active_one_node;
       temp_node != last_active_one_node->next;
       temp_node = temp_node->next)
  {
    if (temp_node->name > max_name)
    {
      side = 1;
      break;
    }
  } 
 

  if (left_side != -1)
    side = left_side;


  if ( side == 0 )
  /* the previous line was:
  if ( zero_count <= one_count )
  */
  {
    g->Last->downleft = contracted_zero;
    g->Last->downright = contracted_one;
    g->Active = first_active_zero_node;
    g->currentN = g->Active->currentNinlist;
  }
  else    /* zero_count > one_count */
  {
    g->Last->downleft = contracted_one;
    g->Last->downright = contracted_zero;
    g->Active = first_active_one_node;
    g->currentN = g->Active->currentNinlist;
  }
  contracted_zero->up = g->Last;
  contracted_zero->downleft = NULL;
  contracted_zero->downright = NULL;
  contracted_zero->firstN = first_active_zero_node;
  contracted_one->up = g->Last;
  contracted_one->downleft = NULL;
  contracted_one->downright = NULL;
  contracted_one->firstN = first_active_one_node; /* probably obsolete */
  g->Last = g->Last->downleft;   /* regardless of the in_cut value */


  /*
     After having fixed the vertex-lists, now lets fix the edges.
     We will contract the edges of the one side, hoping to restore
     them by keeping the reverse arcs on the other side untouched.
  */

  if ( side == 0 )
  /* the previous line was:
  if ( zero_count <= one_count )
  */
  {
    contracted_one->first = g->FirstEmptyOrigArc;
    contracted_one->last = NULL;

    /* First lets fix the edges for the one side... */
  
    for ( next_active_node = first_active_one_node;
          next_active_node != last_active_one_node->next;
          next_active_node = next_active_node->next )
    {
  
      edge_to_contracted = NULL;
      previous_edge = NULL;
      for ( next_edge = next_active_node->first;
            next_edge != NULL;
            next_edge = next_edge->next )
      {

        if ( next_edge->head->in_cut != next_active_node->in_cut )
                                        /* next_edge->head on other side of cut */
        {
  
          if ( edge_to_contracted == NULL )
          {
            if ( contracted_one->last == NULL )
            {
              contracted_one->last = contracted_one->first;
              contracted_one->last->prev = NULL;
            }
            else
            {
              contracted_one->last->next->prev = contracted_one->last;
              contracted_one->last = contracted_one->last->next;
            }
            edge_to_contracted = next_edge;
            edge_to_contracted->head = contracted_one;
            edge_to_contracted->rev = contracted_one->last;
            contracted_one->last->ORIGCAP = next_edge->ORIGCAP;
            contracted_one->last->CAPFIELD = next_edge->ORIGCAP;
            contracted_one->last->head = next_active_node;
            contracted_one->last->rev = edge_to_contracted;
            if ( previous_edge == NULL )
              previous_edge = next_active_node->first;
            else
              previous_edge = previous_edge->next;
          }
          else   /* edge_to_contracted != NULL */
          {
            edge_to_contracted->ORIGCAP += next_edge->ORIGCAP;
            edge_to_contracted->CAPFIELD = edge_to_contracted->ORIGCAP;
 
     contracted_one->last->ORIGCAP += next_edge->ORIGCAP;
     contracted_one->last->CAPFIELD = contracted_one->last->ORIGCAP;
 
            assert ( previous_edge != NULL );
              previous_edge->next = next_edge->next;
              if (next_edge->next != NULL)
                previous_edge->next->prev = previous_edge;
            g->LastEmptyOrigArc->next = next_edge;
            g->LastEmptyOrigArc = g->LastEmptyOrigArc->next;
          }
        }
        else
        {
          if ( previous_edge == NULL )
          {
            previous_edge = next_active_node->first;
            previous_edge->prev = NULL;
          }
          else
            previous_edge = previous_edge->next;
        }
      }

      next_active_node->last = previous_edge;

    }

    if (contracted_one->last == NULL)
    {
      contracted_zero->first = contracted_one->first;
      contracted_one->first = NULL;
      contracted_one->afterlast = contracted_zero->first;
    }
    else
    {
      contracted_zero->first = contracted_one->last->next;
      contracted_one->afterlast = contracted_one->last->next;
      contracted_one->last->next = NULL;
    }
    contracted_zero->last = NULL;

    /* Now lets fix the edges for the zero side... */
  
    for ( next_active_node = first_active_zero_node;
          next_active_node != last_active_zero_node->next;
          next_active_node = next_active_node->next )
    {
  
      edge_to_contracted = NULL;
      previous_edge = NULL;
      for ( next_edge = next_active_node->first;
            next_edge != NULL;
            next_edge = next_edge->next )
      {
  
        if ( next_edge->head->in_cut != next_active_node->in_cut )
                                        /* next_edge->head on other side of cut */
        {
  
          if ( edge_to_contracted == NULL )
          {
            if ( contracted_zero->last == NULL )
            {
              contracted_zero->last = contracted_zero->first;
              contracted_zero->last->prev = NULL;
            }
            else
            {
              contracted_zero->last->next->prev = contracted_zero->last;
              contracted_zero->last = contracted_zero->last->next;
            }
            edge_to_contracted = next_edge;
            edge_to_contracted->head = contracted_zero;
            edge_to_contracted->rev = contracted_zero->last;
            contracted_zero->last->ORIGCAP = next_edge->ORIGCAP;
            contracted_zero->last->CAPFIELD = next_edge->ORIGCAP;
            contracted_zero->last->head = next_active_node;
            contracted_zero->last->rev = edge_to_contracted;
            if ( previous_edge == NULL )
              previous_edge = next_active_node->first;
            else
              previous_edge = previous_edge->next;
          }
          else   /* edge_to_contracted != NULL */
          {
            edge_to_contracted->ORIGCAP += next_edge->ORIGCAP;
            edge_to_contracted->CAPFIELD = edge_to_contracted->ORIGCAP;

     contracted_zero->last->ORIGCAP += next_edge->ORIGCAP;
     contracted_zero->last->CAPFIELD = contracted_zero->last->ORIGCAP;

            assert ( previous_edge != NULL );
              previous_edge->next = next_edge->next;
              if (next_edge->next != NULL)
                previous_edge->next->prev = previous_edge;
            g->LastEmptyOrigArc->next = next_edge;
            g->LastEmptyOrigArc = g->LastEmptyOrigArc->next;
          }
        }
        else
        {
          if ( previous_edge == NULL )
          {
            previous_edge = next_active_node->first;
            previous_edge->prev = NULL;
          }

          else
            previous_edge = previous_edge->next;
        }
      }
 
      next_active_node->last = previous_edge;
 
    }

    if (contracted_zero->last == NULL)
    {
      g->FirstEmptyOrigArc = contracted_zero->first;
      contracted_zero->first = NULL;
      contracted_zero->afterlast = g->FirstEmptyOrigArc; 
    }
    else
    {
      g->FirstEmptyOrigArc = contracted_zero->last->next;
      contracted_zero->afterlast = contracted_zero->last->next; 
      contracted_zero->last->next = NULL;
    }
  }

  else

  {
    contracted_zero->first = g->FirstEmptyOrigArc;
    contracted_zero->last = NULL;

    /* First lets fix the edges for the zero side... */
  
    for ( next_active_node = first_active_zero_node;
          next_active_node != last_active_zero_node->next;
          next_active_node = next_active_node->next )
    {
  
      edge_to_contracted = NULL;
      previous_edge = NULL;
      for ( next_edge = next_active_node->first;
            next_edge != NULL;
            next_edge = next_edge->next )
      {
  
        if ( next_edge->head->in_cut != next_active_node->in_cut )
                                        /* next_edge->head on other side of cut */
        {
  
          if ( edge_to_contracted == NULL )
          {
            if ( contracted_zero->last == NULL )
            {
              contracted_zero->last = contracted_zero->first;
              contracted_zero->last->prev = NULL;
            }
            else
            {
              contracted_zero->last->next->prev = contracted_zero->last;
              contracted_zero->last = contracted_zero->last->next;
            }
            edge_to_contracted = next_edge;
            edge_to_contracted->head = contracted_zero;
            edge_to_contracted->rev = contracted_zero->last;
            contracted_zero->last->ORIGCAP = next_edge->ORIGCAP;
            contracted_zero->last->CAPFIELD = next_edge->ORIGCAP;
            contracted_zero->last->head = next_active_node;
            contracted_zero->last->rev = edge_to_contracted;
            if ( previous_edge == NULL )
              previous_edge = next_active_node->first;
            else
              previous_edge = previous_edge->next;
          }
          else   /* edge_to_contracted != NULL */
          {
            edge_to_contracted->ORIGCAP += next_edge->ORIGCAP;
            edge_to_contracted->CAPFIELD = edge_to_contracted->ORIGCAP;
 
     contracted_zero->last->ORIGCAP += next_edge->ORIGCAP;
     contracted_zero->last->CAPFIELD = contracted_zero->last->ORIGCAP;

            assert ( previous_edge != NULL );
              previous_edge->next = next_edge->next;
              if (next_edge->next != NULL)
                previous_edge->next->prev = previous_edge;
            g->LastEmptyOrigArc->next = next_edge;
            g->LastEmptyOrigArc = g->LastEmptyOrigArc->next;
          }
        }
        else
        {
          if ( previous_edge == NULL )
          {
            previous_edge = next_active_node->first;
            previous_edge->prev = NULL;
          }

          else
            previous_edge = previous_edge->next;
        }
      }
 
      next_active_node->last = previous_edge;
 
    }

    if (contracted_zero->last == NULL)
    {
      contracted_one->first = contracted_zero->first;
      contracted_zero->first = NULL;
      contracted_zero->afterlast = contracted_one->first;
    }
    else
    {
      contracted_one->first = contracted_zero->last->next;
      contracted_zero->afterlast = contracted_zero->last->next;
      contracted_zero->last->next = NULL;
    }
    contracted_one->last = NULL;

    /* Now lets fix the edges for the one side... */
  
    for ( next_active_node = first_active_one_node;
          next_active_node != last_active_one_node->next;
          next_active_node = next_active_node->next )
    {
  
      edge_to_contracted = NULL;
      previous_edge = NULL;
      for ( next_edge = next_active_node->first;
            next_edge != NULL;
            next_edge = next_edge->next )
      {
  
        if ( next_edge->head->in_cut != next_active_node->in_cut )
                                        /* next_edge->head on other side of cut */
        {
  
          if ( edge_to_contracted == NULL )
          {
            if ( contracted_one->last == NULL )
            {
              contracted_one->last = contracted_one->first;
              contracted_one->last->prev = NULL;
            }
            else
            {
              contracted_one->last->next->prev = contracted_one->last;
              contracted_one->last = contracted_one->last->next;
            }
            edge_to_contracted = next_edge;
            edge_to_contracted->head = contracted_one;
            edge_to_contracted->rev = contracted_one->last;
            contracted_one->last->ORIGCAP = next_edge->ORIGCAP;
            contracted_one->last->CAPFIELD = next_edge->ORIGCAP;
            contracted_one->last->head = next_active_node;
            contracted_one->last->rev = edge_to_contracted;
            if ( previous_edge == NULL )
              previous_edge = next_active_node->first;
            else
              previous_edge = previous_edge->next;
          }
          else   /* edge_to_contracted != NULL */
          {
            edge_to_contracted->ORIGCAP += next_edge->ORIGCAP;
            edge_to_contracted->CAPFIELD = edge_to_contracted->ORIGCAP;
 
     contracted_one->last->ORIGCAP += next_edge->ORIGCAP;
     contracted_one->last->CAPFIELD = contracted_one->last->ORIGCAP;
 
            assert ( previous_edge != NULL );
              previous_edge->next = next_edge->next;
              if (next_edge->next != NULL)
                previous_edge->next->prev = previous_edge;
            g->LastEmptyOrigArc->next = next_edge;
            g->LastEmptyOrigArc = g->LastEmptyOrigArc->next;
          }
        }
        else
        {
          if ( previous_edge == NULL )
          {
            previous_edge = next_active_node->first;
            previous_edge->prev = NULL;
          }
          else
            previous_edge = previous_edge->next;
        }
      }
 
      next_active_node->last = previous_edge;
 
    }

    if (contracted_one->last == NULL)
    {
      g->FirstEmptyOrigArc = contracted_one->first;
      contracted_one->first = NULL;
      contracted_one->afterlast = g->FirstEmptyOrigArc;
    }
    else
    {
      g->FirstEmptyOrigArc = contracted_one->last->next;
      contracted_one->afterlast = contracted_one->last->next;
      contracted_one->last->next = NULL;
    }

  }

}





void restoregraph (graph *g)
{

  /* This function is called whenever we reach a bottom leaf of
     splitgraph */


  while (( g->Last->up != NULL ) && ( g->Last == g->Last->up->downright ))
                                                   /* g->Last is right child */
  {

    if (g->Last->last != NULL)  /* g->Last had some adjacent edges */
    {
      g->LastEmptyOrigArc->next = g->Last->first;
      g->LastEmptyOrigArc = g->Last->last;
    } 

    /* Append g->Last to the empty list of contracted nodes */
    g->EmptyLast->next = g->Last;
    g->EmptyLast = g->EmptyLast->next;
    g->Last = g->Last->up;
    g->Last->downright = NULL;


  }

  if ( g->Last->up == NULL )
  {
    g->Active = NULL;        /* we are done */
    return;
  }
  /* else */

  assert ( g->Last->up->downleft == g->Last );

  /* Now we have to switch the contracted sides, from left to right */

  if (g->Last->last != NULL)  /* g->Last had some adjacent edges */
  {
    g->LastEmptyOrigArc->next = g->Last->first; 
    g->LastEmptyOrigArc = g->Last->last;
  }

  g->Last = g->Last->up->downright;
  g->Active = g->Last->firstN;
  g->currentN = g->Active->currentNinlist;
  g->uncontractedN = g->Last->uncontracted_count; 

}


int ReachedLeaf(graph * g)
{
  int UncontractedVertex = 0; 
  node * nextvertex;

  for ( nextvertex = g->Active;
        nextvertex != g->sentinelVertex;
        nextvertex = nextvertex->next )
  {
    if ( nextvertex->contracted == 0 )  /* found uncontracted vertex */
      UncontractedVertex++;

    if ( UncontractedVertex > 1 )
      return 0;
  }

  assert( UncontractedVertex > 0); /* never allow 0 Uncontracted vertices */
  return 1;   /* found one uncontracted vertex => reached leaf */
}


void debug(graph *g)
{

#ifdef LOON_ALLOW_PRINT
  int i;

  for (i=0; i<15; i++)
  {
    if (g->arcs[i].next != NULL)
      printf("g->arcs[%d].next = %ld\n", i, g->arcs[i].next - g->arcs); 
    else
      printf("g->arcs[%d].next = NULL\n", i);
  }
  for (i=0; i<15; i++)
  {
    if (g->vertices[i].next != NULL)
      printf("g->vertices[%d].next = %ld\n", i, g->vertices[i].next - g->vertices);
    else
      printf("g->vertices[%d].next = NULL\n", i);
  }
#endif
}
