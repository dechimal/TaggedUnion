#if !defined DESALT_DATATYPES_RECURSION_HPP_
#define      DESALT_DATATYPES_RECURSION_HPP_
#include <desalt/datatypes/utils.hpp>

namespace desalt { namespace datatypes {

namespace traits {

template<typename, template<typename> class, typename = void> struct need_rec_guard;
template<typename, template<typename> class, typename = void> struct substitute_recursion_placeholder;

}

namespace detail {
namespace rec {

template<typename> struct rec_guard;
template<typename, typename> struct unfold_impl_1;
template<typename, std::size_t, typename> struct unfold_impl_2;
template<std::size_t, typename> struct need_rec_guard;
template<std::size_t> struct rec;
template<std::size_t I> bool operator==(rec<I>, rec<I>);
template<std::size_t I> bool operator<(rec<I>, rec<I>);

template<typename Union, typename T> using unfold = typename unfold_impl_1<Union, T>::type;
template<typename> struct start_new_rec_impl;
template<template<typename> class Subst, typename T> using start_new_rec = start_new_rec_impl<Subst<T>>;

// unfold_impl_1
template<typename Self, typename T>
struct unfold_impl_1 {
    using result = typename unfold_impl_2<Self, 0, T>::type;
    static constexpr bool guard = need_rec_guard<0, T>::value;
    using type = typename std::conditional<guard, rec_guard<result>, result>::type;
};
template<typename Self, typename T>
struct unfold_impl_1<Self, rec_guard<T>>
    : utils::id<rec_guard<typename unfold_impl_2<Self, 0, T>::type>>
{};

// unfold_impl_2
template<typename Union, std::size_t I, typename T>
struct unfold_impl_2 {
    template<typename U> using apply = unfold_impl_2<Union, I, U>;
    using type = typename traits::substitute_recursion_placeholder<T, apply>::type;
};
template<typename Union, std::size_t I> struct unfold_impl_2<Union, I, rec<I>> : utils::id<Union> {};
template<typename Union, std::size_t I, typename T> struct unfold_impl_2<Union, I, T*> : utils::id<typename unfold_impl_2<Union, I, T>::type*> {};
template<typename Union, std::size_t I, typename T> struct unfold_impl_2<Union, I, T&> : utils::id<typename unfold_impl_2<Union, I, T>::type&> {};
template<typename Union, std::size_t I, typename T> struct unfold_impl_2<Union, I, T&&> : utils::id<typename unfold_impl_2<Union, I, T>::type&&> {};
template<typename Union, std::size_t I, typename T> struct unfold_impl_2<Union, I, T const> : utils::id<typename unfold_impl_2<Union, I, T>::type const> {};
template<typename Union, std::size_t I, typename T> struct unfold_impl_2<Union, I, T volatile> : utils::id<typename unfold_impl_2<Union, I, T>::type volatile> {};
template<typename Union, std::size_t I, typename T> struct unfold_impl_2<Union, I, T const volatile> : utils::id<typename unfold_impl_2<Union, I, T>::type const volatile> {};
template<typename Union, std::size_t I, typename T, std::size_t N> struct unfold_impl_2<Union, I, T[N]> : utils::id<typename unfold_impl_2<Union, I, T>::type[N]> {};
template<typename Union, std::size_t I, typename R, typename ...Args> struct unfold_impl_2<Union, I, R(Args...)> : utils::id<typename unfold_impl_2<Union, I, R>::type(typename unfold_impl_2<Union, I, Args>::type...)> {};
template<typename Union, std::size_t I, typename R, typename ...Args> struct unfold_impl_2<Union, I, R(Args...) const> : utils::id<typename unfold_impl_2<Union, I, R>::type(typename unfold_impl_2<Union, I, Args>::type...) const> {};
template<typename Union, std::size_t I, typename R, typename ...Args> struct unfold_impl_2<Union, I, R(Args...) volatile> : utils::id<typename unfold_impl_2<Union, I, R>::type(typename unfold_impl_2<Union, I, Args>::type...) volatile> {};
template<typename Union, std::size_t I, typename R, typename ...Args> struct unfold_impl_2<Union, I, R(Args...) const volatile> : utils::id<typename unfold_impl_2<Union, I, R>::type(typename unfold_impl_2<Union, I, Args>::type...) const volatile> {};
template<typename Union, std::size_t I, typename R, typename ...Args> struct unfold_impl_2<Union, I, R(Args...) &> : utils::id<typename unfold_impl_2<Union, I, R>::type(typename unfold_impl_2<Union, I, Args>::type...) &> {};
template<typename Union, std::size_t I, typename R, typename ...Args> struct unfold_impl_2<Union, I, R(Args...) const &> : utils::id<typename unfold_impl_2<Union, I, R>::type(typename unfold_impl_2<Union, I, Args>::type...) const &> {};
template<typename Union, std::size_t I, typename R, typename ...Args> struct unfold_impl_2<Union, I, R(Args...) volatile &> : utils::id<typename unfold_impl_2<Union, I, R>::type(typename unfold_impl_2<Union, I, Args>::type...) volatile &> {};
template<typename Union, std::size_t I, typename R, typename ...Args> struct unfold_impl_2<Union, I, R(Args...) const volatile &> : utils::id<typename unfold_impl_2<Union, I, R>::type(typename unfold_impl_2<Union, I, Args>::type...) const volatile &> {};
template<typename Union, std::size_t I, typename R, typename ...Args> struct unfold_impl_2<Union, I, R(Args...) &&> : utils::id<typename unfold_impl_2<Union, I, R>::type(typename unfold_impl_2<Union, I, Args>::type...) &&> {};
template<typename Union, std::size_t I, typename R, typename ...Args> struct unfold_impl_2<Union, I, R(Args...) const &&> : utils::id<typename unfold_impl_2<Union, I, R>::type(typename unfold_impl_2<Union, I, Args>::type...) const &&> {};
template<typename Union, std::size_t I, typename R, typename ...Args> struct unfold_impl_2<Union, I, R(Args...) volatile &&> : utils::id<typename unfold_impl_2<Union, I, R>::type(typename unfold_impl_2<Union, I, Args>::type...) volatile &&> {};
template<typename Union, std::size_t I, typename R, typename ...Args> struct unfold_impl_2<Union, I, R(Args...) const volatile &&> : utils::id<typename unfold_impl_2<Union, I, R>::type(typename unfold_impl_2<Union, I, Args>::type...) const volatile &&> {};
template<typename Union, std::size_t I, typename T, typename C> struct unfold_impl_2<Union, I, T (C::*)> : utils::id<typename unfold_impl_2<Union, I, T>::type(unfold_impl_2<Union, I, C>::type::*)> {};

// start_new_rec_impl
template<typename Union, std::size_t I, typename T>
struct start_new_rec_impl<unfold_impl_2<Union, I, T>> : unfold_impl_2<Union, I + 1, T> {};

template<std::size_t I, typename T>
struct start_new_rec_impl<need_rec_guard<I, T>> : need_rec_guard<I + 1, T> {};

// need_rec_guard
template<std::size_t I, typename T> struct need_rec_guard {
    template<typename U> using apply = need_rec_guard<I, U>;
    using type = typename traits::need_rec_guard<T, apply>::type;
    using value_type = bool;
    static constexpr bool value = type::value;
    constexpr operator bool() const noexcept { return value; }
    constexpr bool operator()() const noexcept { return value; }
};
template<std::size_t I, typename T> struct need_rec_guard<I, T*> : std::false_type {};
template<std::size_t I, typename T, typename C> struct need_rec_guard<I, T (C::*)> : std::false_type {};
template<std::size_t I, typename T> struct need_rec_guard<I, T const> : need_rec_guard<I, T> {};
template<std::size_t I, typename T> struct need_rec_guard<I, T volatile> : need_rec_guard<I, T> {};
template<std::size_t I, typename T> struct need_rec_guard<I, T const volatile> : need_rec_guard<I, T> {};
template<std::size_t I> struct need_rec_guard<I, rec<I>> : std::true_type {};

// u<int, _>
// u<int, rec<u<int, _>>>

// u<std::tuple<int, _, _>, x>
// u<rec<std::tuple<int, u<std::tuple<int, _, _>, u<std::tuple<int, _, _>>>, x>

// rec
template<std::size_t>
struct rec {
    // using `rec` as a value is probably failed to be substituted.
    template<int I = 0> rec() { static_assert(I - I, "rec<I> is recursion placeholder. must not use as value."); }
};

// rec_guard
template<typename T>
struct rec_guard {
    template<typename ...Args>
    explicit rec_guard(Args && ...args) : p(new T(std::forward<Args>(args)...)) {}
    explicit rec_guard(rec_guard const & other) : rec_guard(*other.p) {}
    explicit rec_guard(rec_guard && other) noexcept : p(other.p) {
        other.p = nullptr;
    }
    ~rec_guard() { delete p; }

