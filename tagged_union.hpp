#if !defined DESALT_TAGGED_UNION_HPP_INCLUDED_
#define      DESALT_TAGGED_UNION_HPP_INCLUDED_

#include <type_traits>
#include <utility>
#include <stdexcept>
#include <tuple>

#if defined __GNUC__
    #pragma GCC diagnostic push
    #if !defined __clang__
        #pragma GCC diagnostic ignored "-Wunused-but-set-parameter"
    #else
        #pragma GCC diagnostic ignored "-Wmismatched-tags"
        #pragma GCC diagnostic ignored "-Wundefined-internal"
        #pragma GCC diagnostic ignored "-Wundefined-inline"
    #endif
#endif

#define DESALT_TAGGED_UNION_REQUIRE(...) typename = typename std::enable_if<(__VA_ARGS__)>::type
#define DESALT_TAGGED_UNION_VALID_EXPR(...) typename = decltype((__VA_ARGS__), (void)0)

namespace desalt { namespace tagged_union {

// forward declarations
namespace traits {

template<typename, template<typename> class, typename = void> struct need_recursion_protection;
template<typename, template<typename> class, typename = void> struct substitute_recursion_placeholder;

} // namespace traits {

namespace detail {

namespace here = detail;

template<std::size_t I> struct tag_t;
template<typename T> struct id;
template<typename ...> struct tagged_union;
template<typename, typename> struct visitor_table;
template<typename T> inline void destroy(T &);
template<std::size_t, typename ...> struct find_fallback_type;
constexpr bool all();
template<typename T, typename ...Ts> constexpr bool all(T, Ts...);
template<typename T, typename U, DESALT_TAGGED_UNION_VALID_EXPR(std::declval<T>() == std::declval<U>())> std::true_type equality_comparable_test(int);
template<typename, typename> std::false_type equality_comparable_test(...);
template<typename T, typename U, DESALT_TAGGED_UNION_VALID_EXPR(std::declval<T>() < std::declval<U>()), typename = void> std::true_type less_than_comparable_test(int);
template<typename, typename> std::false_type less_than_comparable_test(...);
template<std::size_t, typename ...> struct at_impl;
template<typename> struct recursive;
template<typename> struct unwrap_impl;
template<typename ...Fs> struct tie_t;
template<typename ...Fs> tie_t<Fs...> tie(Fs ...fs);
template<typename F, typename ...Ts, DESALT_TAGGED_UNION_VALID_EXPR(std::declval<F>()(std::declval<Ts>()...))> constexpr std::true_type callable_with_test(int);
template<typename ...> constexpr std::false_type callable_with_test(...);
struct unexpected_case;
struct bad_tag;
template<typename, typename> struct unfold_impl_1;
template<typename, std::size_t, typename> struct unfold_impl_2;
template<std::size_t, typename> struct need_recursion_protection;
template<std::size_t> struct _r;
template<std::size_t I> bool operator==(_r<I>, _r<I>);
template<std::size_t I> bool operator<(_r<I>, _r<I>);
template<typename ...Ts, typename ...Us> bool operator==(tagged_union<Ts...> const &, tagged_union<Us...> const &);
template<typename ...Ts, typename ...Us> bool operator!=(tagged_union<Ts...> const &, tagged_union<Us...> const &);
template<typename ...Ts, typename ...Us> bool operator<(tagged_union<Ts...> const &, tagged_union<Us...> const &);
template<typename ...Ts, typename ...Us> bool operator>(tagged_union<Ts...> const &, tagged_union<Us...> const &);
template<typename ...Ts, typename ...Us> bool operator<=(tagged_union<Ts...> const &, tagged_union<Us...> const &);
template<typename ...Ts, typename ...Us> bool operator>=(tagged_union<Ts...> const &, tagged_union<Us...> const &);
template<typename F> auto fix(F);
template<typename F, std::size_t> auto fix_impl(F);
struct type_fun;
struct make_dependency;
template<typename F, typename ...Ts> using callable_with = decltype(here::callable_with_test<F, Ts...>(0));
template<typename F, typename ...Fs, DESALT_TAGGED_UNION_REQUIRE(callable_with<F, make_dependency>{})> constexpr decltype(auto) static_if(F, Fs ...);
template<typename F, typename ...Fs, DESALT_TAGGED_UNION_REQUIRE(!callable_with<F, make_dependency>{}), typename = void> constexpr decltype(auto) static_if(F, Fs ...);
constexpr void static_if();

template<typename T> using unwrap = typename unwrap_impl<T>::type;
template<std::size_t I, typename ...Ts> using at = typename at_impl<I, Ts...>::type;
template<typename T, typename U> using equality_comparable = decltype(here::equality_comparable_test<T, U>(0));
template<typename T, typename U> using less_than_comparable = decltype(here::less_than_comparable_test<T, U>(0));
using _ = _r<0>;

// implementations

// tagged_union
template<typename ...Ts>
class tagged_union {
    template<typename T> using unfold = typename here::unfold_impl_1<tagged_union, T>::type;
    using fallback_tag = typename find_fallback_type<0, unfold<Ts>...>::type;
    static constexpr std::size_t elements_size = sizeof...(Ts);
    static constexpr bool enable_fallback = fallback_tag::value != elements_size;
    static constexpr std::size_t backup_mask = ~(~(std::size_t)0 >> 1);
    template<std::size_t I> using actual_element = at<I, unfold<Ts>...>;
    template<std::size_t I> using element = unwrap<actual_element<I>>;

