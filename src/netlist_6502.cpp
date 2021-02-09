/*
 Copyright (c) 2010,2014 Michael Steil, Brian Silverman, Barry Silverman

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
*/

/************************************************************
 *
 * Libc Functions and Basic Data Types
 *
 ************************************************************/

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <initializer_list>
#include <stdexcept>

#include "utils/bitmap.hpp"
#include "utils/array_list.hpp"
#include "utils/array_set.hpp"
#include "utils/range_iterator.hpp"
#include "utils/misc.hpp"

#include "types.hpp"
#include "netlist_6502.hpp"
#include "netlist_6502.inl"


struct netlist_6502_static_state_type
{
	count_t	nodes_c1c2offset [netlist_6502_node_count + 1];
	c1c2_t nodes_c1c2s [netlist_6502_transistor_count * 2];

	array_list<nodenum_t, netlist_6502_node_count> nodes_gates [netlist_6502_node_count];
	array_set<nodenum_t, netlist_6502_node_count> nodes_dependant [netlist_6502_node_count];
	array_set<nodenum_t, netlist_6502_node_count> nodes_left_dependant [netlist_6502_node_count];

	constexpr netlist_6502_static_state_type ()
	{
		using namespace node_names;

		count_t c1c2count [netlist_6502_node_count];

		for (auto& list : nodes_gates						)	list.clear ();
		for (auto& list : nodes_dependant				)	list.clear ();
		for (auto& list : nodes_left_dependant	)	list.clear ();
		for (auto& node : nodes_c1c2offset			)	node = 0;
		for (auto& node : c1c2count							) node = 0;
				
		for (auto&& i : range (0, netlist_6502_transistor_count))
		{
			c1c2count [netlist_6502_transdefs [i].c1]++;
			c1c2count [netlist_6502_transdefs [i].c2]++;
		}

		for (auto&& transistor : range (0u, netlist_6502_transistor_count))
			nodes_gates [netlist_6502_transdefs [transistor].gate].push (transistor);

		/* then sum the counts to find each node's offset into the c1c2 array */
		count_t c1c2offset = 0;
		for (auto&& i : range (0, netlist_6502_node_count + 1))
		{
			nodes_c1c2offset [i] = c1c2offset;
			if (i < netlist_6502_node_count)
				c1c2offset += c1c2count [i];
		}

		for (auto& node : c1c2count)
			node = 0;

		for (auto&& node_index : range<uint16_t> (0, netlist_6502_transistor_count))
		{
			nodenum_t c1 = netlist_6502_transdefs [node_index].c1;
			nodenum_t c2 = netlist_6502_transdefs [node_index].c2;
			nodes_c1c2s [nodes_c1c2offset [c1] + c1c2count [c1]++] = c1c2_t{ node_index, c2 };
			nodes_c1c2s [nodes_c1c2offset [c2] + c1c2count [c2]++] = c1c2_t{ node_index, c1 };
		}

		for (auto&& node_index : range (0, netlist_6502_node_count))
		{
			for (auto&& transistor : nodes_gates [node_index])
			{
				const auto c1 = netlist_6502_transdefs [transistor].c1;
				const auto c2 = netlist_6502_transdefs [transistor].c2;

				const auto cond1 = !one_of<vss, vcc> (c1);
				const auto cond2 = !one_of<vss, vcc> (c2);

				if (cond1) nodes_dependant [node_index].insert (c1);
				if (cond2) nodes_dependant [node_index].insert (c2);

				nodes_left_dependant [node_index].insert (cond1 ? c1 : c2);
			}
		}
	}
};

static constexpr inline netlist_6502_static_state_type st_state;


struct state_type
{
	std::uint8_t memory[0x10000];

	bitmap<netlist_6502_node_count>	nodes_pullup;
	bitmap<netlist_6502_node_count>	nodes_pulldown;
	bitmap<netlist_6502_node_count>	nodes_value;

	bitmap<netlist_6502_transistor_count>	trans_state;

	array_set<nodenum_t, netlist_6502_node_count> group;
	group_contains_value_t group_contains_value;