    rec_guard & operator=(rec_guard const & other) & {
        *p = *other.p;
        return *this;
    }
    rec_guard & operator=(rec_guard && other) & {
        *p = std::move(*other.p);
        return *this;
    }
    operator T        & ()        & { return *p; }
    operator T const  & () const  & { return *p; }
    operator T       && ()       && { return std::move(*p); }
    operator T const && () const && { return std::move(*p); }
private:
    T * p;
};

} // namespace rec {
} // namespace detail {

namespace traits {

// need_rec_guard
template<typename, template<typename> class, typename>
struct need_rec_guard : std::false_type {};

template<typename T, typename U, template<typename> class Pred>
struct need_rec_guard<std::pair<T, U>, Pred>
    : std::integral_constant<bool, Pred<T>::value || Pred<U>::value>
{};

template<typename ...Ts, template<typename> class Pred>
struct need_rec_guard<std::tuple<Ts...>, Pred>
    : std::integral_constant<bool, !detail::utils::all(!Pred<Ts>::value...)>
{};

template<typename T, std::size_t N, template<typename> class Pred>
struct need_rec_guard<std::array<T, N>, Pred> : Pred<T> {};

// substitute_recursion_placeholder
template<typename T, template<typename> class, typename>
struct substitute_recursion_placeholder : detail::utils::id<T> {};

template<typename T, std::size_t N, template<typename> class Subst>
struct substitute_recursion_placeholder<std::array<T, N>, Subst>
    : detail::utils::id<std::array<typename Subst<T>::type, N>>
{};

template<template<typename ...> class Tmpl, typename ...Args, template<typename> class Subst>
struct substitute_recursion_placeholder<Tmpl<Args...>, Subst>
    : detail::utils::id<Tmpl<typename Subst<Args>::type...>>
{};

template<template<typename ...> class Tmpl, typename ...Args, template<typename> class Subst>
struct substitute_recursion_placeholder<Tmpl<type_fun, Args...>, Subst>
    : detail::utils::id<typename Tmpl<type_fun, typename Subst<Args>::type...>::type>
{};

} // namespace traits {

using detail::rec::rec_guard;
using detail::rec::start_new_rec;
using detail::rec::rec;
using _ = rec<0>;

}} // namespace desalt { namespace datatypes {

#endif
