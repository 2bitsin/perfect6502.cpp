#pragma once

template <typename Q>
struct range_iterator
{
	using value_type = Q;

	struct iterator_type
	{
		using value_type = Q;

		constexpr iterator_type() = default;
		constexpr iterator_type(value_type v): current(v) {}

		constexpr iterator_type(const iterator_type&) = default;
		constexpr iterator_type(iterator_type&&) = default;
		constexpr iterator_type& operator = (const iterator_type&) = default;
		constexpr iterator_type& operator = (iterator_type&&) = default;

		constexpr auto& operator *  (   ) const { return current; }
		constexpr auto& operator ++ (   ) const { ++current; return *this; }
		constexpr auto  operator ++ (int) const 
		{
			iterator_type tmp { current };
			++current;
			return tmp;
		}
		constexpr auto& operator -- (   ) const { --current; return *this; }
		constexpr auto  operator -- (int) const 
		{
			iterator_type tmp { current };
			--current;
			return tmp;
		}

		constexpr bool operator <=> (const iterator_type&) const = default;

	private:
		value_type current;
	};

	constexpr range_iterator(value_type _begin, value_type _end)
	: _begin	{ std::move (_begin) }, 
		_end		{ std::move	(_end)	 } 
	{}

	constexpr range_iterator() = default;
	constexpr range_iterator(const range_iterator&) = default;
	constexpr range_iterator(range_iterator&&) = default;
	constexpr range_iterator& operator = (const range_iterator&) = default;
	constexpr range_iterator& operator = (range_iterator&&) = default;

	constexpr auto begin()	const { return iterator_type { _begin	}	;	}
	constexpr auto end()		const { return iterator_type { _end		}	;	}
	constexpr auto cbegin() const { return iterator_type { _begin	}	;	}
	constexpr auto cend()		const { return iterator_type { _end		}	;	}

private:
	iterator_type _begin, _end;
};
