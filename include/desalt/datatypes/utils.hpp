#if !defined DESALT_DATATYPES_UTILS_HPP_INCLUDED_
#define      DESALT_DATATYPES_UTILS_HPP_INCLUDED_

#include <utility>
#include <type_traits>
#include <cstdint>
#include <initializer_list>
#include <tuple>

#define DESALT_DATATYPES_REQUIRE(...) typename = typename std::enable_if<(__VA_ARGS__)>::type
#define DESALT_DATATYPES_VALID_EXPR(...) typename = decltype((__VA_ARGS__), (void)0)

namespace desalt { namespace datatypes {

namespace detail {
namespace here = detail;
namespace utils {
namespace here = utils;

template<std::size_t I> struct tag;
template<typename T> struct id;

constexpr bool all();
template<typename T, typename ...Ts> constexpr bool all(T, Ts...);

template<std::size_t, typename ...> struct at_impl;

template<typename F, typename ...Ts, DESALT_DATATYPES_VALID_EXPR(std::declval<F>()(std::declval<Ts>()...))> constexpr std::true_type callable_with_test(int);
template<typename ...> constexpr std::false_type callable_with_test(...);
template<typename F, typename ...Ts> using callable_with = decltype(here::callable_with_test<F, Ts...>(0));

template<typename T, typename U, DESALT_DATATYPES_VALID_EXPR(std::declval<T>() == std::declval<U>())> std::true_type equality_comparable_test(int);
template<typename, typename> std::false_type equality_comparable_test(...);
template<typename T, typename U, DESALT_DATATYPES_VALID_EXPR(std::declval<T>() < std::declval<U>()), typename = void> std::true_type less_than_comparable_test(int);
template<typename, typename> std::false_type less_than_comparable_test(...);

template<typename T, typename U> using equality_comparable = decltype(here::equality_comparable_test<T, U>(0));
template<typename T, typename U> using less_than_comparable = decltype(here::less_than_comparable_test<T, U>(0));
template<typename F, typename ...Ts> using callable_with = decltype(here::callable_with_test<F, Ts...>(0));

template<typename T> T declval();

template<typename F> decltype(auto) fix(F &&);
template<std::size_t, typename F> decltype(auto) fix_impl(F &&);

struct make_dependency;
template<typename F, typename ...Fs, DESALT_DATATYPES_REQUIRE(callable_with<F, make_dependency>{})> constexpr decltype(auto) static_if(F, Fs ...);
template<typename F, typename ...Fs, DESALT_DATATYPES_REQUIRE(!callable_with<F, make_dependency>{}), typename = void> constexpr decltype(auto) static_if(F, Fs ...);
constexpr void static_if();

template<std::size_t, typename F> constexpr decltype(auto) with_index_sequence(F);
template<std::size_t ...Is, typename F> constexpr decltype(auto) with_index_sequence_impl(std::index_sequence<Is...>, F);

template<typename ...Fs> struct tie_t;
template<typename ...Fs> tie_t<Fs...> tie(Fs ...fs);

template<std::size_t I, typename ...Ts> using at = typename at_impl<I, Ts...>::type;

struct type_fun;

// tag
template<std::size_t I>
struct tag
    : std::integral_constant<std::size_t, I>
{
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
    return f(std::integral_constant<std::size_t, Is>{}...);
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
struct type_fun {};

// make_dependency
struct make_dependency {
    constexpr make_dependency() = default;
    template<typename T>
    constexpr T && operator()(T && x) const noexcept { return std::forward<T>(x); }
};

// static_if
template<typename F, typename ...Fs, typename>
constexpr decltype(auto) static_if(F f, Fs ...) {
    return f(make_dependency{});
}

template<typename F, typename ...Fs, typename, typename>
constexpr decltype(auto) static_if(F, Fs ...fs) {
    return here::static_if(std::move(fs)...);
}
constexpr void static_if() {}

// all
constexpr bool all() { return true; }
template<typename T, typename ...Ts>
constexpr bool all(T p, Ts ...ps) {
    return p && here::all(ps...);
}

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
        return i;
    }

    template<typename ...Args, std::size_t Index = find_first_set({callable_with<Fs, Args && ...>::value...}), DESALT_DATATYPES_REQUIRE(Index != sizeof...(Fs))>
    decltype(auto) operator()(Args && ...args) const {
        return std::get<Index>(fs)(std::forward<Args>(args)...);
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

} // namespace utils {
} // namespace detail {

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

}} // namespace desalt { namespace datatypes {

#endif
