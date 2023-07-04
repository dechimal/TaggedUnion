#if !defined DESALT_DATATYPES_UTILS_HPP_INCLUDED_
#define      DESALT_DATATYPES_UTILS_HPP_INCLUDED_

#include <utility>
#include <type_traits>
#include <cstdint>
#include <initializer_list>
#include <tuple>

namespace desalt::datatypes {

namespace detail::utils {
namespace here = utils;

template<std::size_t I> struct tag;
template<typename T> struct id;

template<std::size_t, typename ...> struct at_impl;

template<typename T, typename U>
concept equality_comparable = requires (T const & t, U const & u) {
    { t == u };
};
template<typename T, typename U>
concept less_than_comparable = requires (T const & t, U const & u) {
    { t < u };
};

template<typename T> T declval();

template<typename F> decltype(auto) fix(F &&);
template<std::size_t, typename F> decltype(auto) fix_impl(F &&);

template<std::size_t, typename F> constexpr decltype(auto) with_index_sequence(F);
template<std::size_t ...Is, typename F> constexpr decltype(auto) with_index_sequence_impl(std::index_sequence<Is...>, F);

template<typename ...Fs> struct tie_t;
template<typename ...Fs> tie_t<Fs...> tie(Fs ...fs);

template<std::size_t I, typename ...Ts> using at = typename at_impl<I, Ts...>::type;

template<typename F, typename T> using apply = typename F::template apply<T>;

template<std::size_t N> using zconst = std::integral_constant<std::size_t, N>;
template<bool B>        using bconst = std::bool_constant<B>;

template<typename> struct type_fun;

// tag
template<std::size_t I>
struct tag : zconst<I> {
    using type = tag;
};

// with_index_sequence
template<std::size_t N, typename F>
constexpr decltype(auto) with_index_sequence(F f) {
    return here::with_index_sequence_impl(std::make_index_sequence<N>{}, f);
}

// with_index_sequence_impl
template<std::size_t ...Is, typename F>
constexpr decltype(auto) with_index_sequence_impl(std::index_sequence<Is...>, F f) {
    return f(zconst<Is>{}...);
}

// id
template<typename T>
struct id {
    using type = T;
};

// at
template<typename T, typename ...Ts> struct at_impl<0, T, Ts...> : id<T> {};
template<std::size_t I, typename T, typename ...Ts> struct at_impl<I, T, Ts...> : at_impl<I-1, Ts...> {};

// type_fun
template<typename> struct type_fun {};

// tie_t
// `tie_t` is a function object type to tie some function objects.
// `f` denotes value of `tie_t`, `args...` denotes function argument list,
// `f_i denotes ith element of constructor argument list of `f`.
// In call `f(args...)`, `tie_t` resolves overload by following rule:
//  - if `f_i(std::forward<Args>(args)...)` is valid expression, then it calls `f_i`,
//  - otherwise this step applies to f_i+1.
// This behavior is intent to use as pattern matching in functional programming languages.
template<typename ...Fs>
struct tie_t {
    tie_t(Fs ...fs) : fs(std::move(fs)...) {}

    static constexpr std::size_t find_first_set(std::initializer_list<bool> bs) {
        std::size_t i = 0;
        for (auto b : bs) if (b) return i; else ++i;
        throw i;
    }

    template<typename ...Args>
        requires (std::invocable<Fs, Args && ...> || ...)
    decltype(auto) operator()(Args && ...args) const {
        constexpr std::size_t index = find_first_set({std::invocable<Fs, Args && ...>...});
        return std::get<index>(fs)(std::forward<Args>(args)...);
    }
private:
    std::tuple<Fs...> fs;
};

// tie
template<typename ...Fs>
tie_t<Fs...> tie(Fs ...fs) {
    return { std::move(fs)... };
}

// fix
template<typename F>
decltype(auto) fix(F && f) {
    return here::fix_impl<0>(std::forward<F>(f));
}
template<std::size_t, typename F>
decltype(auto) fix_impl(F && f) {
    return [f{std::forward<F>(f)}] (auto && ...args) -> decltype(auto) {
        return f(here::fix_impl<sizeof...(args)>(f), std::forward<decltype(args)>(args)...);
    };
}

} // namespace detail::utils {

using detail::utils::tag;
using detail::utils::tie;
using detail::utils::fix;
using detail::utils::type_fun;

template<std::size_t N> constexpr tag<N> t{};
constexpr tag<0> _0{};
constexpr tag<1> _1{};
constexpr tag<2> _2{};
constexpr tag<3> _3{};
constexpr tag<4> _4{};
constexpr tag<5> _5{};
constexpr tag<6> _6{};
constexpr tag<7> _7{};
constexpr tag<8> _8{};
constexpr tag<9> _9{};

} // namespace desalt::datatypes {

#endif
