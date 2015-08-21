#if !defined DESALT_TAGGED_UNION_HPP_INCLUDED_
#define      DESALT_TAGGED_UNION_HPP_INCLUDED_

#include <type_traits>
#include <utility>
#include <stdexcept>
#include <tuple>
#include <limits>
#include <cstdint>

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

template<typename, template<typename> class, typename = void> struct need_rec_guard;
template<typename, template<typename> class, typename = void> struct substitute_recursion_placeholder;

} // namespace traits {

namespace detail {

namespace here = detail;

template<std::size_t I> struct tag;
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
template<typename> struct rec_guard;
template<typename> struct unwrap_impl;
template<typename ...Fs> struct tie_t;
template<typename ...Fs> tie_t<Fs...> tie(Fs ...fs);
template<typename F, typename ...Ts, DESALT_TAGGED_UNION_VALID_EXPR(std::declval<F>()(std::declval<Ts>()...))> constexpr std::true_type callable_with_test(int);
template<typename ...> constexpr std::false_type callable_with_test(...);
struct unexpected_case;
struct bad_tag;
template<typename, typename> struct unfold_impl_1;
template<typename, std::size_t, typename> struct unfold_impl_2;
template<std::size_t, typename> struct need_rec_guard;
template<std::size_t> struct rec;
template<std::size_t I> bool operator==(rec<I>, rec<I>);
template<std::size_t I> bool operator<(rec<I>, rec<I>);
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
template<typename ...> struct deduce_return_type_impl;
template<typename T> T declval();
template<typename Union> struct is_tagged_union;
template<typename ..., typename Union, DESALT_TAGGED_UNION_REQUIRE(is_tagged_union<std::decay_t<Union>>{})> decltype(auto) extend(Union &&);
template<typename ..., typename Union, DESALT_TAGGED_UNION_REQUIRE(is_tagged_union<std::decay_t<Union>>{})> decltype(auto) extend_left(Union &&);
template<typename ..., typename Union, DESALT_TAGGED_UNION_REQUIRE(is_tagged_union<std::decay_t<Union>>{})> decltype(auto) extend_right(Union &&);
template<typename, typename> struct extended_element_impl;
template<std::size_t, std::size_t, typename ...> struct extended_tag_impl;
template<std::size_t, typename F> constexpr decltype(auto) with_index_sequence(F);
template<std::size_t ...Is, typename F> constexpr decltype(auto) with_index_sequence_impl(std::index_sequence<Is...>, F);
template<std::size_t N> constexpr auto size_type_impl();

template<typename T> using unwrap = typename unwrap_impl<T>::type;
template<std::size_t I, typename ...Ts> using at = typename at_impl<I, Ts...>::type;
template<typename T, typename U> using equality_comparable = decltype(here::equality_comparable_test<T, U>(0));
template<typename T, typename U> using less_than_comparable = decltype(here::less_than_comparable_test<T, U>(0));
template<typename F, typename ...Ts> using callable_with = decltype(here::callable_with_test<F, Ts...>(0));
template<typename Union, typename T> using unfold = typename unfold_impl_1<Union, T>::type;
template<typename Union, std::size_t I, typename ...Ts> using actual_element = at<I, unfold<Union, Ts>...>;
template<typename Union, std::size_t I, typename ...Ts> using element = unwrap<actual_element<Union, I, Ts...>>;
template<typename ...Ts> using deduce_return_type = typename deduce_return_type_impl<Ts...>::type;
template<typename Union, typename T> using extended_element = typename extended_element_impl<Union, T>::type;
template<std::size_t I, typename ...Ts> using extended_tag = typename extended_tag_impl<I, 0, Ts...>::type;
template<std::size_t N> using size_type = decltype(size_type_impl<N>());


// implementations

// tagged_union
template<typename ...Ts>
class tagged_union {
    template<typename T> using unfold = here::unfold<tagged_union, T>;
    using fallback_tag = typename find_fallback_type<0, unfold<Ts>...>::type;

public:
    static constexpr bool enable_fallback = fallback_tag::value != sizeof...(Ts);
    using which_type = here::size_type<(sizeof...(Ts) + enable_fallback * 2)>;
    static constexpr which_type elements_size = sizeof...(Ts);

private:
    template<std::size_t I> using actual_element = here::actual_element<tagged_union, I, Ts...>;
    template<std::size_t I> using element = here::element<tagged_union, I, Ts...>;

    static constexpr which_type backup_mask = (which_type)~((which_type)~0 >> 1);
    static_assert(((elements_size + enable_fallback) & backup_mask) == 0, "too many elements.");

public:

