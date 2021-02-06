#ifndef _TYPES_H_
#define _TYPES_H_

#include <stddef.h>
#include <stdint.h>

typedef uint16_t nodenum_t;
typedef uint16_t transnum_t;
typedef uint16_t count_t;

typedef struct {
	int gate;
	int c1;
	int c2;
} netlist_transdefs;


#endif