	unsigned in, out;
	array_set<nodenum_t, netlist_6502_node_count> list [2];
};


void initialize_state (state_type& state)
{
	using namespace node_names;

	/* allocate state */
	std::memset(state.memory, 0, sizeof(state.memory));
	state.nodes_pullup = netlist_6502_node_is_pullup;
	state.nodes_pulldown.clear ();
	state.nodes_value.clear ();
	state.trans_state.clear ();
	state.group.clear ();
	state.list [0].clear ();
	state.list [1].clear ();
	state.in = 0;
	state.out = 1;
}


static inline void group_add_node (state_type& state, nodenum_t n)
{
	/*
	 * We need to stop at vss and vcc, otherwise we'll revisit other groups
	 * with the same value - just because they all derive their value from
	 * the fact that they are connected to vcc or vss.
	 */
	if (n == node_names::vss)
	{
		state.group_contains_value = contains_vss;
		return;
	}
	if (n == node_names::vcc)
	{
		if (state.group_contains_value != contains_vss)
			state.group_contains_value = contains_vcc;
		return;
	}

	if (state.group.contains (n))
		return;

	state.group.insert (n);

	switch (state.group_contains_value)
	{
	case contains_nothing:	if (state.nodes_pulldown.get (n)) inplace_max (state.group_contains_value, contains_pulldown);
	case contains_hi:				if (state.nodes_pullup.get (n))	inplace_max (state.group_contains_value, contains_pullup);
	case contains_pullup:		if (state.nodes_value.get (n))	inplace_max (state.group_contains_value, contains_hi);
	default:
		break;
	}

	/* revisit all transistors that control this node */
	const auto& offs = st_state.nodes_c1c2offset;
	for (auto&& transistor : range (offs [n], offs [n + 1]))
	{
		auto c = st_state.nodes_c1c2s [transistor];
		/* if the transistor connects c1 and c2... */
		if (state.trans_state.get (c.transistor))
			group_add_node (state, c.other_node);
	}
}

static inline void
group_add_all_nodes (state_type& state, nodenum_t node)
{
	state.group.clear ();
	state.group_contains_value = contains_nothing;
	group_add_node (state, node);
}

static inline void
recalculate_node (state_type& state, nodenum_t node)
{
	/*
	 * get all nodes that are connected through
	 * transistors, starting with this one
	 */
	group_add_all_nodes (state, node);

	/* get the state of the group */

	const bool new_value{
		one_of<contains_vcc, contains_pullup, contains_hi> (state.group_contains_value)
	};

	/*
	 * - set all nodes to the group state
	 * - check all transistors switched by nodes of the group
	 * - collect all nodes behind toggled transistors
	 *   for the next run
	 */
	for (auto&& node_index : state.group)
	{
		if (state.nodes_value.get (node_index) == new_value)
			continue;

		state.nodes_value.set (node_index, new_value);

		for (auto&& trans : st_state.nodes_gates [node_index])
			state.trans_state.set (trans, new_value);

		auto& dependant = new_value ? st_state.nodes_left_dependant [node_index] 
																: st_state.nodes_dependant [node_index];

		for (auto&& node : dependant)
			state.list [state.out].insert (node);
	}
}

void
recalculate_node_list (state_type& state)
{
	/* loop limiter */
	for (auto j : range(0, 20))
	{			
		/*
		 * make the secondary list our primary list, use
		 * the data storage of the primary list as the
		 * secondary list
		 */
		std::swap (state.in, state.out);

		if (state.list [state.in].empty ())
			break;
		state.list [state.out].clear ();

		/*
		 * for all nodes, follow their paths through
		 * turned-on transistors, find the state of the
		 * path and assign it to all nodes, and re-evaluate
		 * all transistors controlled by this path, collecting
		 * all nodes that changed because of it for the next run
		 */
		for (auto&& node : state.list [state.in])
			recalculate_node (state, node);
	}
	state.list [state.out].clear ();
}

void
stabilize_chip (state_type& state)
{
	for (auto index: range(0, netlist_6502_node_count))
		state.list [state.out].insert (index);

	recalculate_node_list (state);
}