    template<std::size_t I>
    tagged_union(tag<I> t, element<I> const & x) : which_(t.value) {
        this->construct_directly(t, x);
    }
    template<std::size_t I>
    tagged_union(tag<I> t, element<I> && x) : which_(t.value) {
        this->construct_directly(t, std::move(x));
    }
    template<std::size_t I, typename ...Args,
             DESALT_TAGGED_UNION_REQUIRE(std::is_constructible<element<I>, Args &&...>::value)>
    tagged_union(tag<I> t, Args && ...args) : which_(t.value) {
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

    template<std::size_t I> element<I>        & get(tag<I> t)        & { return get_impl(t); }
    template<std::size_t I> element<I> const  & get(tag<I> t) const  & { return get_impl(t); }
    template<std::size_t I> element<I>       && get(tag<I> t)       && { return std::move(get_impl(t)); }
    template<std::size_t I> element<I> const && get(tag<I> t) const && { return std::move(get_impl(t)); }
    template<std::size_t I> element<I>        & get_unchecked(tag<I> t)        & { return get_unchecked_impl(t); }
    template<std::size_t I> element<I> const  & get_unchecked(tag<I> t) const  & { return get_unchecked_impl(t); }
    template<std::size_t I> element<I>       && get_unchecked(tag<I> t)       && { return std::move(get_unchecked_impl(t)); }
    template<std::size_t I> element<I> const && get_unchecked(tag<I> t) const && { return std::move(get_unchecked_impl(t)); }

    which_type which() const {
        return which_ & ~backup_mask;
    }

    // the result type is `decltype(true ? f(tag<0>{}) : true ? f(tag<1>{}) : ... true ? f(tag<N - 2>{}) : f(tag<N - 1>{}))`,
    // where `N` is `sizeof...(Ts)`.
    template<typename F>
    auto dispatch(F f) const -> decltype(auto) {
        // following commented out code is broken still clang 3.6.2
        // using iseq = std::index_sequence_for<Ts...>;
        // return here::visitor_table<F, iseq>::table[which()](f);
        return here::visitor_table<F, std::index_sequence_for<Ts...>>::table[which()](f);
    }

    // the result type is
    // `decltype(true ? f(tag<0>{}, this->get(tag<0>{}))
    //         : true ? f(tag<1>{}, this->get(tag<1>{})) : ...
    //           true ? f(tag<N - 2>{}, this->get(tag<N - 2>{})) : f(tag<N - 1>{}, this->get(tag<N - 1>{})))`,
    // where `N` is `sizeof...(Ts)`.
    template<typename F> auto when(F f)        & -> decltype(auto) { return this->dispatch([&] (auto t) -> decltype(auto) { return f(t, this->get_unchecked(t)); }); }
    template<typename F> auto when(F f) const  & -> decltype(auto) { return this->dispatch([&] (auto t) -> decltype(auto) { return f(t, this->get_unchecked(t)); }); }
    template<typename F> auto when(F f)       && -> decltype(auto) { return this->dispatch([&] (auto t) -> decltype(auto) { return f(t, this->get_unchecked(t)); }); }
    template<typename F> auto when(F f) const && -> decltype(auto) { return this->dispatch([&] (auto t) -> decltype(auto) { return f(t, this->get_unchecked(t)); }); }

private:
    template<std::size_t I, typename E = bad_tag>
    actual_element<I> & get_impl(tag<I> t) {
        if (t.value == which()) return get_unchecked_impl(t);
        else throw E();
    }
    template<std::size_t I, typename E = bad_tag>
    actual_element<I> const & get_impl(tag<I> t) const {
        if (t.value == which()) return get_unchecked_impl(t);
        else throw E();
    }
    template<std::size_t I, bool cond = enable_fallback, DESALT_TAGGED_UNION_REQUIRE(cond)>
    actual_element<I> & get_unchecked_impl(tag<I> t) {
        static_assert(t.value < elements_size, "the tag is too large.");
        return get_typed(t);
    }
    template<std::size_t I, bool cond = enable_fallback, DESALT_TAGGED_UNION_REQUIRE(!cond), typename = void>
    actual_element<I> & get_unchecked_impl(tag<I> t) {
        static_assert(t.value < elements_size, "the tag is too large.");
        if (!this->backedup()) return get_typed(t);
        else return get_backup(t);
    }
    template<std::size_t I, bool cond = enable_fallback, DESALT_TAGGED_UNION_REQUIRE(cond)>
    actual_element<I> const & get_unchecked_impl(tag<I> t) const {
        static_assert(t.value < elements_size, "the tag is too large.");
        return get_typed(t);
    }
    template<std::size_t I, bool cond = enable_fallback, DESALT_TAGGED_UNION_REQUIRE(!cond), typename = void>
    actual_element<I> const & get_unchecked_impl(tag<I> t) const {
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
    void construct_directly(tag<I> t, Args && ...args) {
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

    void set_which(which_type which) {
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

    which_type which_;
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
    using result_type = deduce_return_type<decltype(here::declval<F &>()(tag<Is>{}))...>;
    using visitor_type = result_type(*)(F &);
    template<std::size_t I>
    static result_type visit(F & f) {
        return convert<0, I, Is...>(f);
    }
    template<std::size_t K, std::size_t I, std::size_t J, std::size_t ...Js, DESALT_TAGGED_UNION_REQUIRE(K != I)>
    static deduce_return_type<decltype(here::declval<F &>()(tag<J>{})), decltype(here::declval<F &>()(tag<Js>{}))...> convert(F & f) {
        return convert<K+1, I, Js...>(f);
    }
    template<std::size_t K, std::size_t I, std::size_t ...Js, DESALT_TAGGED_UNION_REQUIRE(K == I), typename = void>
    static deduce_return_type<decltype(here::declval<F &>()(tag<Js>{}))...> convert(F & f) {
        return f(tag<I>{});
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
struct find_fallback_type<I> : tag<I> {};
template<std::size_t I, typename T, typename ...Ts>
struct find_fallback_type<I, T, Ts...>
    : std::conditional<std::is_nothrow_default_constructible<T>::value,
                       tag<I>,
                       find_fallback_type<I+1, Ts...>>::type
{};
// Specialization for rec_guard types. Escape default constructibility inspection of rec_guard<T>.
// Because `std::is_nothrow_default_constructible` may inspect default construtibility of `U` in instaciation of `U`,
// where `U` is a `tagged_union<...>` contains `_`.
// `rec_guard` allocates memory in any constructor. So this check may escape.
template<std::size_t I, typename T, typename ...Ts>
struct find_fallback_type<I, rec_guard<T>, Ts...>
    : find_fallback_type<I+1, Ts...>
{};

// all
constexpr bool all() { return true; }
template<typename T, typename ...Ts>
constexpr bool all(T p, Ts ...ps) {
    return p && here::all(ps...);
}

// tag
template<std::size_t I>
struct tag
    : std::integral_constant<std::size_t, I>
{
    using type = tag<I>;
};

// id
template<typename T>
struct id {
    using type = T;
};

// at
template<typename T, typename ...Ts> struct at_impl<0, T, Ts...> : id<T> {};
template<std::size_t I, typename T, typename ...Ts> struct at_impl<I, T, Ts...> : at_impl<I-1, Ts...> {};

// rec_guard
template<typename T>
struct rec_guard {
    explicit rec_guard() : p(new T()) {}
    explicit rec_guard(T const & x) : p(new T(x)) {}
    explicit rec_guard(T && x) : p(new T(std::move(x))) {}
    template<typename ...Args, DESALT_TAGGED_UNION_REQUIRE(std::is_constructible<T, Args &&...>::value)>
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

// unwrap_impl
template<typename T> struct unwrap_impl { using type = T; };
template<typename T> struct unwrap_impl<rec_guard<T>> { using type = T; };

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
    struct find_tag_impl<false, sizeof...(Fs), Args...> : tag<sizeof...(Fs)> {};
    template<std::size_t I, typename ...Args>
    struct find_tag_impl<true, I, Args...> : tag<I-1> {};

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
    static constexpr bool need_protection = need_rec_guard<0, T>::value;
    using type = typename std::conditional<need_protection, rec_guard<result>, result>::type;
};
template<typename Self, typename T>
struct unfold_impl_1<Self, rec_guard<T>>
    : id<rec_guard<typename unfold_impl_2<Self, 0, T>::type>>
{};

// unfold_impl_2
template<typename Union, std::size_t I, typename T>
struct unfold_impl_2 {
    template<typename U> using apply = unfold_impl_2<Union, I, U>;
    using type = typename traits::substitute_recursion_placeholder<T, apply>::type;
};
template<typename Union, std::size_t I> struct unfold_impl_2<Union, I, rec<I>> : id<Union> {};
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
template<std::size_t I, typename ...Ts> struct need_rec_guard<I, tagged_union<Ts...>> : std::integral_constant<bool, !here::all(!need_rec_guard<I+1, Ts>::value...)> {};
template<std::size_t I> struct need_rec_guard<I, rec<I>> : std::true_type {};

// u<int, _>
// u<int, rec<u<int, _>>>

// u<std::tuple<int, _, _>, x>
// u<rec<std::tuple<int, u<std::tuple<int, _, _>, u<std::tuple<int, _, _>>>, x>

// rec
template<std::size_t>
struct rec {
    // To use `rec` as a value is probably failed to be substituted.
    template<int I = 0> rec() { static_assert(I - I, "rec<I> is recursion placeholder. must not use as value."); }
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

// deduce_return_type_impl
template<typename T>
struct deduce_return_type_impl<T> {
    using type = T;
};
template<>
struct deduce_return_type_impl<void> {
    using type = void;
};
template<typename ...Ts>
struct deduce_return_type_impl<void, Ts...> {
    static_assert(std::is_same<deduce_return_type<Ts...>, void>{}, "failed to deduce return type in tagged_union::when or tagged_union::dispatch.");
    using type = deduce_return_type<Ts...>;
};
template<typename T, typename ...Ts>
struct deduce_return_type_impl<T, Ts...> {
    using type = decltype(true ? here::declval<T>() : here::declval<deduce_return_type<Ts...>>());
};

// extend
template<typename ...Ts, typename Union, typename>
decltype(auto) extend(Union && u) {
    return std::forward<Union>(u).when([&] (auto t, auto && x) {
            return tagged_union<extended_element<std::decay_t<Union>, Ts>...>(extended_tag<t.value, Ts...>{}, std::forward<decltype(x)>(x));
        });
}

// extend_left
template<typename ...Ts, typename Union, typename>
decltype(auto) extend_left(Union && u) {
    return here::with_index_sequence<std::decay_t<decltype(u)>::elements_size>([&] (auto ...is) -> decltype(auto) {
        return here::extend<Ts..., tag<is.value>...>(std::forward<Union>(u));
    });
}

// extend_right
template<typename ...Ts, typename Union, typename>
decltype(auto) extend_right(Union && u) {
    return here::with_index_sequence<std::decay_t<decltype(u)>::elements_size>([&] (auto ...is) -> decltype(auto) {
        return here::extend<Ts..., tag<is.value>...>(std::forward<Union>(u));
    });
}

// is_tagged_union
template<typename> struct is_tagged_union : std::false_type {};
template<typename ...Ts> struct is_tagged_union<tagged_union<Ts...>> : std::true_type {};

// extended_element_impl
template<typename ...Ts, typename U>
struct extended_element_impl<tagged_union<Ts...>, U>
    : id<U>
{};
template<typename ...Ts, std::size_t I>
struct extended_element_impl<tagged_union<Ts...>, tag<I>>
    : id<element<tagged_union<Ts...>, I, Ts...>>
{};

// extended_tag_impl
template<std::size_t I, std::size_t Res, typename ...Ts>
struct extended_tag_impl<I, Res, tag<I>, Ts...>
    : id<tag<Res>>
{};
template<std::size_t I, std::size_t Res>
struct extended_tag_impl<I, Res>
    : id<tag<Res>>
{};
template<std::size_t I, std::size_t Res, typename T, typename ...Ts>
struct extended_tag_impl<I, Res, T, Ts...>
    : extended_tag_impl<I, Res + 1, Ts...>
{};

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

// size_type_impl
template<std::size_t N>
constexpr auto size_type_impl() {
    return here::static_if([] (auto, std::enable_if_t<(N <= std::numeric_limits<std::uint_least8_t>::max())> * = {}) {
        return (std::uint_least8_t)0;
    }, [] (auto, std::enable_if_t<(N <= std::numeric_limits<std::uint_least16_t>::max())> * = {}) {
        return (std::uint_least16_t)0;
    }, [] (auto, std::enable_if_t<(N <= std::numeric_limits<std::uint_least32_t>::max())> * = {}) {
        return (std::uint_least32_t)0;
    }, [] (auto) {
        return (std::uint_least64_t)0;
    });
}

#undef DESALT_TAGGED_UNION_REQUIRE
#undef DESALT_TAGGED_VALID_EXPR

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
    : std::integral_constant<bool, !detail::all(!Pred<Ts>::value...)>
{};

template<typename T, std::size_t N, template<typename> class Pred>
struct need_rec_guard<std::array<T, N>, Pred> : Pred<T> {};

// substitute_recursion_placeholder
template<typename T, template<typename> class, typename>
struct substitute_recursion_placeholder : detail::id<T> {};

template<typename T, std::size_t N, template<typename> class Pred>
struct substitute_recursion_placeholder<std::array<T, N>, Pred>
    : detail::id<std::array<typename Pred<T>::type, N>>
{};

} // namespace traits {

using detail::tagged_union;
using detail::tag;
using detail::rec_guard;
using detail::tie;
using detail::fix;
using detail::rec;
using _ = rec<0>;
using detail::type_fun;
using detail::extend;
using detail::extend_left;
using detail::extend_right;

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

}} // namespace desalt { namespace tagged_union {

#if defined __GNUC__ || __clang__
    #pragma GCC diagnostic pop
#endif

#endif
