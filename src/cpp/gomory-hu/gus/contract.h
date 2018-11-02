/* contract.h: routines for graph contraction 
*/

#include "graph.h"

#if defined(CONTRACT) && !defined(CONTRACT_H)
#define CONTRACT_H

int OLDsafetoContract(graph *g, node *v, node *w);
int safetoContract(graph *g, node *v, node *w);

/* delete incident arc a from v in g */
void DeleteArc(graph *g, node *v, arc *a);

/******* set-union contraction functions *******/

/* set-union contraction */
void setUnionContract(graph *g, node *v, node *w);

/* find the node this vertex is part of */
node *findLeader(node *v);

/* compactify set-union representation */
void compact(graph *g);

/******* compact contraction functions *********/

/* compact contraction */
void compactContract(graph *g, node *v, node *w, int type);

/* the type option above says what to do when v and w have a common
 * neighbor.  we either merge v's edge into w's or vice versa. this
 * makes a difference if compact_contract is called while scanning v's
 * edges. type should be one of: */
enum {MERGE_INTO_V,     /* append w's arcs to v's */
      MERGE_INTO_W};     /* prepend w's arcs to v's */

#ifdef K
void compactContractLite(graph *g, node *v, node *w);
#endif

/******* other functions ***********************/

/* print current (contracted) graph */
void printGraph(graph *g); 

#ifdef ENABLE_GRAPH_BACKUPS
graph *makeBackupGraph(graph *g);

/* restore g from backup*/
void restoreBackupGraph(graph *g, graph *backup);

/* backups are not well formed, so need a special cleanup */
void freeBackupGraph(graph *backup);
#endif /* ENABLE_GRAPH_BACKUPS */

#endif
