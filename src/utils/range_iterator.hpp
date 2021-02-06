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