    static_assert(((elements_size + enable_fallback) & backup_mask) == 0, "too many elements.");

public:
    template<std::size_t I>
    tagged_union(tag_t<I> t, element<I> const & x) : which_(t.value) {
        this->construct_directly(t, x);
    }
    template<std::size_t I>
    tagged_union(tag_t<I> t, element<I> && x) : which_(t.value) {
        this->construct_directly(t, std::move(x));
    }
    template<std::size_t I, typename ...Args,
             DESALT_TAGGED_UNION_REQUIRE(std::is_constructible<element<I>, Args &&...>::value)>
    tagged_union(tag_t<I> t, Args && ...args) : which_(t.value) {
        this->construct_directly(t, std::forward<Args>(args)...);
    }
    tagged_union(tagged_union const & other) : which_(other.which()) {
        this->construct(other);
    }
    tagged_union(tagged_union && other) : which_(other.which()) {
        this->construct(std::move(other));
    }
    template<typename ...Us, DESALT_TAGGED_UNION_REQUIRE(here::all(std::is_constructible<Ts, Us>::value...))>
    tagged_union(tagged_union<Us...> const & other) : which_(other.which()) {
        this->construct(other);
    }
    template<typename ...Us, DESALT_TAGGED_UNION_REQUIRE(here::all(std::is_constructible<Ts, Us>::value...))>
    tagged_union(tagged_union<Us...> && other) : which_(other.which()) {
        this->construct(std::move(other));
    }
    ~tagged_union() {
        destroy();
    }

    tagged_union & operator=(tagged_union const & other) & {
        this->assign(other);
        return *this;
    }
    tagged_union & operator=(tagged_union && other) & {
        this->assign(std::move(other));
        return *this;
    }
    template<typename ...Us>
    typename std::enable_if<here::all(std::is_assignable<Ts, Us>::value...), tagged_union &>::type operator=(tagged_union<Us...> const & other) & {
        this->assign(other);
        return *this;
    }
    template<typename ...Us>
    typename std::enable_if<here::all(std::is_assignable<Ts, Us>::value...), tagged_union &>::type operator=(tagged_union<Us...> && other) & {
        this->assign(std::move(other));
        return *this;
    }

    template<std::size_t I> element<I>        & get(tag_t<I> t)        & { return get_impl(t); }
    template<std::size_t I> element<I> const  & get(tag_t<I> t) const  & { return get_impl(t); }
    template<std::size_t I> element<I>       && get(tag_t<I> t)       && { return std::move(get_impl(t)); }
    template<std::size_t I> element<I> const && get(tag_t<I> t) const && { return std::move(get_impl(t)); }
    template<std::size_t I> element<I>        & get_unchecked(tag_t<I> t)        & { return get_unchecked_impl(t); }
    template<std::size_t I> element<I> const  & get_unchecked(tag_t<I> t) const  & { return get_unchecked_impl(t); }
    template<std::size_t I> element<I>       && get_unchecked(tag_t<I> t)       && { return std::move(get_unchecked_impl(t)); }
    template<std::size_t I> element<I> const && get_unchecked(tag_t<I> t) const && { return std::move(get_unchecked_impl(t)); }

    std::size_t which() const {
        return which_ & ~backup_mask;
    }

