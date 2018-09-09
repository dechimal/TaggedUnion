#if !defined DESALT_DATATYPES_SUM_HPP_INCLUDED_
#define      DESALT_DATATYPES_SUM_HPP_INCLUDED_

#include <type_traits>
#include <utility>
#include <stdexcept>
#include <tuple>
#include <limits>
#include <cstdint>
#include <array>
#include <desalt/datatypes/utils.hpp>
#include <desalt/datatypes/recursion.hpp>

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

namespace desalt { namespace datatypes {

namespace detail {
namespace here = detail;

namespace sum {
namespace here = sum;

using utils::tag;

template<typename ...> struct sum;
template<typename, typename> struct visitor_table;
template<typename T> inline void destroy(T &);
template<std::size_t, typename ...> struct find_fallback_type;
struct unexpected_case;
struct bad_tag;
template<typename ...Ts, typename ...Us> bool operator==(sum<Ts...> const &, sum<Us...> const &);
template<typename ...Ts, typename ...Us> bool operator!=(sum<Ts...> const &, sum<Us...> const &);
template<typename ...Ts, typename ...Us> bool operator<(sum<Ts...> const &, sum<Us...> const &);
template<typename ...Ts, typename ...Us> bool operator>(sum<Ts...> const &, sum<Us...> const &);
template<typename ...Ts, typename ...Us> bool operator<=(sum<Ts...> const &, sum<Us...> const &);
template<typename ...Ts, typename ...Us> bool operator>=(sum<Ts...> const &, sum<Us...> const &);
template<typename ...> struct deduce_return_type_impl;
template<typename Union> struct is_sum;
template<typename ..., typename Union, DESALT_DATATYPES_REQUIRE(is_sum<std::decay_t<Union>>{})> decltype(auto) extend(Union &&);
template<typename ..., typename Union, DESALT_DATATYPES_REQUIRE(is_sum<std::decay_t<Union>>{})> decltype(auto) extend_left(Union &&);
template<typename ..., typename Union, DESALT_DATATYPES_REQUIRE(is_sum<std::decay_t<Union>>{})> decltype(auto) extend_right(Union &&);
template<typename, typename> struct extended_element_impl;
template<std::size_t, std::size_t, typename ...> struct extended_tag_impl;
template<std::size_t N> constexpr auto size_type_impl();

template<typename ...Ts> using deduce_return_type = typename deduce_return_type_impl<Ts...>::type;
template<typename Union, typename T> using extended_element = typename extended_element_impl<Union, T>::type;
template<std::size_t I, typename ...Ts> using extended_tag = typename extended_tag_impl<I, 0, Ts...>::type;
template<std::size_t N> using size_type = decltype(here::size_type_impl<N>());


// implementations

// sum
template<typename ...Ts>
class sum {
    template<typename T> using unfold = rec::unfold<sum, T>;
    using fallback_tag = typename find_fallback_type<0, unfold<Ts>...>::type;

public:
    static constexpr bool enable_fallback = fallback_tag::value != sizeof...(Ts);
    using which_type = here::size_type<(sizeof...(Ts) + enable_fallback * 2)>;
    static constexpr which_type elements_size = sizeof...(Ts);

private:
    template<std::size_t I> using stored = rec::nth_unfold<sum, I, Ts...>;
    template<std::size_t I> using element = rec::nth_element<sum, I, Ts...>;

    static constexpr which_type backup_mask = (which_type)~((which_type)~0 >> 1);
    static_assert(((elements_size + enable_fallback) & backup_mask) == 0, "too many elements.");

public:

