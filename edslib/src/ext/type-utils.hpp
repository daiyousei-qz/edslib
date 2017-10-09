#pragma once
#include <type_traits>
#include <iterator>

// Additional type_traits
namespace eds::type
{
	namespace detail
	{
		template <typename TIter, typename TValue>
		inline constexpr bool IsIteratorOf()
		{
			using UnderlyingType = typename std::iterator_traits<TIter>::value_type;

			return std::is_same_v<UnderlyingType, TValue>;
		}
	}

	// is_iterator
	//

	// test via if category_tag could be found
	template <typename T, typename = void>
	struct is_iterator : public std::false_type { };
	template <typename T>
	struct is_iterator<T, std::void_t<typename std::iterator_traits<T>::iterator_category>>
		: public std::true_type { };

	template <typename T>
	constexpr bool is_iterator_v = is_iterator<T>::value;

	template <typename TIter, typename TValue>
	constexpr bool is_iterator_of_v = detail::IsIteratorOf<TIter, TValue>();
}