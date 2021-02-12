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

#include <cstdint>
#include <cstddef>
#include <cassert>
#include <initializer_list>
#include <algorithm>

#include "range.hpp"

template <typename _Value_type, std::size_t _Max_size>
struct array_list
{
	using value_type = _Value_type;
	
	constexpr array_list()
	: _size	{ 0u },
		_data { } 
	{}

	constexpr array_list(const array_list& prev)
	: _size { prev._size }		
	{
		for (auto i = 0u; i < prev._size; ++i)
			_data[i] = prev._data[i];
	}

	constexpr array_list(std::initializer_list<value_type> values)
	: _size { values.size() }		
	{
		auto to_copy = std::min(values.size(), capacity());
		for (auto i = 0u; i < to_copy; ++i)
			_data[i] = *(values.begin() + i);
	}

	constexpr auto&& operator [] (std::size_t i) const
	{
		assert(i < _size);
		return _data[i];
	}

	constexpr auto&& operator [] (std::size_t i) 
	{
		assert(i < _size);
		return _data[i];
	}

	constexpr void push(value_type v) 
	{
		assert(_size < capacity());
		_data[_size++] = std::move(v);				
	}

	constexpr void insert_unique_linear(value_type v) 
	{
		assert(_size < capacity());
		if (contains(v))
			return;
		_data[_size++] = std::move(v);				
	}


	constexpr void pop()
	{
		assert(_size > 0);
		--_size;
	}

	constexpr void clear()
	{
		_size = 0;
	}

	constexpr auto size() const 
	{
		return _size;
	}

	constexpr auto empty() const
	{
		return size () == 0;
	}

	static constexpr auto capacity () 
	{
		return _Max_size;
	}

	constexpr void swap (array_list& rhs)
	{
		auto to_swap = std::max(size(), rhs.size());
		for (auto i = 0u; i < to_swap; ++i)
			std::swap(_data[i], rhs._data[i]);
		std::swap(_size, rhs._size);
	}

	constexpr std::size_t count(const value_type& value) const
	{
		std::size_t _count { 0u };
		for (auto&& curr : *this)
			if (curr == value)
				++_count;
		return _count;
	}

	constexpr bool contains(const value_type& value) const
	{
		for (auto&& curr : *this)
			if (curr == value)
				return true;
		return false;
	}

	constexpr auto* begin() 
	{
		return &_data[0];
	}

	constexpr auto* begin() const 
	{
		return &_data[0];
	}

	constexpr auto* cbegin() const 
	{
		return &_data[0];
	}

	constexpr auto* end() 
	{
		return &_data[size()];
	}

	constexpr auto* end() const 
	{
		return &_data[size()];
	}

	constexpr auto* cend() const 
	{
		return &_data[size()];
	}

	constexpr auto indexes() const 
	{
		return range<std::size_t> { 0u, size() };
	}

	constexpr auto insert_unique(value_type value)
	{
		if (size() >= capacity ())
			return false;

		auto pos = (std::size_t)std::distance(begin(), 
			std::lower_bound(begin(), end(), value));

		if (pos < size() && _data[pos] == value)
			return false;

		push (std::move (value));

		for (std::size_t j = size(); j > pos + 1; --j)
			std::swap(_data[j - 1],  _data[j - 2]);

		return true;
	}

private:
	std::size_t _size { 0u };
	value_type _data [_Max_size] { };
};