    template<typename F>
    auto dispatch(F f) const -> decltype(auto) {
        // following comment out code is broken still with clang 3.6.2
        // using iseq = std::index_sequence_for<Ts...>;
        // return here::visitor_table<F, iseq>::table[which()](f);
        return here::visitor_table<F, std::index_sequence_for<Ts...>>::table[which()](f);
    }

    template<typename F>
    auto when(F f) const -> decltype(auto) {
        return this->dispatch([&] (auto t) {
                return f(t, this->get_unchecked(t));
            });
    }

private:
    template<std::size_t I, typename E = bad_tag>
    actual_element<I> & get_impl(tag_t<I> t) {
        if (t.value == which()) return get_unchecked_impl(t);
        else throw E();
    }
    template<std::size_t I, typename E = bad_tag>
    actual_element<I> const & get_impl(tag_t<I> t) const {
        if (t.value == which()) return get_unchecked_impl(t);
        else throw E();
    }
    template<std::size_t I, bool cond = enable_fallback, DESALT_TAGGED_UNION_REQUIRE(cond)>
    actual_element<I> & get_unchecked_impl(tag_t<I> t) {
        static_assert(t.value < elements_size, "the tag is too large.");
        return get_typed(t);
    }
    template<std::size_t I, bool cond = enable_fallback, DESALT_TAGGED_UNION_REQUIRE(!cond), typename = void>
    actual_element<I> & get_unchecked_impl(tag_t<I> t) {
        static_assert(t.value < elements_size, "the tag is too large.");
        if (!this->backedup()) return get_typed(t);
        else return get_backup(t);
    }
    template<std::size_t I, bool cond = enable_fallback, DESALT_TAGGED_UNION_REQUIRE(cond)>
    actual_element<I> const & get_unchecked_impl(tag_t<I> t) const {
        static_assert(t.value < elements_size, "the tag is too large.");
        return get_typed(t);
    }
    template<std::size_t I, bool cond = enable_fallback, DESALT_TAGGED_UNION_REQUIRE(!cond), typename = void>
    actual_element<I> const & get_unchecked_impl(tag_t<I> t) const {
        static_assert(t.value < elements_size, "the tag is too large.");
        if (!backedup()) return get_typed(t);
        else return get_backup(t);
    }
    template<typename Tag, typename T = actual_element<Tag::value>>
    T & get_typed(Tag) {
        return *reinterpret_cast<T*>(&storage_);
    }
    template<typename Tag, typename T = actual_element<Tag::value>>
    T const & get_typed(Tag) const {
        return *reinterpret_cast<T const *>(&storage_);
    }
    template<typename Tag, typename T = actual_element<Tag::value>, bool cond = enable_fallback, DESALT_TAGGED_UNION_REQUIRE(!cond)>
    T & get_backup(Tag) {
        return **reinterpret_cast<T**>(&storage_);
    }
    template<typename Tag, typename T = actual_element<Tag::value>, bool cond = enable_fallback, DESALT_TAGGED_UNION_REQUIRE(!cond)>
    T const & get_backup(Tag) const {
        return **reinterpret_cast<T * const *>(&storage_);
    }

    bool nothrow_copy_constructible() const {
        return this->dispatch([] (auto t) {
                return std::is_nothrow_copy_constructible<actual_element<t.value>>::value;
            });
    }
    bool nothrow_move_constructible() const {
        return this->dispatch([] (auto t) {
                return std::is_nothrow_move_constructible<actual_element<t.value>>::value;
            });
    }
    template<typename Union>
    bool nothrow_constructible(Union && other) const {
        using value_type = typename std::decay<Union>::type;
        return other.dispatch([] (auto t) {
                return std::is_nothrow_constructible<actual_element<t.value>, typename value_type::template actual_element<t.value>&&>::value;
            });
    }
    template<typename Union>
    void construct(Union && other) {
        other.dispatch([&] (auto t) {
                this->construct_directly(t, std::forward<Union>(other).get_unchecked(t));
            });
    }
    template<std::size_t I, typename ...Args>
    void construct_directly(tag_t<I> t, Args && ...args) {
        new(&storage_) actual_element<t.value>(std::forward<Args>(args)...);
    }
    void destroy() {
        this->dispatch([&] (auto t) {
                this->destroy(t);
            });
    }
    template<typename Tag, bool cond = enable_fallback, DESALT_TAGGED_UNION_REQUIRE(cond)>
    void destroy(Tag t) {
        here::destroy(get_typed(t));
    }
    template<typename Tag, bool cond = enable_fallback, DESALT_TAGGED_UNION_REQUIRE(!cond), typename = void>
    void destroy(Tag t) {
        if (backedup()) delete &get_backup(t);
        else here::destroy(get_typed(t));
    }

