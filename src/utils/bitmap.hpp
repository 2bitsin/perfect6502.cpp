/*
 Copyright (c) 2021 Aleksandr Ševčenko

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

#pragma once

#include <cstddef>
#include <cstdint>
#include <utility>
#include <initializer_list>
#include <algorithm>

#include "misc.hpp"

template <std::size_t _Num_bits, typename _Word_type = std::uint64_t>
struct bitmap
{
	using word_type = _Word_type;
	static inline constexpr auto num_bits = _Num_bits;
	static inline constexpr auto word_size = sizeof(word_type) * 8;
	static inline constexpr auto num_words = (num_bits + word_size - 1)/word_size;	

	constexpr bitmap(): store { 0u } {}
	constexpr bitmap(const bitmap& prev): bitmap(prev, std::make_index_sequence<num_words>{}) {}
	constexpr bitmap(const int (&values) [num_bits])
	: store { 0u }
	{
		for (auto i = 0u; i < num_bits; ++i)
			set (i, values[i]);
	}

	constexpr bitmap(std::initializer_list<int> values)
	:	store { 0u }
	{		
		const auto x = std::min (num_bits, values.size());
		for (auto i = 0u; i < x; ++i)
			set (i, *(values.begin() + i));
	}

	constexpr void set(std::size_t index, bool value)
	{
		index = index % num_bits;
		const auto m = word_type(1) << (index % word_size);
		if (value)
			store [index / word_size] |= m;
		else
			store [index / word_size] &= ~m;
	}

	constexpr bool get(std::size_t index) const
	{
		index = index % num_bits;
		const auto m = word_type(1) << (index % word_size);
		return !!(store [index / word_size] & m);
	}

	static constexpr auto size() { return num_bits ; }

	constexpr auto clear() 
	{
		for (auto&& cell : store) 
			cell = word_type { 0 };
	}

	template <typename _New_type, auto... _Index>
	requires (sizeof... (_Index) <= sizeof (_New_type) * 8 && sizeof... (_Index) > 1u)
	constexpr auto get_bits() const 
	{
		if constexpr (is_aligned_single_byte<_Index...>())
		{
			static constexpr auto bits_index = get_first_index<_Index...>();
			static constexpr auto word_index = bits_index / word_size;
			static constexpr auto word_shift = bits_index % word_size;
			return _New_type ((store[word_index] >> word_shift) & 0xffu);
		}
		else
		{
			auto value = _New_type { 0u };
			static constexpr auto q = sizeof...(_Index) - 1u;
			for (const auto index : { _Index ... })
			{
				value >>= 1u;
				value |= (get (index) << q);
			}
			return value;		
		}
	}

	template <auto... _Index, typename _New_type>
	requires (sizeof... (_Index) <= sizeof (_New_type) * 8 && sizeof... (_Index) > 1u)
	constexpr auto set_bits(_New_type&& value) 
	{
		if constexpr (is_aligned_single_byte<_Index...>())
		{
			static constexpr auto bits_index = get_first_index<_Index...>();
			static constexpr auto word_index = bits_index / word_size;
			static constexpr auto word_shift = bits_index % word_size;
			static constexpr auto mask = word_type{ 0xffu } << word_shift;
			store[word_index] = (store[word_index] & ~mask) | ((word_type { value } << word_shift) & mask);
		}
		else
		{
			auto value = _New_type { 0u };
			static constexpr auto q = sizeof...(_Index) - 1u;
			for (const auto index : { _Index ... })
			{
				value >>= 1u;
				value |= (get (index) << q);
			}
			return value;		
		}
	}


private:

	template <std::size_t ... I>
	constexpr bitmap(const bitmap& prev, std::index_sequence<I...>): store { prev.store[I]... } {}


	word_type store [num_words];
};