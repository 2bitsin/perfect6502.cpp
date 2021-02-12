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

#include <algorithm>

template <typename Q, typename ... U>
inline constexpr auto one_of(Q&& q, U&&... u)
{
	return ((u == q) || ... );
}

template <auto... U, typename Q>
inline constexpr auto one_of(Q&& q)
{	
	return ((U == q) || ... );
}

template <typename U, typename Q>
inline void inplace_max(U& u, Q&& q)
{
	if (q > u) 
		std::swap(q, u);
}

template <typename... _Index>
inline constexpr auto is_aligned_single_byte(_Index&&... index)
{
	using value_type = std::tuple_element_t<0, std::tuple<_Index...>>;
	value_type index_array [] = { index... };

	if (sizeof...(_Index) != 8u)
		return false;
	if (index_array [0] % 8)
		return false;
	for(auto i = 0u; i < sizeof...(_Index) - 1; ++i)
		if ((index_array[i + 1] - index_array[i]) != 1)
			return false;
	return true;
}

template <auto... _Index>
inline constexpr auto is_aligned_single_byte()
{
	return is_aligned_single_byte(_Index...);
}


template <auto _First, auto... _Rest>
inline constexpr auto get_first_index()
{
	return _First;
}

