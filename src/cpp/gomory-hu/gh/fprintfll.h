/* A portable function to print long long ints. 
 */

#ifndef FPRINTFLL_H
#define FPRINTFLL_H

#include <stdio.h>

int fprintfll(FILE *fp, char *format, long long int val);

#endif
