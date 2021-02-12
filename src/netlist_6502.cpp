/*
 Copyright (c) 2010,2014,2021 Michael Steil, Brian Silverman, Barry Silverman, Aleksandr Ševčenko

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

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <initializer_list>
#include <stdexcept>

#include "utils/bitmap.hpp"
#include "utils/array_list.hpp"
#include "utils/array_set.hpp"
#include "utils/range.hpp"
#include "utils/misc.hpp"

#include "types.hpp"
#include "netlist_6502.hpp"
#include "netlist_6502.inl"


struct netlist_6502_static_state_type
{
	count_t	nodes_c1c2offset [netlist_6502_node_count + 1];
	std::pair<nodenum_t, nodenum_t> nodes_c1c2s [netlist_6502_transistor_count * 2];
	array_list<nodenum_t, netlist_6502_node_count> nodes_gates [netlist_6502_node_count];
	array_list<nodenum_t, netlist_6502_node_count> nodes_dependant [2][netlist_6502_node_count];

	constexpr netlist_6502_static_state_type ()
	{
		using namespace node_names;

		count_t c1c2count [netlist_6502_node_count];

		for (auto& list : nodes_gates					)	list.clear ();
		for (auto& list : nodes_dependant [0]	)	list.clear ();
		for (auto& list : nodes_dependant [1]	)	list.clear ();
		for (auto& node : nodes_c1c2offset		)	node = 0;
		for (auto& node : c1c2count						) node = 0;

		for (auto&& tindex : range (0, netlist_6502_transistor_count))
		{
			c1c2count [netlist_6502_transdefs [tindex].c1]++;
			c1c2count [netlist_6502_transdefs [tindex].c2]++;
		}

		/* then sum the counts to find each node's offset into the c1c2 array */
		count_t c1c2offset = 0u;
		for (auto&& nindex : range (0, netlist_6502_node_count + 1))
		{
			nodes_c1c2offset [nindex] = c1c2offset;
			if (nindex < netlist_6502_node_count)
				c1c2offset += c1c2count [nindex];
		}

		for (auto& node : c1c2count)
			node = 0;

		for (auto&& tindex : range<uint16_t> (0, netlist_6502_transistor_count))
		{
			auto&& [gate, c1, c2] = netlist_6502_transdefs [tindex];
			nodes_gates [gate].push (tindex);
			nodes_c1c2s [nodes_c1c2offset [c1] + c1c2count [c1]++] = { tindex, c2 };
			nodes_c1c2s [nodes_c1c2offset [c2] + c1c2count [c2]++] = { tindex, c1 };
		}

		for (auto&& nindex : range (0, netlist_6502_node_count))
		{
			for (auto&& tindex : nodes_gates [nindex])
			{
				auto&& [_, c1, c2] = netlist_6502_transdefs [tindex];

				const auto cond1 = !one_of<vss, vcc> (c1);
				const auto cond2 = !one_of<vss, vcc> (c2);

				if (cond1) nodes_dependant [0][nindex].push_unique (c1);
				if (cond2) nodes_dependant [0][nindex].push_unique (c2);
				nodes_dependant [1][nindex].push_unique (cond1 ? c1 : c2);
			}
		}
	}
};

static constexpr inline netlist_6502_static_state_type st_state;

struct state_type
{
	bitmap<netlist_6502_node_count>	nodes_pullup;
	bitmap<netlist_6502_node_count>	nodes_pulldn;
	bitmap<netlist_6502_node_count>	nodes_value;
	bitmap<netlist_6502_transistor_count>	trans_state;
	array_set<nodenum_t, netlist_6502_node_count> group;
	group_contains_value_t group_contains_value;
	array_set<nodenum_t, netlist_6502_node_count> list [2];
	unsigned in, out;
};

