#pragma once
#include <type_traits>

// provides facilities not in standard library yet
namespace eds
{
	namespace detail {

		template<typename... TFuncs>
		struct VariantVisitor;

		template<typename F, typename... TFuncs>
		struct VariantVisitor<F, TFuncs...>
			: public F, public VariantVisitor<TFuncs...>
		{
			using F::operator ();
			using VariantVisitor<TFuncs...>::operator ();

			VariantVisitor(F f1, TFuncs... fs)
				: F(f1), VariantVisitor<TFuncs...>(fs...) {}
		};

		template<typename F>
		struct VariantVisitor<F> : public F
		{
			using F::operator ();

			VariantVisitor(F f) : F(f) {}
		};
	}

	template<class...Fs>
	auto MakeVariantVisitor(Fs&& ...fs)
	{
		using Visitor = detail::VariantVisitor<std::decay_t<Fs>...>;

		return Visitor{ std::forward<Fs>(fs)... };
	};
}