    template<std::size_t I>
    sum(tag<I> t, element<I> const & x) : which_(t.value) {
        this->construct_directly(t, x);
    }
    template<std::size_t I>
    sum(tag<I> t, element<I> && x) : which_(t.value) {
        this->construct_directly(t, std::move(x));
    }
    template<std::size_t I, typename ...Args,
             DESALT_DATATYPES_REQUIRE(std::is_constructible<element<I>, Args &&...>::value)>
    sum(tag<I> t, Args && ...args) : which_(t.value) {
        this->construct_directly(t, std::forward<Args>(args)...);
    }
    sum(sum const & other) : which_(other.which()) {
        this->construct(other);
    }
    sum(sum && other) : which_(other.which()) {
        this->construct(std::move(other));
    }
    template<typename ...Us, DESALT_DATATYPES_REQUIRE(utils::all(std::is_constructible<Ts, Us>::value...))>
    sum(sum<Us...> const & other) : which_(other.which()) {
        this->construct(other);
    }
    template<typename ...Us, DESALT_DATATYPES_REQUIRE(utils::all(std::is_constructible<Ts, Us>::value...))>
    sum(sum<Us...> && other) : which_(other.which()) {
        this->construct(std::move(other));
    }
    ~sum() {
        destroy();
    }

    sum & operator=(sum const & other) & {
        this->assign(other);
        return *this;
    }
    sum & operator=(sum && other) & {
        this->assign(std::move(other));
        return *this;
    }
    template<typename ...Us>
    typename std::enable_if<utils::all(std::is_assignable<Ts, Us>::value...), sum &>::type operator=(sum<Us...> const & other) & {
        this->assign(other);
        return *this;
    }
    template<typename ...Us>
    typename std::enable_if<utils::all(std::is_assignable<Ts, Us>::value...), sum &>::type operator=(sum<Us...> && other) & {
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
    decltype(auto) dispatch(F f) const {
        // following commented out code is broken still clang 3.6.2
        // using iseq = std::index_sequence_for<Ts...>;
        // return here::visitor_table<F, iseq>::table[which()](f);
        return here::visitor_table<F, std::index_sequence_for<Ts...>>::table[which()](f);
    }
    template<typename ...Fs>
    decltype(auto) dispatch(Fs ...fs) const {
        return dispatch(utils::tie(std::move(fs)...));
    }

    // the result type is
    // `decltype(true ? f(tag<0>{}, this->get(tag<0>{}))
    //         : true ? f(tag<1>{}, this->get(tag<1>{})) : ...
    //           true ? f(tag<N - 2>{}, this->get(tag<N - 2>{})) : f(tag<N - 1>{}, this->get(tag<N - 1>{})))`,
    // where `N` is `sizeof...(Ts)`.
    template<typename F> decltype(auto) when(F f)        & { return this->dispatch([&] (auto t) -> decltype(auto) { return f(t, this->get_unchecked(t)); }); }
    template<typename F> decltype(auto) when(F f) const  & { return this->dispatch([&] (auto t) -> decltype(auto) { return f(t, this->get_unchecked(t)); }); }
    template<typename F> decltype(auto) when(F f)       && { return this->dispatch([&] (auto t) -> decltype(auto) { return f(t, this->get_unchecked(t)); }); }
    template<typename F> decltype(auto) when(F f) const && { return this->dispatch([&] (auto t) -> decltype(auto) { return f(t, this->get_unchecked(t)); }); }
    template<typename ...Fs> decltype(auto) when(Fs ...fs)        & { return when(utils::tie(std::move(fs)...)); }
    template<typename ...Fs> decltype(auto) when(Fs ...fs) const  & { return when(utils::tie(std::move(fs)...)); }
    template<typename ...Fs> decltype(auto) when(Fs ...fs)       && { return when(utils::tie(std::move(fs)...)); }
    template<typename ...Fs> decltype(auto) when(Fs ...fs) const && { return when(utils::tie(std::move(fs)...)); }

private:
    template<std::size_t I, typename E = bad_tag>
    stored<I> & get_impl(tag<I> t) {
        if (t.value == which()) return get_unchecked_impl(t);
        else throw E();
    }
    template<std::size_t I, typename E = bad_tag>
    stored<I> const & get_impl(tag<I> t) const {
        if (t.value == which()) return get_unchecked_impl(t);
        else throw E();
    }
    template<std::size_t I, bool cond = enable_fallback, DESALT_DATATYPES_REQUIRE(cond)>
    stored<I> & get_unchecked_impl(tag<I> t) {
        static_assert(t.value < elements_size, "the index is too large.");
        return get_typed(t);
    }
    template<std::size_t I, bool cond = enable_fallback, DESALT_DATATYPES_REQUIRE(!cond), typename = void>
    stored<I> & get_unchecked_impl(tag<I> t) {
        static_assert(t.value < elements_size, "the index is too large.");
        if (!this->backedup()) return get_typed(t);
        else return get_backup(t);
    }
    template<std::size_t I, bool cond = enable_fallback, DESALT_DATATYPES_REQUIRE(cond)>
    stored<I> const & get_unchecked_impl(tag<I> t) const {
        static_assert(t.value < elements_size, "the index is too large.");
        return get_typed(t);
    }
    template<std::size_t I, bool cond = enable_fallback, DESALT_DATATYPES_REQUIRE(!cond), typename = void>
    stored<I> const & get_unchecked_impl(tag<I> t) const {
        static_assert(t.value < elements_size, "the index is too large.");
        if (!backedup()) return get_typed(t);
        else return get_backup(t);
    }
    template<typename Tag, typename T = stored<Tag::value>>
    T & get_typed(Tag) {
        return *reinterpret_cast<T*>(&storage_);
    }
    template<typename Tag, typename T = stored<Tag::value>>
    T const & get_typed(Tag) const {
        return *reinterpret_cast<T const *>(&storage_);
    }
    template<typename Tag, typename T = stored<Tag::value>, bool cond = enable_fallback, DESALT_DATATYPES_REQUIRE(!cond)>
    T & get_backup(Tag) {
        return **reinterpret_cast<T**>(&storage_);
    }
    template<typename Tag, typename T = stored<Tag::value>, bool cond = enable_fallback, DESALT_DATATYPES_REQUIRE(!cond)>
    T const & get_backup(Tag) const {
        return **reinterpret_cast<T * const *>(&storage_);
    }

    bool nothrow_copy_constructible() const {
        return this->dispatch([] (auto t) {
                return std::is_nothrow_copy_constructible<stored<t.value>>::value;
            });
    }
    bool nothrow_move_constructible() const {
        return this->dispatch([] (auto t) {
                return std::is_nothrow_move_constructible<stored<t.value>>::value;
            });
    }
    template<typename Union>
    bool nothrow_constructible(Union && other) const {
        using value_type = typename std::decay<Union>::type;
        return other.dispatch([] (auto t) {
                return std::is_nothrow_constructible<stored<t.value>, typename value_type::template stored<t.value>&&>::value;
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
        new(&storage_) stored<t.value>(std::forward<Args>(args)...);
    }
    void destroy() {
        this->dispatch([&] (auto t) {
                this->destroy(t);
            });
    }
    template<typename Tag, bool cond = enable_fallback, DESALT_DATATYPES_REQUIRE(cond)>
    void destroy(Tag t) {
        here::destroy(get_typed(t));
    }
    template<typename Tag, bool cond = enable_fallback, DESALT_DATATYPES_REQUIRE(!cond), typename = void>
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
                auto fallback = utils::static_if([&] (auto dep, std::enable_if_t<decltype(dep)()(std::is_lvalue_reference<Union&&>::value)> * = {}) {
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
                        utils::static_if([&] (auto dep, std::enable_if_t<decltype(dep)()(enable_fallback)> * = {}) {
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
                using other_stored = typename std::decay_t<Union>::template stored<t.value>;
                other_stored tmp(other.get_unchecked(t));
                this->destroy();
                this->construct_directly(t, std::move(tmp));
            });
    }
    template<typename Union>
    void construct_using_auto_storage_save(Union && other) {
        this->dispatch([&] (auto t) {
                stored<t.value> tmp(std::move(this->get_unchecked(t)));
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
            auto p = new stored<t.value>(std::move_if_noexcept(this->get_unchecked(t)));
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

    template<typename ...Ts1, typename ...Ts2> friend bool operator==(sum<Ts1...> const &, sum<Ts2...> const &);
    template<typename ...Ts1, typename ...Ts2> friend bool operator!=(sum<Ts1...> const &, sum<Ts2...> const &);
    template<typename ...Ts1, typename ...Ts2> friend bool operator<(sum<Ts1...> const &, sum<Ts2...> const &);
    template<typename ...Ts1, typename ...Ts2> friend bool operator>(sum<Ts1...> const &, sum<Ts2...> const &);
    template<typename ...Ts1, typename ...Ts2> friend bool operator<=(sum<Ts1...> const &, sum<Ts2...> const &);
    template<typename ...Ts1, typename ...Ts2> friend bool operator>=(sum<Ts1...> const &, sum<Ts2...> const &);

    void set_which(which_type which) {
        which_ = which;
    }
    template<bool cond = enable_fallback, DESALT_DATATYPES_REQUIRE(!cond)>
    bool backedup() const {
        return (which_ & backup_mask) != 0;
    }
    template<bool cond = enable_fallback, DESALT_DATATYPES_REQUIRE(!cond)>
    void mark_as_backup() {
        which_ |= backup_mask;
    }

    which_type which_;
    std::conditional_t<enable_fallback,
        std::aligned_union_t<0, unfold<Ts>...>,
        std::aligned_union_t<0, unfold<Ts>..., void*>> storage_;
};

template<typename ...Ts, typename ...Us>
bool operator==(sum<Ts...> const & a, sum<Us...> const & b) {
    static_assert(utils::all(utils::equality_comparable<rec::unwrap<Ts>, rec::unwrap<Us>>::value...), "each element type must be equality comparable.");
    if (a.which() != b.which()) return false;
    return a.dispatch([&] (auto t) -> bool {
            return a.get_unchecked(t) == b.get_unchecked(t);
        });
}
template<typename ...Ts, typename ...Us>
bool operator!=(sum<Ts...> const & a, sum<Us...> const & b) {
    return !(a == b);
}
template<typename ...Ts, typename ...Us>
bool operator<(sum<Ts...> const & a, sum<Us...> const & b) {
    static_assert(utils::all(utils::less_than_comparable<rec::unwrap<Ts>, rec::unwrap<Us>>::value...), "each element type must be less than comparable.");
    if (a.which() != b.which()) return a.which() < b.which();
    return a.dispatch([&] (auto t) -> bool {
            return a.get_unchecked(t) < b.get_unchecked(t);
        });
}
template<typename ...Ts, typename ...Us>
bool operator>(sum<Ts...> const & a, sum<Us...> const & b) {
    return b < a;
}
template<typename ...Ts, typename ...Us>
bool operator<=(sum<Ts...> const & a, sum<Us...> const & b) {
    return !(b < a);
}
template<typename ...Ts, typename ...Us>
bool operator>=(sum<Ts...> const & a, sum<Us...> const & b) {
    return !(a < b);
}

// visitor_table
template<typename F, std::size_t ...Is>
struct visitor_table<F, std::index_sequence<Is...>> {
    using result_type = deduce_return_type<decltype(utils::declval<F &>()(tag<Is>{}))...>;
    using visitor_type = result_type(*)(F &);
    template<std::size_t I>
    static result_type visit(F & f) {
        return convert<0, I, Is...>(f);
    }
    template<std::size_t K, std::size_t I, std::size_t J, std::size_t ...Js, DESALT_DATATYPES_REQUIRE(K != I)>
    static deduce_return_type<decltype(utils::declval<F &>()(tag<J>{})), decltype(utils::declval<F &>()(tag<Js>{}))...> convert(F & f) {
        return convert<K+1, I, Js...>(f);
    }
    template<std::size_t K, std::size_t I, std::size_t ...Js, DESALT_DATATYPES_REQUIRE(K == I), typename = void>
    static deduce_return_type<decltype(utils::declval<F &>()(tag<Js>{}))...> convert(F & f) {
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
// where `U` is a `sum<...>` contains `_`.
// `rec_guard` allocates memory in any constructor. So this check may escape.
template<std::size_t I, typename T, typename ...Ts>
struct find_fallback_type<I, rec_guard<T>, Ts...>
    : find_fallback_type<I+1, Ts...>
{};

// unexpected_case
struct unexpected_case : std::logic_error {
    unexpected_case() : logic_error("unexpected case") {}
};
// bad_tag
struct bad_tag : std::invalid_argument {
    bad_tag() : invalid_argument("bad tag") {}
};

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
    static_assert(std::is_same<deduce_return_type<Ts...>, void>{}, "failed to deduce return type in sum::when or sum::dispatch.");
    using type = deduce_return_type<Ts...>;
};
template<typename T, typename ...Ts>
struct deduce_return_type_impl<T, Ts...> {
    using type = decltype(true ? utils::declval<T>() : utils::declval<deduce_return_type<Ts...>>());
};

// extend
template<typename ...Ts, typename Union, typename>
decltype(auto) extend(Union && u) {
    return std::forward<Union>(u).when([&] (auto t, auto && x) {
            return sum<extended_element<std::decay_t<Union>, Ts>...>(extended_tag<t.value, Ts...>{}, std::forward<decltype(x)>(x));
        });
}

// extend_left
template<typename ...Ts, typename Union, typename>
decltype(auto) extend_left(Union && u) {
    return utils::with_index_sequence<std::decay_t<decltype(u)>::elements_size>([&] (auto ...is) -> decltype(auto) {
        return here::extend<Ts..., tag<is.value>...>(std::forward<Union>(u));
    });
}

// extend_right
template<typename ...Ts, typename Union, typename>
decltype(auto) extend_right(Union && u) {
    return utils::with_index_sequence<std::decay_t<decltype(u)>::elements_size>([&] (auto ...is) -> decltype(auto) {
        return here::extend<Ts..., tag<is.value>...>(std::forward<Union>(u));
    });
}

// is_sum
template<typename> struct is_sum : std::false_type {};
template<typename ...Ts> struct is_sum<sum<Ts...>> : std::true_type {};

// extended_element_impl
template<typename ...Ts, typename U>
struct extended_element_impl<sum<Ts...>, U>
    : utils::id<U>
{};
template<typename ...Ts, std::size_t I>
struct extended_element_impl<sum<Ts...>, tag<I>>
    : utils::id<rec::nth_element<sum<Ts...>, I, Ts...>>
{};

// extended_tag_impl
template<std::size_t I, std::size_t Res, typename ...Ts>
struct extended_tag_impl<I, Res, tag<I>, Ts...>
    : utils::id<tag<Res>>
{};
template<std::size_t I, std::size_t Res>
struct extended_tag_impl<I, Res>
    : utils::id<tag<Res>>
{};
template<std::size_t I, std::size_t Res, typename T, typename ...Ts>
struct extended_tag_impl<I, Res, T, Ts...>
    : extended_tag_impl<I, Res + 1, Ts...>
{};

// size_type_impl
template<std::size_t N>
constexpr auto size_type_impl() {
    return utils::static_if([] (auto, std::enable_if_t<(N <= std::numeric_limits<std::uint_least8_t>::max())> * = {}) {
        return (std::uint_least8_t)0;
    }, [] (auto, std::enable_if_t<(N <= std::numeric_limits<std::uint_least16_t>::max())> * = {}) {
        return (std::uint_least16_t)0;
    }, [] (auto, std::enable_if_t<(N <= std::numeric_limits<std::uint_least32_t>::max())> * = {}) {
        return (std::uint_least32_t)0;
    }, [] (auto) {
        return (std::uint_least64_t)0;
    });
}

} // namespace sum {
} // namespace detail {

namespace traits {

template<typename ...Ts, template<typename> class Subst>
struct substitute_recursion_placeholder<detail::sum::sum<Ts...>, Subst>
    : detail::utils::id<detail::sum::sum<typename start_new_rec<Subst, Ts>::type...>>
{};

template<typename ...Ts, template<typename> class Pred>
struct need_rec_guard<detail::sum::sum<Ts...>, Pred>
    : std::integral_constant<bool, !detail::utils::all(!start_new_rec<Pred, Ts>::value...)>
{};

} // namespace traits {

using detail::sum::sum;
using detail::sum::extend;
using detail::sum::extend_left;
using detail::sum::extend_right;

}} // namespace desalt { namespace datatypes {

#if defined __GNUC__ || __clang__
    #pragma GCC diagnostic pop
#endif

#endif
