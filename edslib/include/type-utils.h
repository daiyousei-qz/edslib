#pragma once
#include <type_traits>
#include <iterator>

// A sample implementation of concepts mentioned in https://www.youtube.com/watch?v=PFdWqa68LmA
namespace eds::type
{
    // ==================================================================
    // Helpful Structs
    //

    template <typename T>
    struct AlwaysTrue : std::true_type
    {
    };
    template <typename T>
    struct AlwaysFalse : std::false_type
    {
    };

    // ==================================================================
    // Type Checker
    //
    namespace detail
    {
        // CONCEPT:
        // A checker type should inherts from TypeChecker
        // and implement bool Evaluate<T>()
        struct TypeChecker
        {
        };

        template <typename... Ps>
        inline constexpr bool IsTypeChecker()
        {
            return std::conjunction_v<
                std::is_base_of<TypeChecker, Ps>...>;
        }

        template <typename... Ps>
        using EnsureTypeChecker = std::enable_if_t<IsTypeChecker<Ps...>()>;

        // Basic Compositions
        //

        template <typename P1, typename P2>
        struct And : public TypeChecker
        {
            template <typename T>
            static constexpr bool Evaluate()
            {
                return P1::template Evaluate<T>() && P2::template Evaluate<T>();
            }
        };

        template <typename P1, typename P2>
        struct Or : public TypeChecker
        {
            template <typename T>
            static constexpr bool Evaluate()
            {
                return P1::template Evaluate<T>() || P2::template Evaluate<T>();
            }
        };

        template <typename P>
        struct Not : public TypeChecker
        {
            template <typename T>
            static constexpr bool Evaluate()
            {
                return !P::template Evaluate<T>();
            }
        };

        // a generic type checker that adapt to STL bool_constant interface
        template <template <typename> typename Pred>
        struct GenericTypeChecker : public TypeChecker
        {
            template <typename T>
            static constexpr bool Evaluate()
            {
                static_assert(std::is_same_v<Pred<T>::value_type, bool>);

                return Pred<T>::value;
            }
        };
    }

    // Operator overloads
    //

    template <
        typename P1,
        typename P2,
        typename = detail::EnsureTypeChecker<P1, P2>>
    inline constexpr auto operator&&(P1, P2)
    {
        return detail::And<P1, P2>{};
    }

    template <
        typename P1,
        typename P2,
        typename = detail::EnsureTypeChecker<P1, P2>>
    inline constexpr auto operator||(P1, P2)
    {
        return detail::Or<P1, P2>{};
    }

    template <
        typename P,
        typename = detail::EnsureTypeChecker<P>>
    inline constexpr auto operator!(P)
    {
        return detail::Not<P>{};
    }

    // Generic type checker
    //
    template <template <typename> typename Pred>
    static constexpr auto generic_check = detail::GenericTypeChecker<Pred>{};

    // Evaluation function
    //
    template <
        typename T,
        typename P,
        typename = detail::EnsureTypeChecker<P>>
    static constexpr bool Constraint(P)
    {
        return P::template Evaluate<T>();
    }

    // ==================================================================
    // Wrapper for standard type_traits
    //

    // Primary type categories
    //
    static constexpr auto is_void                    = generic_check<std::is_void>;
    static constexpr auto is_null_pointer            = generic_check<std::is_null_pointer>;
    static constexpr auto is_integral                = generic_check<std::is_integral>;
    static constexpr auto is_floating_point          = generic_check<std::is_floating_point>;
    static constexpr auto is_array                   = generic_check<std::is_array>;
    static constexpr auto is_enum                    = generic_check<std::is_enum>;
    static constexpr auto is_union                   = generic_check<std::is_union>;
    static constexpr auto is_class                   = generic_check<std::is_class>;
    static constexpr auto is_function                = generic_check<std::is_function>;
    static constexpr auto is_pointer                 = generic_check<std::is_pointer>;
    static constexpr auto is_lvalue_reference        = generic_check<std::is_lvalue_reference>;
    static constexpr auto is_rvalue_reference        = generic_check<std::is_rvalue_reference>;
    static constexpr auto is_member_object_pointer   = generic_check<std::is_member_object_pointer>;
    static constexpr auto is_member_function_pointer = generic_check<std::is_member_function_pointer>;

    // Composite type categories
    //
    static constexpr auto is_fundamental    = generic_check<std::is_fundamental>;
    static constexpr auto is_arithmetic     = generic_check<std::is_arithmetic>;
    static constexpr auto is_scalar         = generic_check<std::is_scalar>;
    static constexpr auto is_object         = generic_check<std::is_object>;
    static constexpr auto is_compound       = generic_check<std::is_compound>;
    static constexpr auto is_reference      = generic_check<std::is_reference>;
    static constexpr auto is_member_pointer = generic_check<std::is_member_pointer>;

