#pragma once

#include <algorithm>

template <typename Q, typename ... U>
constexpr auto one_of(Q&& q, U&&... u)
{
	return ((u == q) || ... );
}

template <auto... U, typename Q>
constexpr auto one_of(Q&& q)
{	
	return ((U == q) || ... );
}

template <typename U, typename Q>
void inplace_max(U& u, Q&& q)
{
	if (q > u) 
		std::swap(q, u);
}
