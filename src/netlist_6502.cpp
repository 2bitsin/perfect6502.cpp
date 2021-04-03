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
#include <vector>

#include "utils/bitmap.hpp"
#include "utils/array_list.hpp"
#include "utils/array_set.hpp"
#include "utils/range.hpp"
#include "utils/misc.hpp"

#include "types.hpp"
#include "netlist_6502.hpp"
#include "netlist_6502_labels.hpp"
#include "netlist_6502_transdefs.inl"

template <typename _Array, typename _Index, typename _Begin>
auto make_indexed_range(_Array&& array, _Index&& index, _Begin&& begin)
{
	return indexed_range(array, index[begin], index[begin+1]);
}

struct state_type
{
	bitmap<netlist_6502_node_count>	nodes_pullu;
	bitmap<netlist_6502_node_count>	nodes_pulld;
	bitmap<netlist_6502_node_count>	nodes_value;
	bitmap<netlist_6502_transistor_count>	is_connected;
	array_set<std::uint16_t, netlist_6502_node_count> group;
	array_set<std::uint16_t, netlist_6502_node_count> outputs;
	group_contains_value_t group_contains_value;
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

	if (!state.group.insert_unique (nindex))
		return;

	switch (state.group_contains_value)
	{
	case contains_nothing:	if (state.nodes_pulld.get (nindex)) inplace_max (state.group_contains_value, contains_pulldown);
	case contains_hi:				if (state.nodes_pullu.get (nindex)) inplace_max (state.group_contains_value, contains_pullup);
	case contains_pullup:		if (state.nodes_value.get (nindex)) inplace_max (state.group_contains_value, contains_hi);
	default:
		break;
	}
	/* revisit all transistors that control this node */	
	for (auto&& [tindex, nindex0] : make_indexed_range(node_bridge, node_bridge_index, nindex))
	{		 
		/* if the transistor connects c1 and c2... */
		if (state.is_connected.get (tindex))
			group_add_node (state, nindex0);			
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
		if (!state.nodes_value.try_set(nindex, new_value))
			continue;

		for (auto&& transistor : make_indexed_range (gate_to_transistor, gate_to_transistor_index, nindex))
			state.is_connected.set (transistor, new_value);

		auto&& node_deps_index	= new_value ? node_depends_lhs_index	: node_depends_rhs_index;
		auto&& node_deps				= new_value ? node_depends_lhs				: node_depends_rhs;

		for (auto&& nindex : make_indexed_range(node_deps, node_deps_index, nindex))
			state.outputs.insert_unique (nindex);
	}
}

static inline void
recalculate_node_list (state_type& state)
{
	/* loop limiter */
	static int max = 0;
	for (auto j : range (0, 100))
	{		
		if (state.outputs.empty ())
			break;
		auto inputs = state.outputs.as_array();
		state.outputs.clear ();

		/*
		 * for all nodes, follow their paths through
		 * turned-on transistors, find the state of the
		 * path and assign it to all nodes, and re-evaluate
		 * all transistors controlled by this path, collecting
		 * all nodes that changed because of it for the next run
		 */
		for (auto&& nindex : inputs)
			recalculate_node (state, nindex);
	}
	state.outputs.clear ();
}

template <auto... _Index, typename _New_value>
requires (sizeof...(_Index) <= sizeof(_New_value) * 8)
static inline void
write_nodes (state_type& state, _New_value value)
{
	_New_value not_value {};
	if constexpr (sizeof...(_Index) != 1)		
		not_value = _New_value(value ^ ~_New_value(0u));
	else
		not_value = !value;

	state.nodes_pullu.set_bits<_Index...>(value);
	state.nodes_pulld.set_bits<_Index...>(not_value);
	for (const auto index : { _Index ... })
		state.outputs.insert_unique (index);
}

template <auto... _Index, typename _New_value>
requires (sizeof...(_Index) <= sizeof(_New_value) * 8)
static inline void
read_nodes (const state_type& state, _New_value& value)
{
	value = state.nodes_value.get_bits<_New_value, _Index...>();
}

template <typename _New_value, auto... _Index>
static inline auto
read_nodes (const state_type& state) -> _New_value
{
	_New_value value{ 0u };
	read_nodes<_Index...> (state, value);
	return value;
}


netlist_6502::netlist_6502 ()
: state{ std::make_unique<state_type> () }
{
	auto& state = *this->state;

	state.nodes_pullu = netlist_6502_initial_state;
	state.nodes_pulld.clear ();
	state.nodes_value.clear ();
	state.is_connected.clear ();
	state.group.clear ();
	state.outputs.clear ();

	reset	(0);
	clock	(1);
	ready	(1);
	irq	(1);
	nmi	(1);
	so (1);

	for (auto index : range (0, netlist_6502_node_count))
		state.outputs.insert_unique (index);

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
	const auto value = read_nodes<uint8_t, P0, P1, P2, P3, P4, P5, P6, P7>(*state);
	return (value & 0b1100'1111) | 0b0010'0000;
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
	write_nodes<P0, P1, P2, P3>(*state, val & 0x0f);
	write_nodes<P6, P7>(*state, (val >> 6)&3);
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