    // Type properties
    //
    static constexpr auto is_const              = generic_check<std::is_const>;
    static constexpr auto is_volatile           = generic_check<std::is_volatile>;
    static constexpr auto is_trivial            = generic_check<std::is_trivial>;
    static constexpr auto is_trivially_copyable = generic_check<std::is_trivially_copyable>;
    static constexpr auto is_standard_layout    = generic_check<std::is_standard_layout>;
    static constexpr auto is_pod                = generic_check<std::is_pod>;
    static constexpr auto is_empty              = generic_check<std::is_empty>;
    static constexpr auto is_polymorphic        = generic_check<std::is_polymorphic>;
    static constexpr auto is_abstract           = generic_check<std::is_abstract>;
    static constexpr auto is_final              = generic_check<std::is_final>;
    static constexpr auto is_signed             = generic_check<std::is_signed>;
    static constexpr auto is_unsigned           = generic_check<std::is_unsigned>;

    // Type relationships
    //
    namespace detail
    {
        template <typename... Us>
        struct SameToAny : public TypeChecker
        {
            template <typename T>
            static constexpr bool Evaluate()
            {
                return std::conjunction_v<std::is_same<T, Us>...>;
            }
        };

        template <typename... BasePack>
        struct DeriveFromAny : public TypeChecker
        {
            template <typename T>
            static constexpr bool Evaluate()
            {
                return std::conjunction_v<std::is_base_of<BasePack, T>...>;
            }
        };

        template <typename... Us>
        struct ConvertibleToAny : public TypeChecker
        {
            template <typename T>
            static constexpr bool Evaluate()
            {
                return std::conjunction_v<std::is_convertible<T, Us>...>;
            }
        };

        template <typename... TArgs>
        struct Invocable : public TypeChecker
        {
            template <typename T>
            static constexpr bool Evaluate()
            {
                return std::is_invocable_v<T, TArgs...>;
            }
        };

        template <typename... TArgs>
        struct Invocable_R : public TypeChecker
        {
            template <typename T>
            static constexpr bool Evaluate()
            {
                return std::is_invocable_r_v<T, TArgs...>;
            }
        };
    }

    template <typename U>
    static constexpr auto same_to = detail::SameToAny<U>();
    template <typename... Us>
    static constexpr auto same_to_any = detail::SameToAny<Us...>();
    template <typename Base>
    static constexpr auto derive_from = detail::DeriveFromAny<Base>();
    template <typename... BasePack>
    static constexpr auto derived_from_any = detail::DeriveFromAny<BasePack...>();
    template <typename U>
    static constexpr auto convertible_to = detail::ConvertibleToAny<U>();
    template <typename... Us>
    static constexpr auto convertible_to_any = detail::ConvertibleToAny<Us...>();
    template <typename... TArgs>
    static constexpr auto invocable = detail::Invocable<TArgs...>();
    template <typename... TArgs>
    static constexpr auto invocable_r = detail::Invocable_R<TArgs...>();

    // ==================================================================
    // Additional type traits
    //

    namespace detail
    {
        template <typename U>
        struct SimilarTo : public TypeChecker
        {
            template <typename T>
            static constexpr bool Evaluate()
            {
                return std::is_same_v<std::decay_t<U>, std::decay_t<T>>;
            }
        };

        // test via if category_tag could be found
        template <typename T, typename = void>
        struct is_iterator : public std::false_type
        {
        };
        template <typename T>
        struct is_iterator<T, std::void_t<typename std::iterator_traits<T>::iterator_category>>
            : public std::true_type
        {
        };

        struct IsIterator : public TypeChecker
        {
            template <typename T>
            static constexpr bool Evaluate()
            {
                return is_iterator<T>::value;
            }
        };

        template <typename U>
        struct IsIteratorOf : public TypeChecker
        {
            template <typename T>
            static constexpr bool Evaluate()
            {
                using UnderlyingType = typename std::iterator_traits<T>::value_type;

                return std::is_same_v<UnderlyingType, U>;
            }
        };

        template <typename U>
        struct IsEnumOf : public TypeChecker
        {
            template <typename T>
            static constexpr bool Evaluate()
            {
                if constexpr (std::is_enum_v<T>)
                {
                    return std::is_same_v<std::underlying_type_t<T>, U>;
                }
                else
                {
                    return false;
                }
            }
        };

        template <typename U>
        struct IsReferenceOf : public TypeChecker
        {
            template <typename T>
            static constexpr bool Evaluate()
            {
                return std::is_reference_v<T> && std::is_same_v<std::remove_reference_t<T>, U>;
            }
        };

        template <typename U>
        struct IsPointerOf : public TypeChecker
        {
            template <typename T>
            static constexpr bool Evaluate()
            {
                return std::is_pointer_v<T> && std::is_same_v<std::remove_pointer_t<T>, U>;
            }
        };
    }

    // both type can decay into the same type
    template <typename U>
    static constexpr auto similar_to = detail::SimilarTo<U>{};

    static constexpr auto is_iterator = detail::IsIterator{};

    template <typename U>
    static constexpr auto is_iterator_of = detail::IsIteratorOf<U>{};

    template <typename U>
    static constexpr auto is_enum_of = detail::IsEnumOf<U>{};

    template <typename U>
    static constexpr auto is_reference_of = detail::IsReferenceOf<U>{};

    template <typename U>
    static constexpr auto is_pointer_of = detail::IsPointerOf<U>{};
}

// TODO: have no idea should make this be in global scope or not
//       note SFINAE filters non-type-checkers anyway, it won't interfere anything
using ::eds::type::operator&&;
using ::eds::type::operator||;
using ::eds::type::operator!;