void 
set_node (state_type& state, nodenum_t nn, int s)
{
	state.nodes_pullup.set (nn, s);
	state.nodes_pulldown.set (nn, !s);
	state.list [state.out].insert (nn);

	recalculate_node_list (state);
}

bool 
get_node(state_type& state, nodenum_t nn)
{
	return state.nodes_value.get(nn);
}

unsigned int
read_nodes (state_type* state, std::initializer_list<nodenum_t> nodelist)
{
	int result = 0;
	for (long long i = nodelist.size () - 1; i >= 0; i--)
	{
		result <<= 1;
		result |= (int)state->nodes_value.get (*(nodelist.begin () + i));
	}
	return result;
}

template <auto... _Index, typename _Value>
void write_nodes (state_type& state, _Value value)
{
	for(const auto index: { _Index ... })
	{
		set_node (state, index, value & 1u);
		value >>= 1u;
	}
}

template <auto... _Index, typename _Value>
void read_nodes (state_type& state, _Value& value)
{
	static constexpr auto q = sizeof...(_Index) - 1u;
	for(const auto index: { _Index ... })
	{
		value >>= 1u;
		value |= (get_node (state, index) << q);	
	}	
}

template <typename _Value, auto... _Index>
auto read_nodes (state_type& state) -> _Value
{
	_Value value { 0u };
	read_nodes<_Index...>(state, value);
	return value;
}

static inline void
handle_memory (netlist_6502& nlsym)
{
	using namespace node_names;
	if (get_node (*nlsym.state, rw))
		nlsym.set(nlsym.bus_data, nlsym.memory()[nlsym.get(nlsym.bus_addr)]);
	else
		nlsym.memory()[nlsym.get(nlsym.bus_addr)] = nlsym.get(nlsym.bus_data);
}

using namespace node_names;
void init_and_reset_chip (netlist_6502& nl)
{
	using namespace node_names;
	/* set up data structures for efficient emulation */
		
	initialize_state (*nl.state);

	set_node (*nl.state, res, 0);
	set_node (*nl.state, clk0, 1);
	set_node (*nl.state, rdy, 1);
	set_node (*nl.state, so, 0);
	set_node (*nl.state, irq, 1);
	set_node (*nl.state, nmi, 1);

	stabilize_chip (*nl.state);

	/* hold RESET for 8 cycles */
	for (int i = 0; i < 16; i++)
		nl.step ();

	/* release RESET */
	set_node (*nl.state, res, 1);
	recalculate_node_list (*nl.state);

}

netlist_6502::netlist_6502 ()
:	state { std::make_unique<state_type>() }
{ 
	init_and_reset_chip(*this); 
}

netlist_6502::~netlist_6502 ()
{ }

void netlist_6502::step ()
{
	using namespace node_names;
	int clk = get_node(*state, clk0);

	/* invert clock */
	set_node (*state, clk0, !clk);
	recalculate_node_list (*state);

	/* handle memory reads and writes */
	if (!clk)
		handle_memory (*this);
}

