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

template <typename Q>
struct range
{
	using value_type = Q;

	struct iterator
	{
		using value_type = Q;

		constexpr iterator() = default;
		constexpr iterator(value_type v): current(v) {}

		constexpr iterator(const iterator&) = default;
		constexpr iterator(iterator&&) = default;
		constexpr iterator& operator = (const iterator&) = default;
		constexpr iterator& operator = (iterator&&) = default;

		constexpr auto& operator *  () const { return current; }
		constexpr auto& operator ++ () { ++current; return *this; }
		constexpr auto& operator -- () { --current; return *this; }
		constexpr auto  operator ++ (int)  
		{
			iterator tmp { current };
			++current;
			return tmp;
		}
		constexpr auto  operator -- (int)
		{
			iterator tmp { current };
			--current;
			return tmp;
		}

		constexpr auto operator <=> (const iterator&) const = default;

	private:
		value_type current;
	};

	constexpr range(value_type _begin, value_type _end)
	: _begin	{ std::move (_begin) }, 
		_end		{ std::move	(_end)	 } 
	{}

	constexpr range() = default;
	constexpr range(const range&) = default;
	constexpr range(range&&) = default;
	constexpr range& operator = (const range&) = default;
	constexpr range& operator = (range&&) = default;

	constexpr auto begin()	const { return iterator { _begin	}	;	}
	constexpr auto end()		const { return iterator { _end		}	;	}
	constexpr auto cbegin() const { return iterator { _begin	}	;	}
	constexpr auto cend()		const { return iterator { _end		}	;	}

private:
	value_type _begin, _end;
};


template <typename _Lhs, typename _Rhs>
range(_Lhs&&, _Rhs&&) -> range<std::common_type_t<_Lhs, _Rhs>>;


template <typename _Index, typename _Array>
struct indexed_range
{
	using index_type = _Index;
	using array_type = _Array;

	struct iterator
	{
	
		constexpr iterator(array_type& _array, index_type _index)
		:	_array{ &_array },
			_index{ _index }
		{}
	
		constexpr iterator(const iterator&) = default;
		constexpr iterator(iterator&&) = default;

		constexpr iterator& operator = (const iterator& prev) {
			this->~iterator();
			new (this) iterator(prev._array, prev._index);
			return *this;
		};

		constexpr iterator& operator = (iterator&&) = default;
	
		constexpr auto& operator *  () const { return (*_array) [_index]; }
		constexpr auto& operator ++ () { ++_index; return *this; }
		constexpr auto& operator -- () { --_index; return *this; }

		constexpr auto operator ++ (int)  
		{
			iterator tmp { *this };
			++*this;
			return tmp;
		}
		constexpr auto operator -- (int)
		{
			iterator tmp { *this };
			--*this;
			return tmp;
		}

		constexpr bool operator <		(const iterator& rhs) const { return _index <  rhs._index; }
		constexpr bool operator ==	(const iterator& rhs) const { return _index == rhs._index; }

		constexpr auto operator <=> (const iterator&) const = default;
	
	private:
		array_type* _array;
		index_type  _index;
	};


	constexpr indexed_range(array_type& _array, index_type _begin, index_type _end)
	: _array	{ _array },
		_begin	{ std::move (_begin) }, 
		_end		{ std::move	(_end)	 } 
	{}

	constexpr indexed_range() = default;
	constexpr indexed_range(const indexed_range&) = default;
	constexpr indexed_range(indexed_range&&) = default;
	constexpr indexed_range& operator = (const indexed_range&) = default;
	constexpr indexed_range& operator = (indexed_range&&) = default;

	constexpr auto begin()	const { return iterator( _array, _begin	)	;	}
	constexpr auto end()		const { return iterator( _array, _end		)	;	}
	constexpr auto cbegin() const { return iterator( _array, _begin	)	;	}
	constexpr auto cend()		const { return iterator( _array, _end		)	;	}

private:
	array_type& _array;
	index_type	_begin;
	index_type	_end;
};

template <typename _Array, typename _Lhs, typename _Rhs>
indexed_range(_Array&, _Lhs&&, _Rhs&&) -> indexed_range<_Array, std::common_type_t<_Lhs, _Rhs>>;
