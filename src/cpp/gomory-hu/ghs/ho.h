/* HO stuff needed by pr.c */

#ifndef HO_H
#define HO_H

#define SOURCE 2
#define FROZEN 0
#define REGULAR 1
#define CONTRACTED 3

#ifndef NO_EXCESS_DETECTION
#define EXCESS_DETECTION
#endif

#ifndef NO_GLOBAL_UPDATES
#define GLOBAL_UPDATES
#endif

#ifdef CONTRACT
void  fixFlowForContraction(graph *g, node *v, node *w);
#endif

#endif