static inline void
group_add_node (state_type& state, nodenum_t nindex)
{
	/*
	 * We need to stop at vss and vcc, otherwise we'll revisit other groups
	 * with the same value - just because they all derive their value from
	 * the fact that they are connected to vcc or vss.
	 */
	if (nindex == node_names::vss)
	{
		state.group_contains_value = contains_vss;
		return;
	}

	if (nindex == node_names::vcc)
	{
		if (state.group_contains_value != contains_vss)
			state.group_contains_value = contains_vcc;
		return;
	}

	if (!state.group.insert (nindex))
		return;

	switch (state.group_contains_value)
	{
	case contains_nothing:	if (state.nodes_pulldn.get (nindex)) inplace_max (state.group_contains_value, contains_pulldown);
	case contains_hi:				if (state.nodes_pullup.get (nindex)) inplace_max (state.group_contains_value, contains_pullup);
	case contains_pullup:		if (state.nodes_value	.get (nindex)) inplace_max (state.group_contains_value, contains_hi);
	default:
		break;
	}

	/* revisit all transistors that control this node */
	auto&& offset = st_state.nodes_c1c2offset;
	for (auto&& tindex : range (offset [nindex], offset [nindex+1]))
	{
		auto&& [tindex0, nindex] = st_state.nodes_c1c2s [tindex];
		/* if the transistor connects c1 and c2... */
		if (state.trans_state.get (tindex0))
			group_add_node (state, nindex);
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

	for (auto&& nindex : state.group)
	{
		if (state.nodes_value.get (nindex) == new_value)
			continue;
		state.nodes_value.set (nindex, new_value);
		for (auto&& tindex : st_state.nodes_gates [nindex])
			state.trans_state.set (tindex, new_value);
		for (auto&& nindex : st_state.nodes_dependant [new_value][nindex])
			state.list [state.out].insert (nindex);
	}
}

static inline void
recalculate_node_list (state_type& state)
{
	/* loop limiter */
	for (auto j : range (0, 20))
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
		for (auto&& nindex : state.list [state.in])
			recalculate_node (state, nindex);
	}
	state.list [state.out].clear ();
}

template <auto... _Index, typename _Value>
requires (sizeof...(_Index) <= sizeof(_Value) * 8)
static inline void
write_nodes (state_type& state, _Value value)
{
	if constexpr (sizeof...(_Index) != 1)		
		state.nodes_pulldn.set_bits<_Index...>(_Value(value ^ ~_Value(0u)));
	else
		state.nodes_pulldn.set_bits<_Index...>(!value);
	state.nodes_pullup.set_bits<_Index...>(value);
	for (const auto index : { _Index ... })
		state.list [state.out].insert (index);
}

template <auto... _Index, typename _Value>
requires (sizeof...(_Index) <= sizeof(_Value) * 8)
static inline void
read_nodes (const state_type& state, _Value& value)
{
	value = state.nodes_value.get_bits<_Value, _Index...>();
}

template <typename _Value, auto... _Index>
static inline auto
read_nodes (const state_type& state) -> _Value
{
	_Value value{ 0u };
	read_nodes<_Index...> (state, value);
	return value;
}


netlist_6502::netlist_6502 ()
: state{ std::make_unique<state_type> () }
{
	auto& state = *this->state;

	state.nodes_pullup = netlist_6502_node_is_pullup;
	state.nodes_pulldn.clear ();
	state.nodes_value.clear ();
	state.trans_state.clear ();
	state.group.clear ();
	state.list [state.in  = 0].clear ();
	state.list [state.out = 1].clear ();

	reset	(0);
	clock	(1);
	ready	(1);
	irq	(1);
	nmi	(1);
	so (0);

	for (auto index : range (0, netlist_6502_node_count))
		state.list [state.out].insert (index);

	eval();
}

netlist_6502::~netlist_6502 ()
{ }


void netlist_6502::eval ()
{
	recalculate_node_list (*state);
}

auto netlist_6502::address () const -> std::uint16_t
{
	using namespace node_names;
	const auto lsb = read_nodes<uint8_t, ab0, ab1, ab2,  ab3,  ab4,  ab5,  ab6,  ab7 >(*state);
	const auto msb = read_nodes<uint8_t, ab8, ab9, ab10, ab11, ab12, ab13, ab14, ab15>(*state);
	return lsb + 0x100 * msb; 
				 
}

auto netlist_6502::data () const -> std::uint8_t
{
	using namespace node_names;
	return read_nodes<uint8_t, db0, db1, db2, db3, db4, db5, db6, db7>(*state);	
}

void netlist_6502::address (std::uint16_t val) 
{ 
	using namespace node_names;	
	write_nodes<ab0, ab1, ab2,  ab3,  ab4,  ab5,  ab6,  ab7 >(*state, (val >> 0u) & 0xffu);	 
	write_nodes<ab8, ab9, ab10, ab11, ab12, ab13, ab14, ab15>(*state, (val >> 8u) & 0xffu);	
}

void netlist_6502::data (std::uint8_t val)
{ 
	using namespace node_names;
	write_nodes<db0, db1, db2, db3, db4, db5, db6, db7>(*state, val);	
}

auto netlist_6502::clock () const -> bool
{
	return read_nodes<bool, node_names::clk0>(*state);	
}

void netlist_6502::clock (bool value)
{ 
	write_nodes<node_names::clk0>(*state, value);	
}

auto netlist_6502::ready () const -> bool
{
	return read_nodes<bool, node_names::rdy>(*state);
}

void netlist_6502::ready (bool value)
{ 
	write_nodes<node_names::rdy>(*state, value);
}

auto netlist_6502::nmi () const -> bool
{
	return read_nodes<bool, node_names::nmi>(*state);
}

void netlist_6502::nmi (bool value)
{ 
	write_nodes<node_names::nmi>(*state, value);
}