    template<typename Union>
    void assign(Union && other) {
        if (which_ == other.which_) {
            this->copy_or_move_assign(std::forward<Union>(other));
        } else {
            if (this->nothrow_constructible(std::forward<Union>(other))) {
                this->construct_using_nothrow_constructor(std::forward<Union>(other));
            } else {
                auto fallback = here::static_if([&] (auto dep, std::enable_if_t<decltype(dep)()(std::is_lvalue_reference<Union&&>::value)> * = {}) {
                    bool nothrow_move_constructible = other.nothrow_move_constructible();
                    if (nothrow_move_constructible) {
                        this->construct_using_right_hand_move_constructor(other);
                    }
                    return !nothrow_move_constructible;
                }, [] (auto) {
                    return std::true_type{};
                });
                if (fallback) {
                    if (this->nothrow_move_constructible()) {
                        this->construct_using_auto_storage_save(std::forward<Union>(other));
                    } else {
                        here::static_if([&] (auto dep, std::enable_if_t<decltype(dep)()(enable_fallback)> * = {}) {
                            this->construct_using_fallback_type(std::forward<Union>(other));
                        }, [&] (auto) {
                            this->construct_using_dynamic_storage_save(std::forward<Union>(other));
                        });
                    }
                }
            }
            this->set_which(other.which());
        }
    }
    template<typename Union>
    void copy_or_move_assign(Union && other) {
        this->dispatch([&] (auto t) {
                this->get_unchecked(t) = std::forward<Union>(other).get_unchecked(t);
            });
    }
    template<typename Union>
    void construct_using_nothrow_constructor(Union && other) {
        this->destroy();
        this->construct(std::forward<Union>(other));
    }
    template<typename Union>
    void construct_using_right_hand_move_constructor(Union const & other) {
        other.dispatch([&] (auto t) {
                using other_actual_element = typename std::decay_t<Union>::template actual_element<t.value>;
                other_actual_element tmp(other.get_unchecked(t));
                this->destroy();
                this->construct_directly(t, std::move(tmp));
            });
    }
    template<typename Union>
    void construct_using_auto_storage_save(Union && other) {
        this->dispatch([&] (auto t) {
                actual_element<t.value> tmp(std::move(this->get_unchecked(t)));
                this->destroy(t);
                try {
                    this->construct(std::forward<Union>(other));
                } catch (...) {
                    this->construct_directly(t, std::move(tmp));
                    throw;
                }
            });
    }
    template<typename Union, typename FallbackTag = fallback_tag>
    void construct_using_fallback_type(Union && other) {
        this->dispatch([&] (auto t) {
            try {
                this->destroy(t);
                this->construct(std::forward<Union>(other));
            } catch (...) {
                this->construct_directly(FallbackTag{});
                this->set_which(FallbackTag::value);
                throw;
            }
        });
    }
    template<typename Union>
    void construct_using_dynamic_storage_save(Union && other) {
        this->dispatch([&] (auto t) {
            auto p = new actual_element<t.value>(std::move_if_noexcept(this->get_unchecked(t)));
            try {
                this->destroy(t);
                this->construct(std::forward<Union>(other));
                delete p;
            } catch (...) {
                *reinterpret_cast<void **>(&this->storage_) = p;
                this->mark_as_backup();
                throw;
            }
        });
    }

    template<typename ...Ts1, typename ...Ts2> friend bool operator==(tagged_union<Ts1...> const &, tagged_union<Ts2...> const &);
    template<typename ...Ts1, typename ...Ts2> friend bool operator!=(tagged_union<Ts1...> const &, tagged_union<Ts2...> const &);
    template<typename ...Ts1, typename ...Ts2> friend bool operator<(tagged_union<Ts1...> const &, tagged_union<Ts2...> const &);
    template<typename ...Ts1, typename ...Ts2> friend bool operator>(tagged_union<Ts1...> const &, tagged_union<Ts2...> const &);
    template<typename ...Ts1, typename ...Ts2> friend bool operator<=(tagged_union<Ts1...> const &, tagged_union<Ts2...> const &);
    template<typename ...Ts1, typename ...Ts2> friend bool operator>=(tagged_union<Ts1...> const &, tagged_union<Ts2...> const &);

