#ifndef _TYPES_H_
#define _TYPES_H_

#include <cstddef>
#include <cstdint>

using nodenum_t		= uint16_t;
using transnum_t	= uint16_t;
using count_t			= uint16_t;

enum group_contains_value_t
{
	contains_nothing,
	contains_hi,
	contains_pullup,
	contains_pulldown,
	contains_vcc,
	contains_vss
};

struct netlist_transdefs 
{
	nodenum_t gate;
	nodenum_t c1;
	nodenum_t c2;
} ;


#endif