auto netlist_6502::irq () const -> bool
{
	return read_nodes<bool, node_names::irq>(*state);
}

void netlist_6502::irq (bool value)
{ 
	write_nodes<node_names::irq>(*state, value);
}

auto netlist_6502::reset () const -> bool
{
	return read_nodes<bool, node_names::res>(*state);	
}

void netlist_6502::reset (bool value)
{ 
	write_nodes<node_names::res>(*state, value);	
}

auto netlist_6502::read () const -> bool
{
	return read_nodes<bool, node_names::rw>(*state);	
}

void netlist_6502::read (bool value)
{ 
	write_nodes<node_names::rw>(*state, value);	
}

auto netlist_6502::sync () const -> bool
{
	return read_nodes<bool, node_names::sync_>(*state);
}

auto netlist_6502::so () const -> bool
{
	return read_nodes<bool, node_names::so>(*state);
}

void netlist_6502::sync (bool val) 
{
	return write_nodes<node_names::sync_>(*state, val);
}

void netlist_6502::so (bool val)
{ 
	return write_nodes<node_names::so>(*state, val);
}

auto netlist_6502::a () const -> std::uint8_t
{
	using namespace node_names;
	return read_nodes<uint8_t, a0, a1, a2, a3, a4, a5, a6, a7>(*state);
}

auto netlist_6502::x () const -> std::uint8_t
{
	using namespace node_names;
	return read_nodes<uint8_t, x0, x1, x2, x3, x4, x5, x6, x7>(*state);
}

auto netlist_6502::y () const -> std::uint8_t
{
	using namespace node_names;
	return read_nodes<uint8_t, y0, y1, y2, y3, y4, y5, y6, y7>(*state);
}

auto netlist_6502::s () const -> std::uint8_t
{
	using namespace node_names;
	return read_nodes<uint8_t, s0, s1, s2, s3, s4, s5, s6, s7>(*state);
}

auto netlist_6502::p () const -> std::uint8_t
{
	using namespace node_names;
	return read_nodes<uint8_t, p0, p1, p2, p3, p4, p5, p6, p7>(*state);
}

auto netlist_6502::pc () const -> std::uint16_t
{
	using namespace node_names;
	return pcl() + 0x100*pch();
}

auto netlist_6502::pch () const -> std::uint8_t
{
	using namespace node_names;
	return read_nodes<uint8_t, pch0, pch1, pch2, pch3, pch4, pch5, pch6, pch7>(*state);
}

auto netlist_6502::pcl () const -> std::uint8_t
{
	using namespace node_names;
	return read_nodes<uint8_t, pcl0, pcl1, pcl2, pcl3, pcl4, pcl5, pcl6, pcl7>(*state);
}

auto netlist_6502::ir () const -> std::uint8_t
{
	using namespace node_names;
	return read_nodes<uint8_t, notir0, notir1, notir2, notir3, notir4, notir5, notir6, notir7>(*state)^0xff;
}

void netlist_6502::a (std::uint8_t val)
{ 
	using namespace node_names;
	return write_nodes<a0, a1, a2, a3, a4, a5, a6, a7>(*state, val);
}

void netlist_6502::x (std::uint8_t val)
{ 
	using namespace node_names;
	return write_nodes<x0, x1, x2, x3, x4, x5, x6, x7>(*state, val);
}

void netlist_6502::y (std::uint8_t val)
{ 
	using namespace node_names;
	return write_nodes<y0, y1, y2, y3, y4, y5, y6, y7>(*state, val);
}

void netlist_6502::s (std::uint8_t val)
{ 
	using namespace node_names;
	return write_nodes<s0, s1, s2, s3, s4, s5, s6, s7>(*state, val);	
}

void netlist_6502::p (std::uint8_t val)
{ 
	using namespace node_names;
	return write_nodes<p0, p1, p2, p3, p4, p5, p6, p7>(*state, val);
}

void netlist_6502::pc (std::uint16_t val)
{ 
	using namespace node_names;
	pcl((val >> 0u) & 0xffu);
	pch((val >> 8u) & 0xffu);
}

void netlist_6502::pch (std::uint8_t val)
{ 
	using namespace node_names;
	return write_nodes<pch0, pch1, pch2, pch3, pch4, pch5, pch6, pch7>(*state, val);
}

void netlist_6502::pcl (std::uint8_t val)
{ 
	using namespace node_names;
	return write_nodes<pcl0, pcl1, pcl2, pcl3, pcl4, pcl5, pcl6, pcl7>(*state, val);
}

void netlist_6502::ir (std::uint8_t val)
{ 
	using namespace node_names;
	return write_nodes<notir0, notir1, notir2, notir3, notir4, notir5, notir6, notir7>(*state, std::uint8_t(val^0xff));
}
