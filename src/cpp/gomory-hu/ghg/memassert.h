/* homegrown assert specifically for checking allocations.
 */

#ifndef MEMASSERT_H
#define MEMASSERT_H

#include <stdio.h>
#include <stdlib.h>

#define memassert(x) { if ((x)==NULL) {fprintf(stderr, "Memory exhausted at " __FILE__ ":%d " #x "\n", __LINE__); exit(1); } }

/* hack to deal with fact that checker doesn't like assert */
#ifdef __CHECKER__
#define assert(x) { if (!(x)) {fprintf(stderr, "Assertion failed at " __FILE__ ":%d " #x "\n", __LINE__); exit(1); } }
#endif

#endif
