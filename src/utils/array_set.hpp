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

#include "bitmap.hpp"
#include "array_list.hpp"


template <typename _Value_type, std::size_t _Max_size>
struct array_set
{
	using value_type = _Value_type;
	static inline constexpr auto _capacity = _Max_size;

	constexpr array_set(): marks{}, store{} {}

	constexpr array_set(const array_set& prev): marks{prev.marks}, store{prev.store} {}

	constexpr array_set(std::initializer_list<value_type> values)
	{
		for(auto&& v: values)
			insert(std::move (v));
	}

	constexpr bool insert(value_type v)
	{
		if (!marks.get(v))
		{
			marks.set(v, true);
			store.push(std::move (v));
			return true;
		}
		return false;	
	}

	constexpr auto&& operator [] (std::size_t i) const
	{
		return store[i];
	}

	constexpr auto&& operator [] (std::size_t i) 
	{
		return store[i];
	}

	constexpr void clear()
	{
		marks.clear();
		store.clear();
	}

	constexpr auto size() const 
	{
		return store.size();
	}

	constexpr auto empty() const
	{
		return store.empty();
	}

	static constexpr auto capacity () 
	{
		return _capacity;
	}

	constexpr bool contains(const value_type& v)
	{
		return marks.get(v);
	}

	decltype(auto) begin() { return store.begin () ;}
	decltype(auto) begin() const { return store.begin () ;}
	decltype(auto) cbegin() const { return store.cbegin () ;}


	decltype(auto) end() { return store.end () ;}
	decltype(auto) end() const { return store.end () ;}
	decltype(auto) cend() const { return store.cend () ;}

private:
	bitmap<_capacity> marks;
	array_list<value_type, _capacity> store;
};