    void set_which(std::size_t which) {
        which_ = which;
    }
    template<bool cond = enable_fallback, DESALT_TAGGED_UNION_REQUIRE(!cond)>
    bool backedup() const {
        return (which_ & backup_mask) != 0;
    }
    template<bool cond = enable_fallback, DESALT_TAGGED_UNION_REQUIRE(!cond)>
    void mark_as_backup() {
        which_ |= backup_mask;
    }

    std::size_t which_;
    typename std::conditional<enable_fallback,
        std::aligned_union_t<0, unfold<Ts>...>,
        std::aligned_union_t<0, unfold<Ts>..., void*>>::type storage_;
};

template<typename ...Ts, typename ...Us>
bool operator==(tagged_union<Ts...> const & a, tagged_union<Us...> const & b) {
    static_assert(here::all(equality_comparable<unwrap<Ts>, unwrap<Us>>::value...), "each element type must be equality comparable.");
    if (a.which() != b.which()) return false;
    return a.dispatch([&] (auto t) -> bool {
            return a.get_unchecked(t) == b.get_unchecked(t);
        });
}
template<typename ...Ts, typename ...Us>
bool operator!=(tagged_union<Ts...> const & a, tagged_union<Us...> const & b) {
    return !(a == b);
}
template<typename ...Ts, typename ...Us>
bool operator<(tagged_union<Ts...> const & a, tagged_union<Us...> const & b) {
    static_assert(here::all(less_than_comparable<unwrap<Ts>, unwrap<Us>>::value...), "each element type must be less than comparable.");
    if (a.which() != b.which()) return a.which() < b.which();
    return a.dispatch([&] (auto t) -> bool {
            return a.get_unchecked(t) < b.get_unchecked(t);
        });
}
template<typename ...Ts, typename ...Us>
bool operator>(tagged_union<Ts...> const & a, tagged_union<Us...> const & b) {
    return b < a;
}
template<typename ...Ts, typename ...Us>
bool operator<=(tagged_union<Ts...> const & a, tagged_union<Us...> const & b) {
    return !(b < a);
}
template<typename ...Ts, typename ...Us>
bool operator>=(tagged_union<Ts...> const & a, tagged_union<Us...> const & b) {
    return !(a < b);
}

// visitor_table
template<typename F, std::size_t ...Is>
struct visitor_table<F, std::index_sequence<Is...>> {
    using result_type = typename std::common_type<decltype(std::declval<F &>()(tag_t<Is>{}))...>::type;
    using visitor_type = result_type(*)(F &);
    template<typename std::size_t I>
    static result_type visit(F & f) {
        return f(tag_t<I>{});
    }
    static constexpr visitor_type table[sizeof...(Is)]{ &visit<Is>... };
};
template<typename F, std::size_t ...Is>
constexpr typename visitor_table<F, std::index_sequence<Is...>>::visitor_type visitor_table<F, std::index_sequence<Is...>>::table[sizeof...(Is)];

// destroy
template<typename T>
void destroy(T & x) {
    x.~T();
}

// find_fallback_type
template<std::size_t I>
struct find_fallback_type<I> : tag_t<I> {};
template<std::size_t I, typename T, typename ...Ts>
struct find_fallback_type<I, T, Ts...>
    : std::conditional<std::is_nothrow_default_constructible<T>::value,
                       tag_t<I>,
                       find_fallback_type<I+1, Ts...>>::type
{};
// Specialization for recursive types. Escape default constructibility inspection of recursive<T>.
// Because `std::is_nothrow_default_constructible` may inspect default construtibility of `U` in instaciation of `U`,
// where `U` is a `tagged_union<...>` contains `_`.
// `recursive` allocates memory in any constructor. So this check may escape.
template<std::size_t I, typename T, typename ...Ts>
struct find_fallback_type<I, recursive<T>, Ts...>
    : find_fallback_type<I+1, Ts...>
{};

// all
constexpr bool all() { return true; }
template<typename T, typename ...Ts>
constexpr bool all(T p, Ts ...ps) {
    return p && here::all(ps...);
}

// tag_t
template<std::size_t I>
struct tag_t
    : std::integral_constant<std::size_t, I>
{
    using type = tag_t<I>;
};

// id
template<typename T>
struct id {
    using type = T;
};

// at
template<typename T, typename ...Ts> struct at_impl<0, T, Ts...> : id<T> {};
template<std::size_t I, typename T, typename ...Ts> struct at_impl<I, T, Ts...> : at_impl<I-1, Ts...> {};

// recursive
template<typename T>
struct recursive {
    explicit recursive() : p(new T()) {}
    explicit recursive(T const & x) : p(new T(x)) {}
    explicit recursive(T && x) : p(new T(std::move(x))) {}
    template<typename ...Args, DESALT_TAGGED_UNION_REQUIRE(std::is_constructible<T, Args &&...>::value)>
    explicit recursive(Args && ...args) : p(new T(std::forward<Args>(args)...)) {}
    explicit recursive(recursive const & other) : recursive(*other.p) {}
    explicit recursive(recursive && other) noexcept : p(other.p) {
        other.p = nullptr;
    }
    ~recursive() { delete p; }

