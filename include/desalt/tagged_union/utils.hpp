#if !defined DESALT_TAGGED_UNION_UTILS_HPP_INCLUDED_
#define      DESALT_TAGGED_UNION_UTILS_HPP_INCLUDED_

#include <utility>
#include <type_traits>
#include <cstdint>
#include <initializer_list>
#include <tuple>

#define DESALT_TAGGED_UNION_REQUIRE(...) typename = typename std::enable_if<(__VA_ARGS__)>::type
#define DESALT_TAGGED_UNION_VALID_EXPR(...) typename = decltype((__VA_ARGS__), (void)0)

namespace desalt { namespace tagged_union {

namespace detail {
namespace here = detail;

template<typename T> struct id;

constexpr bool all();
template<typename T, typename ...Ts> constexpr bool all(T, Ts...);

template<std::size_t, typename ...> struct at_impl;

template<typename F, typename ...Ts, DESALT_TAGGED_UNION_VALID_EXPR(std::declval<F>()(std::declval<Ts>()...))> constexpr std::true_type callable_with_test(int);
template<typename ...> constexpr std::false_type callable_with_test(...);
template<typename F, typename ...Ts> using callable_with = decltype(here::callable_with_test<F, Ts...>(0));

template<typename F> decltype(auto) fix(F &&);
template<std::size_t, typename F> decltype(auto) fix_impl(F &&);

struct make_dependency;
template<typename F, typename ...Fs, DESALT_TAGGED_UNION_REQUIRE(callable_with<F, make_dependency>{})> constexpr decltype(auto) static_if(F, Fs ...);
template<typename F, typename ...Fs, DESALT_TAGGED_UNION_REQUIRE(!callable_with<F, make_dependency>{}), typename = void> constexpr decltype(auto) static_if(F, Fs ...);
constexpr void static_if();

template<std::size_t, typename F> constexpr decltype(auto) with_index_sequence(F);
template<std::size_t ...Is, typename F> constexpr decltype(auto) with_index_sequence_impl(std::index_sequence<Is...>, F);

template<typename ...Fs> struct tie_t;
template<typename ...Fs> tie_t<Fs...> tie(Fs ...fs);

template<std::size_t I, typename ...Ts> using at = typename at_impl<I, Ts...>::type;

struct type_fun;

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

    template<typename ...Args, std::size_t Index = find_first_set({callable_with<Fs, Args && ...>::value...}), DESALT_TAGGED_UNION_REQUIRE(Index != sizeof...(Fs))>
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

} // namespace detail {

using detail::tie;
using detail::fix;
using detail::type_fun;

}} // namespace desalt { namespace tagged_union {

#endif // DESALT_TAGGED_UNION_UTILS_HPP_INCLUDED_
