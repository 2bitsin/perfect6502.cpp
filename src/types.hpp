#ifndef _TYPES_H_
#define _TYPES_H_

#include <cstddef>
#include <cstdint>

typedef uint16_t nodenum_t;
typedef uint16_t transnum_t;
typedef uint16_t count_t;

enum group_contains_value_t
{
	contains_nothing,
	contains_hi,
	contains_pullup,
	contains_pulldown,
	contains_vcc,
	contains_vss
};

typedef struct {
	uint16_t gate;
	uint16_t c1;
	uint16_t c2;
} netlist_transdefs;


typedef struct
{
	transnum_t transistor;
	nodenum_t other_node;
} c1c2_t;


#endif