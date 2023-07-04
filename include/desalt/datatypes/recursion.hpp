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
template<typename, std::size_t> struct unfold_impl;
template<typename, typename> struct unfold_guarded_impl;
template<std::size_t> struct need_rec_guard;
template<std::size_t> struct rec;
template<std::size_t I> bool operator==(rec<I>, rec<I>);
template<std::size_t I> bool operator<(rec<I>, rec<I>);
template<typename> struct unwrap_impl;
template<typename> struct start_new_rec;
template<typename T, template<typename> typename> struct default_subst;

template<typename Self, typename T> using unfold = typename unfold_impl<Self, 0>::template apply<T>;
template<typename Self, typename T> using storage = typename unfold_guarded_impl<Self, T>::type;
template<typename T> using unwrap = typename unwrap_impl<T>::type;
template<typename Self, typename T> using element = unfold<Self, unwrap<T>>;
template<typename Self, std::size_t I, typename ...Ts> using nth_element = element<Self, unwrap<utils::at<I, Ts...>>>;
template<typename Self, std::size_t I, typename ...Ts> using nth_storage = storage<Self, utils::at<I, Ts...>>;

#define DEF_VAR_VARIANTS(f) f(*) f(&) f(&&) f(const) f(volatile) f(const volatile)
#define DEF_VAR_SPECIALIZATION(qual) \
    template<typename T> \
    struct apply_impl<T qual> : id<apply<T> qual> {};

#define DEF_FUNC_VARIANTS_CV(f, ref, e) f(, ref, e) f(const, ref, e) f(volatile, ref, e) f(const volatile, ref, e)
#define DEF_FUNC_VARIANTS_REF(f, e) DEF_FUNC_VARIANTS_CV(f, , e) DEF_FUNC_VARIANTS_CV(f, &, e) DEF_FUNC_VARIANTS_CV(f, &&, e)
#define DEF_FUNC_VARIANTS(f) DEF_FUNC_VARIANTS_REF(f,) DEF_FUNC_VARIANTS_REF(f, noexcept)

#define DEF_FUNC_SPECIALIZATION(cv, ref, e) \
    template<typename R, typename ...Args> \
    struct apply_impl<R(Args...) cv ref e> : id<apply<R>(apply<Args>...) cv ref e> {};

// unfold_impl
template<typename Self, std::size_t I>
struct unfold_impl {
    template<typename T> struct apply_impl;
    template<typename T> using apply = typename apply_impl<T>::type;
    template<typename T> struct apply_impl
        : traits::substitute_recursion_placeholder<T, apply>
    {};

    template<typename T> struct apply_impl<start_new_rec<T>>
        : unfold_impl<Self, I + 1>::template apply_impl<T>
    {};
    template<std::size_t J> requires (J == I) struct apply_impl<rec<J>    > : id<Self> {};
    template<typename T               > struct apply_impl<T [ ]           > : id<apply<T>[ ]> {};
    template<typename T, std::size_t N> struct apply_impl<T [N]           > : id<apply<T>[N]> {};
    template<typename T, typename C   > struct apply_impl<T (C::*)        > : id<apply<T>(apply<C>::*)> {};

    DEF_VAR_VARIANTS(DEF_VAR_SPECIALIZATION)
    DEF_FUNC_VARIANTS(DEF_FUNC_SPECIALIZATION)
};

// need_rec_guard
template<std::size_t I> struct need_rec_guard {
    template<typename T> struct apply_impl;
    template<typename T> using apply = typename apply_impl<T>::type;
    template<typename T> struct apply_impl : traits::need_rec_guard<T, apply> {};

    template<typename T> struct apply_impl<start_new_rec<T>>
        : need_rec_guard<I + 1>::template apply_impl<T>
    {};
    template<typename TyCons> struct apply_impl<type_fun<TyCons>>
        : apply_impl<typename TyCons::template apply<id>>
    {};
    template<std::size_t J> requires (J == I) struct apply_impl<rec<J>    > : std::true_type {};
    template<typename T               > struct apply_impl<T *             > : std::false_type {};
    template<typename T               > struct apply_impl<T const         > : apply_impl<T> {};
    template<typename T               > struct apply_impl<T       volatile> : apply_impl<T> {};
    template<typename T               > struct apply_impl<T const volatile> : apply_impl<T> {};
    template<typename T, std::size_t N> struct apply_impl<T [N]           > : apply_impl<T> {};
    template<typename T, typename C   > struct apply_impl<T (C::*)        > : std::false_type {};
};

// unfold_guarded_impl
template<typename Self, typename T>
struct unfold_guarded_impl
    : std::conditional<
        need_rec_guard<0>::template apply<T>::value,
        rec_guard<unfold<Self, T>>,
        unfold<Self, T>
    >
{};
template<typename Self, typename T>
struct unfold_guarded_impl<Self, rec_guard<T>>
    : id<rec_guard<unfold<Self, T>>>
{};

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
struct default_subst
    : id<T>
{};
template<template<typename...> typename Tmpl, typename ...Args, template<typename> typename Subst>
struct default_subst<Tmpl<Args...>, Subst>
    : id<Tmpl<Subst<Args>...>>
{};

} // namespace detail::rec {

namespace traits {

// need_rec_guard
template<typename, template<typename> class>
struct need_rec_guard : std::false_type {};

template<typename T, typename U, template<typename> class Pred>
struct need_rec_guard<std::pair<T, U>, Pred>
    : detail::utils::bconst<Pred<T>::value || Pred<U>::value>
{};

template<typename ...Ts, template<typename> class Pred>
struct need_rec_guard<std::tuple<Ts...>, Pred>
    : detail::utils::bconst<(Pred<Ts>::value || ...)>
{};

template<typename T, std::size_t N, template<typename> class Pred>
struct need_rec_guard<std::array<T, N>, Pred> : Pred<T> {};

// substitute_recursion_placeholder
template<typename T, template<typename> class Subst>
struct substitute_recursion_placeholder
    : detail::rec::default_subst<T, Subst>
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