auto netlist_6502::get (bits _bits) const -> uint16_t
{
	switch(_bits)
	{
	case bits::bus_addr:
		return read_nodes<uint16_t, ab0, ab1, ab2, ab3, ab4, ab5, ab6, ab7, ab8, ab9, ab10, ab11, ab12, ab13, ab14, ab15>(*state);
	case bits::bus_data:
		return read_nodes<uint8_t, db0, db1, db2, db3, db4, db5, db6, db7>(*state);
	case bits::bus_nmi:
		return read_nodes<uint8_t, nmi>(*state);
	case bits::bus_reset:
		return read_nodes<uint8_t, res>(*state);
	case bits::bus_irq:
		return read_nodes<uint8_t, irq>(*state);
	case bits::bus_rw:
		return read_nodes<uint8_t, rw>(*state);
	case bits::bus_sync:
		return read_nodes<uint8_t, sync_>(*state);
	case bits::bus_clock:
		return read_nodes<uint8_t, clk0>(*state);
	case bits::bus_ready:
		return read_nodes<uint8_t, rdy>(*state);
	case bits::reg_a:	
		return read_nodes<uint8_t, a0, a1, a2, a3, a4, a5, a6, a7>(*state);
	case bits::reg_x:
		return read_nodes<uint8_t, x0, x1, x2, x3, x4, x5, x6, x7>(*state);
	case bits::reg_y:
		return read_nodes<uint8_t, y0, y1, y2, y3, y4, y5, y6, y7>(*state);
	case bits::reg_s:
		return read_nodes<uint8_t, s0, s1, s2, s3, s4, s5, s6, s7>(*state);
	case bits::reg_p:
		return read_nodes<uint8_t, p0, p1, p2, p3, p4, p5, p6, p7>(*state);
	case bits::reg_pc:
		return read_nodes<uint16_t, pcl0, pcl1, pcl2, pcl3, pcl4, pcl5, pcl6, pcl7, pch0, pch1, pch2, pch3, pch4, pch5, pch6, pch7>(*state);
	case bits::reg_pch:
		return read_nodes<uint8_t, pch0, pch1, pch2, pch3, pch4, pch5, pch6, pch7>(*state);
	case bits::reg_pcl:
		return read_nodes<uint8_t, pcl0, pcl1, pcl2, pcl3, pcl4, pcl5, pcl6, pcl7>(*state);
	case bits::reg_ir:
		return read_nodes<uint8_t, notir0, notir1, notir2, notir3, notir4, notir5, notir6, notir7>(*state)^0xffu;
	default:
		throw std::runtime_error("Not implemented.");
	}
}

void netlist_6502::set (bits _bits, uint16_t val)
{ 
	switch(_bits)
	{
	case bits::bus_addr:
		return write_nodes<ab0, ab1, ab2, ab3, ab4, ab5, ab6, ab7, ab8, ab9, ab10, ab11, ab12, ab13, ab14, ab15>(*state, val);
	case bits::bus_data:
		return write_nodes<db0, db1, db2, db3, db4, db5, db6, db7>(*state, val);
	case bits::bus_nmi:
		return write_nodes<nmi>(*state, val);
	case bits::bus_reset:
		return write_nodes<res>(*state, val);
	case bits::bus_irq:
		return write_nodes<irq>(*state, val);
	case bits::bus_rw:
		return write_nodes<rw>(*state, val);
	case bits::bus_sync:
		return write_nodes<sync_>(*state, val);
	case bits::bus_clock:
		return write_nodes<clk0>(*state, val);
	case bits::bus_ready:
		return write_nodes<rdy>(*state, val);
	case bits::reg_a:	
		return write_nodes<a0, a1, a2, a3, a4, a5, a6, a7>(*state, val);
	case bits::reg_x:
		return write_nodes<x0, x1, x2, x3, x4, x5, x6, x7>(*state, val);
	case bits::reg_y:
		return write_nodes<y0, y1, y2, y3, y4, y5, y6, y7>(*state, val);
	case bits::reg_s:
		return write_nodes<s0, s1, s2, s3, s4, s5, s6, s7>(*state, val);
	case bits::reg_p:
		return write_nodes<p0, p1, p2, p3, p4, p5, p6, p7>(*state, val);
	case bits::reg_pc:
		return write_nodes<pcl0, pcl1, pcl2, pcl3, pcl4, pcl5, pcl6, pcl7, pch0, pch1, pch2, pch3, pch4, pch5, pch6, pch7>(*state, val);
	case bits::reg_pch:
		return write_nodes<pch0, pch1, pch2, pch3, pch4, pch5, pch6, pch7>(*state, val);
	case bits::reg_pcl:
		return write_nodes<pcl0, pcl1, pcl2, pcl3, pcl4, pcl5, pcl6, pcl7>(*state, val);
	case bits::reg_ir:
		return write_nodes<notir0, notir1, notir2, notir3, notir4, notir5, notir6, notir7>(*state, uint8_t (val^0xffu));
	default:
		throw std::runtime_error("Not implemented.");
	}
}

auto netlist_6502::memory () -> std::span<std::uint8_t>
{
	return std::span<std::uint8_t> (state->memory);
}
