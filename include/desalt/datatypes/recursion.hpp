#if !defined DESALT_DATATYPES_RECURSION_HPP_
#define      DESALT_DATATYPES_RECURSION_HPP_

#include <desalt/datatypes/utils.hpp>

namespace desalt::datatypes {

namespace traits {

template<typename, template<typename> class> struct need_rec_guard;
template<typename, template<typename> class> struct substitute_recursion_placeholder;

}

namespace detail::rec {
namespace here = rec;

using utils::id;
using utils::apply;

template<typename> struct rec_guard;
template<typename, typename> struct unfold_impl_1;
template<typename, typename> struct unfold_impl_2;
template<std::size_t, typename> struct need_rec_guard;
template<std::size_t> struct need_rec_guard_impl;
template<std::size_t> struct rec;
template<std::size_t I> bool operator==(rec<I>, rec<I>);
template<std::size_t I> bool operator<(rec<I>, rec<I>);
template<typename> struct unwrap_impl;
template<typename> struct start_new_rec;

template<typename Self, typename T> using unfold = typename unfold_impl_1<Self, T>::type;
template<typename T> using unwrap = typename unwrap_impl<T>::type;
template<typename Self, typename T> using element = unwrap<unfold<Self, T>>;
template<typename Self, std::size_t I, typename ...Ts> using nth_unfold = unfold<Self, utils::at<I, Ts...>>;
template<typename Self, std::size_t I, typename ...Ts> using nth_element = unwrap<nth_unfold<Self, I, Ts...>>;

// subst
template<typename Self, std::size_t I>
struct subst {
    template<typename T>
    using apply = typename unfold_impl_2<subst, T>::type;
};

// unfold_impl_1
template<typename Self, typename T>
struct unfold_impl_1 {
    using result = apply<subst<Self, 0>, T>;
    static constexpr bool guard = need_rec_guard<0, T>::value;
    using type = typename std::conditional<guard, rec_guard<result>, result>::type;
};
template<typename Self, typename T>
struct unfold_impl_1<Self, rec_guard<T>>
    : id<rec_guard<apply<subst<Self, 0>, T>>>
{};

// unfold_impl_2
template<typename Subst, typename T>
struct unfold_impl_2
    : traits::substitute_recursion_placeholder<T, Subst::template apply>
{};
template<typename Self, std::size_t I> struct unfold_impl_2<subst<Self, I>, rec<I>> : id<Self> {};
template<typename Self, std::size_t I, typename T> struct unfold_impl_2<subst<Self, I>, start_new_rec<T>> : unfold_impl_2<subst<Self, I+1>, T> {};

template<typename Subst, typename T> struct unfold_impl_2<Subst, T *             > : id<apply<Subst, T> *             > {};
template<typename Subst, typename T> struct unfold_impl_2<Subst, T &             > : id<apply<Subst, T> &             > {};
template<typename Subst, typename T> struct unfold_impl_2<Subst, T &&            > : id<apply<Subst, T> &&            > {};
template<typename Subst, typename T> struct unfold_impl_2<Subst, T const         > : id<apply<Subst, T> const         > {};
template<typename Subst, typename T> struct unfold_impl_2<Subst, T       volatile> : id<apply<Subst, T>       volatile> {};
template<typename Subst, typename T> struct unfold_impl_2<Subst, T const volatile> : id<apply<Subst, T> const volatile> {};

template<typename Subst, typename T               > struct unfold_impl_2<Subst, T[ ]> : id<apply<Subst, T>[ ]> {};
template<typename Subst, typename T, std::size_t N> struct unfold_impl_2<Subst, T[N]> : id<apply<Subst, T>[N]> {};

#define CV(f, ref, e) f(, ref, e) f(const, ref, e) f(volatile, ref, e) f(const volatile, ref, e)
#define REF(f, e) CV(f, , e) CV(f, &, e) CV(f, &&, e)
#define NOEXCEPT(f) REF(f,) REF(f, noexcept)

#define DEF_FUNC_SPECIALIZATION(cv, ref, e) \
    template<typename Subst, typename R, typename ...Args> \
    struct unfold_impl_2<Subst, R(Args...) cv ref e> : id<apply<Subst, R>(apply<Subst, Args>...) cv ref e> {};

NOEXCEPT(DEF_FUNC_SPECIALIZATION)

template<typename Subst, typename T, typename C>
struct unfold_impl_2<Subst, T (C::*)> : id<apply<Subst, T>(apply<Subst, C>::*)> {};

// need_rec_guard
template<std::size_t I, typename T> struct need_rec_guard
    : traits::need_rec_guard<T, need_rec_guard_impl<I>::template apply>
{};
template<std::size_t I, typename T> struct need_rec_guard<I, start_new_rec<T>> : need_rec_guard<I + 1, T> {};
template<std::size_t I, typename TyCons> struct need_rec_guard<I, type_fun<TyCons>> : need_rec_guard<I + 1, typename TyCons::template apply<id>> {};
template<std::size_t I, typename T> struct need_rec_guard<I, T*> : std::false_type {};
template<std::size_t I, typename T, typename C> struct need_rec_guard<I, T (C::*)> : std::false_type {};
template<std::size_t I, typename T> struct need_rec_guard<I, T const> : need_rec_guard<I, T> {};
template<std::size_t I, typename T> struct need_rec_guard<I, T volatile> : need_rec_guard<I, T> {};
template<std::size_t I, typename T> struct need_rec_guard<I, T const volatile> : need_rec_guard<I, T> {};
template<std::size_t I> struct need_rec_guard<I, rec<I>> : std::true_type {};

// need_rec_guard_impl
template<std::size_t I> struct need_rec_guard_impl {
    template<typename T> using apply = typename need_rec_guard<I, T>::type;
};

// u<int, _>
// u<int, rec<u<int, _>>>

// u<std::tuple<int, _, _>, x>
// u<rec<std::tuple<int, u<std::tuple<int, _, _>, u<std::tuple<int, _, _>>>, x>

// rec
template<std::size_t>
struct rec {
    // using `rec` as a value is probably failed to be substituted.
    template<int I = 0> rec() {
        static_assert(I - I, "rec<I> is recursion placeholder. must not use as value.");
    }
};

// unwrap_impl
template<typename T> struct unwrap_impl : utils::id<T> {};
template<typename T> struct unwrap_impl<rec_guard<T>> : utils::id<T> {};

// rec_guard
template<typename T>
struct rec_guard {
    template<typename ...Args>
    explicit rec_guard(Args && ...args) : p(new T(std::forward<Args>(args)...)) {}
    rec_guard(rec_guard const & other) : rec_guard(*other.p) {}
    rec_guard(rec_guard && other) noexcept : p(other.p) {
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

template<typename T, template<typename> typename>
struct default_substitute_recursion_placeholder
    : id<T>
{};
template<template<typename...> typename Tmpl, typename ...Args, template<typename> typename Subst>
struct default_substitute_recursion_placeholder<Tmpl<Args...>, Subst>
    : id<Tmpl<Subst<Args>...>>
{};

} // namespace detail::rec {

namespace traits {

// need_rec_guard
template<typename, template<typename> class>
struct need_rec_guard : std::false_type {};

template<typename T, typename U, template<typename> class Pred>
struct need_rec_guard<std::pair<T, U>, Pred>
    : std::integral_constant<bool, Pred<T>::value || Pred<U>::value>
{};

template<typename ...Ts, template<typename> class Pred>
struct need_rec_guard<std::tuple<Ts...>, Pred>
    : std::integral_constant<bool, (Pred<Ts>::value || ...)>
{};

template<typename T, std::size_t N, template<typename> class Pred>
struct need_rec_guard<std::array<T, N>, Pred> : Pred<T> {};

// substitute_recursion_placeholder
template<typename T, template<typename> class Subst>
struct substitute_recursion_placeholder
    : detail::rec::default_substitute_recursion_placeholder<T, Subst>
{};

template<typename T, std::size_t N, template<typename> class Subst>
struct substitute_recursion_placeholder<std::array<T, N>, Subst>
    : detail::utils::id<std::array<Subst<T>, N>>
{};

template<typename TyCons, template<typename> class Subst>
struct substitute_recursion_placeholder<detail::utils::type_fun<TyCons>, Subst>
    : detail::utils::id<typename TyCons::template apply<Subst>>
{};

} // namespace traits {

using detail::rec::rec_guard;
using detail::rec::start_new_rec;
using detail::rec::rec;
using _ = rec<0>;

} // namespace desalt::datatypes {

#endif