    recursive & operator=(recursive const & other) & {
        *p = *other.p;
        return *this;
    }
    recursive & operator=(recursive && other) & {
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

// unwrap_impl
template<typename T> struct unwrap_impl { using type = T; };
template<typename T> struct unwrap_impl<recursive<T>> { using type = T; };

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

    template<bool cond, std::size_t I, typename ...Args>
    struct find_tag_impl : find_tag_impl<callable_with<at<I, Fs...>, Args...>::value, I+1, Args...> {};
    template<typename ...Args>
    struct find_tag_impl<false, sizeof...(Fs), Args...> : tag_t<sizeof...(Fs)> {};
    template<std::size_t I, typename ...Args>
    struct find_tag_impl<true, I, Args...> : tag_t<I-1> {};

    template<typename ...Args>
    using find_tag = typename find_tag_impl<callable_with<at<0, Fs...>, Args...>::value, 1, Args...>::type;

    template<typename ...Args, typename Tag = find_tag<Args && ...>, DESALT_TAGGED_UNION_REQUIRE(Tag::value != sizeof...(Fs))>
    auto operator()(Args && ...args) const -> decltype(auto) {
        return std::get<Tag::value>(fs)(std::forward<Args>(args)...);
    }
private:
    std::tuple<Fs...> fs;
};

// tie
template<typename ...Fs>
tie_t<Fs...> tie(Fs ...fs) {
    return { std::move(fs)... };
}

// unexpected_case
struct unexpected_case : std::logic_error {
    unexpected_case() : logic_error("unexpected case") {}
};
// bad_tag
struct bad_tag : std::invalid_argument {
    bad_tag() : invalid_argument("bad tag") {}
};

// fix
template<typename F>
auto fix(F f) {
    return here::fix_impl<F, 0>(std::move(f));
}
template<typename F, std::size_t>
auto fix_impl(F f) {
    return [f=std::move(f)] (auto && ...args) {
            return f(here::fix_impl<F, sizeof...(args)>(f), std::forward<decltype(args)>(args)...);
        };
}

// unfold_impl_1
template<typename Self, typename T>
struct unfold_impl_1 {
    using result = typename unfold_impl_2<Self, 0, T>::type;
    static constexpr bool need_protection = need_recursion_protection<0, T>::value;
    using type = typename std::conditional<need_protection, recursive<result>, result>::type;
};
template<typename Self, typename T>
struct unfold_impl_1<Self, recursive<T>>
    : id<recursive<typename unfold_impl_2<Self, 0, T>::type>>
{};

// unfold_impl_2
template<typename Union, std::size_t I, typename T>
struct unfold_impl_2 {
    template<typename U> using apply = unfold_impl_2<Union, I, U>;
    using type = typename traits::substitute_recursion_placeholder<T, apply>::type;
};
template<typename Union, std::size_t I> struct unfold_impl_2<Union, I, _r<I>> : id<Union> {};
template<typename Union, std::size_t I, typename ...Ts> struct unfold_impl_2<Union, I, tagged_union<Ts...>> : id<tagged_union<typename unfold_impl_2<Union, I+1, Ts>::type...>> {};
template<typename Union, std::size_t I, typename T> struct unfold_impl_2<Union, I, T*> : id<typename unfold_impl_2<Union, I, T>::type*> {};
template<typename Union, std::size_t I, typename T> struct unfold_impl_2<Union, I, T&> : id<typename unfold_impl_2<Union, I, T>::type&> {};
template<typename Union, std::size_t I, typename T> struct unfold_impl_2<Union, I, T&&> : id<typename unfold_impl_2<Union, I, T>::type&&> {};
template<typename Union, std::size_t I, typename T> struct unfold_impl_2<Union, I, T const> : id<typename unfold_impl_2<Union, I, T>::type const> {};
template<typename Union, std::size_t I, typename T> struct unfold_impl_2<Union, I, T volatile> : id<typename unfold_impl_2<Union, I, T>::type volatile> {};
template<typename Union, std::size_t I, typename T> struct unfold_impl_2<Union, I, T const volatile> : id<typename unfold_impl_2<Union, I, T>::type const volatile> {};
template<typename Union, std::size_t I, typename T, std::size_t N> struct unfold_impl_2<Union, I, T[N]> : id<typename unfold_impl_2<Union, I, T>::type[N]> {};
template<typename Union, std::size_t I, template<typename ...> class Tmpl, typename ...Ts> struct unfold_impl_2<Union, I, Tmpl<Ts...>> : id<Tmpl<typename unfold_impl_2<Union, I, Ts>::type...>> {};
template<typename Union, std::size_t I, template<typename ...> class Tmpl, typename ...Ts> struct unfold_impl_2<Union, I, Tmpl<type_fun, Ts...>> : id<typename Tmpl<type_fun, typename unfold_impl_2<Union, I, Ts>::type...>::type> {};
template<typename Union, std::size_t I, typename R, typename ...Args> struct unfold_impl_2<Union, I, R(Args...)> : id<typename unfold_impl_2<Union, I, R>::type(typename unfold_impl_2<Union, I, Args>::type...)> {};
template<typename Union, std::size_t I, typename R, typename ...Args> struct unfold_impl_2<Union, I, R(Args...) const> : id<typename unfold_impl_2<Union, I, R>::type(typename unfold_impl_2<Union, I, Args>::type...) const> {};
template<typename Union, std::size_t I, typename R, typename ...Args> struct unfold_impl_2<Union, I, R(Args...) volatile> : id<typename unfold_impl_2<Union, I, R>::type(typename unfold_impl_2<Union, I, Args>::type...) volatile> {};
template<typename Union, std::size_t I, typename R, typename ...Args> struct unfold_impl_2<Union, I, R(Args...) const volatile> : id<typename unfold_impl_2<Union, I, R>::type(typename unfold_impl_2<Union, I, Args>::type...) const volatile> {};
template<typename Union, std::size_t I, typename R, typename ...Args> struct unfold_impl_2<Union, I, R(Args...) &> : id<typename unfold_impl_2<Union, I, R>::type(typename unfold_impl_2<Union, I, Args>::type...) &> {};
template<typename Union, std::size_t I, typename R, typename ...Args> struct unfold_impl_2<Union, I, R(Args...) const &> : id<typename unfold_impl_2<Union, I, R>::type(typename unfold_impl_2<Union, I, Args>::type...) const &> {};
template<typename Union, std::size_t I, typename R, typename ...Args> struct unfold_impl_2<Union, I, R(Args...) volatile &> : id<typename unfold_impl_2<Union, I, R>::type(typename unfold_impl_2<Union, I, Args>::type...) volatile &> {};
template<typename Union, std::size_t I, typename R, typename ...Args> struct unfold_impl_2<Union, I, R(Args...) const volatile &> : id<typename unfold_impl_2<Union, I, R>::type(typename unfold_impl_2<Union, I, Args>::type...) const volatile &> {};
template<typename Union, std::size_t I, typename R, typename ...Args> struct unfold_impl_2<Union, I, R(Args...) &&> : id<typename unfold_impl_2<Union, I, R>::type(typename unfold_impl_2<Union, I, Args>::type...) &&> {};
template<typename Union, std::size_t I, typename R, typename ...Args> struct unfold_impl_2<Union, I, R(Args...) const &&> : id<typename unfold_impl_2<Union, I, R>::type(typename unfold_impl_2<Union, I, Args>::type...) const &&> {};
template<typename Union, std::size_t I, typename R, typename ...Args> struct unfold_impl_2<Union, I, R(Args...) volatile &&> : id<typename unfold_impl_2<Union, I, R>::type(typename unfold_impl_2<Union, I, Args>::type...) volatile &&> {};
template<typename Union, std::size_t I, typename R, typename ...Args> struct unfold_impl_2<Union, I, R(Args...) const volatile &&> : id<typename unfold_impl_2<Union, I, R>::type(typename unfold_impl_2<Union, I, Args>::type...) const volatile &&> {};
template<typename Union, std::size_t I, typename T, typename C> struct unfold_impl_2<Union, I, T (C::*)> : id<typename unfold_impl_2<Union, I, T>::type(unfold_impl_2<Union, I, C>::type::*)> {};

// need_recursion_protection
template<std::size_t I, typename T> struct need_recursion_protection {
    template<typename U> using apply = need_recursion_protection<I, U>;
    using type = typename traits::need_recursion_protection<T, apply>::type;
    using value_type = bool;
    static constexpr bool value = type::value;
    constexpr operator bool() const noexcept { return value; }
    constexpr bool operator()() const noexcept { return value; }
};
template<std::size_t I, typename T> struct need_recursion_protection<I, T*> : std::false_type {};
template<std::size_t I, typename T, typename C> struct need_recursion_protection<I, T (C::*)> : std::false_type {};
template<std::size_t I, typename T> struct need_recursion_protection<I, T const> : need_recursion_protection<I, T> {};
template<std::size_t I, typename T> struct need_recursion_protection<I, T volatile> : need_recursion_protection<I, T> {};
template<std::size_t I, typename T> struct need_recursion_protection<I, T const volatile> : need_recursion_protection<I, T> {};
template<std::size_t I, typename ...Ts> struct need_recursion_protection<I, tagged_union<Ts...>> : std::integral_constant<bool, !here::all(!need_recursion_protection<I+1, Ts>::value...)> {};
template<std::size_t I> struct need_recursion_protection<I, _r<I>> : std::true_type {};

// u<int, _>
// u<int, rec<u<int, _>>>

// u<std::tuple<int, _, _>, x>
// u<rec<std::tuple<int, u<std::tuple<int, _, _>, u<std::tuple<int, _, _>>>, x>

// _r
template<std::size_t>
struct _r {
    // To use `_r` as a value is probably failed to be substituted.
    template<int I = 0> _r() { static_assert(I - I, "_r<I> is recursion placeholder. must not use as value."); }
};

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

#undef DESALT_TAGGED_UNION_REQUIRE
#undef DESALT_TAGGED_VALID_EXPR

} // namespace detail {

namespace traits {

// need_recursion_protection
template<typename, template<typename> class, typename>
struct need_recursion_protection : std::false_type {};

template<typename T, typename U, template<typename> class Pred>
struct need_recursion_protection<std::pair<T, U>, Pred>
    : std::integral_constant<bool, Pred<T>::value || Pred<U>::value>
{};

template<typename ...Ts, template<typename> class Pred>
struct need_recursion_protection<std::tuple<Ts...>, Pred>
    : std::integral_constant<bool, !detail::all(!Pred<Ts>::value...)>
{};

template<typename T, std::size_t N, template<typename> class Pred>
struct need_recursion_protection<std::array<T, N>, Pred> : Pred<T> {};

// substitute_recursion_placeholder
template<typename T, template<typename> class, typename>
struct substitute_recursion_placeholder : detail::id<T> {};

template<typename T, std::size_t N, template<typename> class Pred>
struct substitute_recursion_placeholder<std::array<T, N>, Pred>
    : detail::id<std::array<typename Pred<T>::type, N>>
{};

} // namespace traits {

using detail::tagged_union;
using detail::tag_t;
using detail::recursive;
using detail::tie;
using detail::fix;
using detail::_;
using detail::_r;
using detail::type_fun;

constexpr tag_t<0> _0{};
constexpr tag_t<1> _1{};
constexpr tag_t<2> _2{};
constexpr tag_t<3> _3{};
constexpr tag_t<4> _4{};
constexpr tag_t<5> _5{};
constexpr tag_t<6> _6{};
constexpr tag_t<7> _7{};
constexpr tag_t<8> _8{};
constexpr tag_t<9> _9{};

}} // namespace desalt { namespace tagged_union {

#if defined __GNUC__ || __clang__
    #pragma GCC diagnostic pop
#endif

#endif