#include <type_traits>
#include <utility>
#include <stdexcept>
#include <tuple>

namespace desalt { namespace disjoint_union {

namespace detail {

namespace here = detail;

#define DESALT_DISJOINT_UNION_REQUIRE(...) typename = typename std::enable_if<(__VA_ARGS__)>::type
#define DESALT_DISJOINT_UNION_VALID_EXPR(...) typename = decltype((__VA_ARGS__), (void)0)

// forward declarations
template<std::size_t I> struct tag_t;
template<typename ...> struct disjoint_union;
template<typename, typename> struct visitor_table;
template<typename T> inline void destroy(T &);
template<std::size_t, typename ...> struct find_fallback_type;
template<std::size_t, typename ...> union aligned_union_t;
template<typename ...> union aligned_union_impl;
constexpr bool all();
template<typename T, typename ...Ts> constexpr bool all(T, Ts...);
template<typename T, DESALT_DISJOINT_UNION_VALID_EXPR(std::declval<T>() == std::declval<T>())> std::true_type equality_comparable_test(int);
template<typename> std::false_type equality_comparable_test(...);
template<typename T, DESALT_DISJOINT_UNION_VALID_EXPR(std::declval<T>() < std::declval<T>())> std::true_type less_comparable_test(int);
template<typename> std::false_type less_comparable_test(...);
template<std::size_t, typename ...> struct at_impl;
template<typename> struct recursive;
template<typename> struct unwrap;
template<typename ...Fs> struct tie_t;
template<typename ...Fs> tie_t<Fs...> tie(Fs ...fs);
template<typename F, typename ...Ts, DESALT_DISJOINT_UNION_VALID_EXPR(std::declval<F>()(std::declval<Ts>()...))> std::true_type callable_with_test(int);
template<typename ...> std::false_type callable_with_test(...);
struct unexpected_case;

// aliases
template<typename T> using equality_comparable = decltype(here::equality_comparable_test<T>(0));
template<typename T> using less_comparable = decltype(here::less_comparable_test<T>(0));
template<std::size_t I, typename ...Ts> using at = typename at_impl<I, Ts...>::type;
template<typename F, typename ...Ts> using callable_with = decltype(here::callable_with_test<F, Ts...>(0));

// implementations

// disjoint_union
template<typename ...Ts>
class disjoint_union {
    using fallback_tag = typename find_fallback_type<0, Ts...>::type;
    static constexpr std::size_t elements_size = sizeof...(Ts);
    static constexpr bool enable_fallback = fallback_tag::value != elements_size;
    static constexpr std::size_t backup_mask = ~(~(std::size_t)0 >> 1);
    template<std::size_t I> using element = at<I, Ts...>;
    template<std::size_t I> using unwrap_element = typename unwrap<element<I>>::type;

    static_assert(((elements_size + enable_fallback) & backup_mask) == 0, "too many elements.");

public:
    template<std::size_t I, typename ...Args,
             DESALT_DISJOINT_UNION_REQUIRE(std::is_constructible<unwrap_element<I>, Args &&...>::value)>
    disjoint_union(tag_t<I> t, Args && ...args) : which_(t.value) {
        this->construct(t, std::forward<Args>(args)...);
    }
    template<std::size_t I>
    disjoint_union(tag_t<I> t, unwrap_element<I> const & x) : which_(t.value) {
        this->construct(t, x);
    }
    template<std::size_t I>
    disjoint_union(tag_t<I> t, unwrap_element<I> && x) : which_(t.value) {
        this->construct(t, std::move(x));
    }
    disjoint_union(disjoint_union const & other) : which_(other.which_) {
        other.copy_construct_to(&storage_);
    }
    disjoint_union(disjoint_union && other) : which_(other.which_) {
        other.move_construct_to(&storage_);
    }
    ~disjoint_union() {
        destroy();
    }

    disjoint_union & operator=(disjoint_union const & other) & {
        if (which_ == other.which_) {
            this->dispatch([&] (auto t) {
                    this->get_unchecked(t) = other.get_unchecked(t);
                });
        } else {
            if (other.nothrow_copy_constructible()) {
                this->destroy();
                other.copy_construct_to(&this->storage_);
            } else if (other.nothrow_move_constructible()) {
                other.dispatch([&] (auto t) {
                        element<t.value> tmp(other.get_unchecked(t));
                        this->destroy();
                        this->construct(t, std::move(tmp));
                    });
            } else if (this->nothrow_move_constructible()) {
                this->dispatch([&] (auto t) {
                        element<t.value> tmp(std::move(this->get_unchecked(t)));
                        this->destroy(t);
                        try {
                            other.copy_construct_to(&this->storage_);
                        } catch (...) {
                            this->construct(t, std::move(tmp));
                            throw;
                        }
                    });
            } else {
                this->copy_assign_without_nothrow_guarantee(other);
            }
            this->set_which(other.which());
        }
        return *this;
    }

    disjoint_union & operator=(disjoint_union && other) & {
        if (which_ == other.which_) {
            this->dispatch([&] (auto t) {
                    this->get_unchecked(t) = std::move(other.get_unchecked(t));
                });
        } else {
            if (other.nothrow_move_constructible()) {
                this->destroy();
                other.move_construct_to(&this->storage_);
            } else if (this->nothrow_move_constructible()) {
                this->dispatch([&] (auto t) {
                        element<t.value> tmp(std::move(this->get_unchecked(t)));
                        this->destroy(t);
                        try {
                            other.move_construct_to(&this->storage_);
                        } catch (...) {
                            this->construct(t, std::move(tmp));
                            throw;
                        }
                    });
            } else {
                this->move_assign_without_nothrow_guarantee(std::move(other));
            }
            this->set_which(other.which());
        }
        return *this;
    }

    template<std::size_t I> unwrap_element<I>        & get(tag_t<I> t)        & { return get_impl(t); }
    template<std::size_t I> unwrap_element<I> const  & get(tag_t<I> t) const  & { return get_impl(t); }
    template<std::size_t I> unwrap_element<I>       && get(tag_t<I> t)       && { return std::move(get_impl(t)); }
    template<std::size_t I> unwrap_element<I> const && get(tag_t<I> t) const && { return std::move(get_impl(t)); }
    template<std::size_t I> unwrap_element<I>        & get_unchecked(tag_t<I> t)        & { return get_unchecked_impl(t); }
    template<std::size_t I> unwrap_element<I> const  & get_unchecked(tag_t<I> t) const  & { return get_unchecked_impl(t); }
    template<std::size_t I> unwrap_element<I>       && get_unchecked(tag_t<I> t)       && { return std::move(get_unchecked_impl(t)); }
    template<std::size_t I> unwrap_element<I> const && get_unchecked(tag_t<I> t) const && { return std::move(get_unchecked_impl(t)); }

    std::size_t which() const {
        return which_ & ~backup_mask;
    }

    template<typename F>
    auto dispatch(F f) const -> decltype(auto) {
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

    friend bool operator==(disjoint_union const & a, disjoint_union const & b) {
        static_assert(here::all(equality_comparable<Ts>::value...), "each element type must be equality comparable.");
        if (a.which() != b.which()) return false;
        return a.dispatch([&] (auto t) {
                return a.get_unchecked(t) == b.get_unchecked(t);
            });
    }
    friend bool operator!=(disjoint_union const & a, disjoint_union const & b) {
        return !(a == b);
    }
    friend bool operator<(disjoint_union const & a, disjoint_union const & b) {
        static_assert(here::all(less_comparable<Ts>::value...), "each element type must be less than comparable.");
        if (a.which() != b.which()) return a.which() < b.which();
        return a.dispatch([&] (auto t) {
                return a.get_unchecked(t) < b.get_unchecked(t);
            });
    }
    friend bool operator>(disjoint_union const & a, disjoint_union const & b) {
        return b < a;
    }
    friend bool operator<=(disjoint_union const & a, disjoint_union const & b) {
        return !(b < a);
    }
    friend bool operator>=(disjoint_union const & a, disjoint_union const & b) {
        return !(a < b);
    }

private:
    template<std::size_t I>
    element<I> & get_impl(tag_t<I> t) {
        if (t.value == which()) return get_unchecked_impl(t);
        else throw std::invalid_argument("bad tag.");
    }
    template<std::size_t I>
    element<I> const & get_impl(tag_t<I> t) const {
        if (t.value == which()) return get_unchecked_impl(t);
        else throw std::invalid_argument("bad tag.");
    }
    template<std::size_t I, bool cond = enable_fallback, DESALT_DISJOINT_UNION_REQUIRE(cond)>
    element<I> & get_unchecked_impl(tag_t<I> t) {
        static_assert(t.value < elements_size, "tag is too large.");
        return get_typed(t);
    }
    template<std::size_t I, bool cond = enable_fallback, DESALT_DISJOINT_UNION_REQUIRE(!cond), typename = void>
    element<I> & get_unchecked_impl(tag_t<I> t) {
        static_assert(t.value < elements_size, "tag is too large.");
        if (!backedup()) return get_typed(t);
        else return get_backup(t);
    }
    template<std::size_t I, bool cond = enable_fallback, DESALT_DISJOINT_UNION_REQUIRE(cond)>
    element<I> const & get_unchecked_impl(tag_t<I> t) const {
        static_assert(t.value < elements_size, "tag is too large.");
        return get_typed(t);
    }
    template<std::size_t I, bool cond = enable_fallback, DESALT_DISJOINT_UNION_REQUIRE(!cond), typename = void>
    element<I> const & get_unchecked_impl(tag_t<I> t) const {
        static_assert(t.value < elements_size, "tag is too large.");
        if (!backedup()) return get_typed(t);
        else return get_backup(t);
    }
    template<typename Tag, typename T = element<Tag::value>>
    T & get_typed(Tag) {
        return *reinterpret_cast<T*>(&storage_);
    }
    template<typename Tag, typename T = element<Tag::value>>
    T const & get_typed(Tag) const {
        return *reinterpret_cast<T const *>(&storage_);
    }
    template<typename Tag, typename T = element<Tag::value>, bool cond = enable_fallback, DESALT_DISJOINT_UNION_REQUIRE(!cond)>
    T & get_backup(Tag) {
        return **reinterpret_cast<T**>(&storage_);
    }
    template<typename Tag, typename T = element<Tag::value>, bool cond = enable_fallback, DESALT_DISJOINT_UNION_REQUIRE(!cond)>
    T const & get_backup(Tag) const {
        return **reinterpret_cast<T * const *>(&storage_);
    }

    bool nothrow_copy_constructible() const {
        return this->dispatch([&] (auto t) {
                return std::is_nothrow_copy_constructible<element<t.value>>::value;
            });
    }
    bool nothrow_move_constructible() const {
        return this->dispatch([&] (auto t) {
                return std::is_nothrow_move_constructible<element<t.value>>::value;
            });
    }
    void copy_construct_to(void * storage_ptr) const {
        this->dispatch([&] (auto t) {
                new(storage_ptr) element<t.value>(this->get_unchecked(t));
            });
    }
    void move_construct_to(void * storage_ptr) {
        this->dispatch([&] (auto t) {
                new(storage_ptr) element<t.value>(std::move(this->get_unchecked(t)));
            });
    }
    void destroy() {
        this->dispatch([&] (auto t) {
                this->destroy(t);
            });
    }
    template<bool cond = enable_fallback, DESALT_DISJOINT_UNION_REQUIRE(cond)>
    void copy_assign_without_nothrow_guarantee(disjoint_union const & other) {
        this->dispatch([&] (auto t) {
                try {
                    this->destroy(t);
                    other.copy_construct_to(&this->storage_);
                } catch (...) {
                    this->construct(fallback_tag{});
                    this->set_which(fallback_tag::value);
                    throw;
                }
            });
    }
    template<bool cond = enable_fallback, DESALT_DISJOINT_UNION_REQUIRE(!cond), typename = void>
    void copy_assign_without_nothrow_guarantee(disjoint_union const & other) {
        this->dispatch([&] (auto t) {
                auto p = new element<t.value>(std::move_if_noexcept(this->get_unchecked(t)));
                try {
                    this->destroy(t);
                    other.copy_construct_to(&this->storage_);
                    delete p;
                } catch (...) {
                    *reinterpret_cast<void **>(&this->storage_) = p;
                    this->mark_as_backup();
                    throw;
                }
            });
    }
    template<bool cond = enable_fallback, DESALT_DISJOINT_UNION_REQUIRE(cond)>
    void move_assign_without_nothrow_guarantee(disjoint_union && other) {
        this->dispatch([&] (auto t) {
                try {
                    this->destroy(t);
                    other.move_construct_to(&this->storage_);
                } catch (...) {
                    this->construct(fallback_tag{});
                    this->set_which(fallback_tag::value);
                    throw;
                }
            });
    }
    template<bool cond = enable_fallback, DESALT_DISJOINT_UNION_REQUIRE(!cond), typename = void>
    void move_assign_without_nothrow_guarantee(disjoint_union && other) {
        this->dispatch([&] (auto t) {
                auto p = new element<t.value>(std::move_if_noexcept(this->get_unchecked(t)));
                try {
                    this->destroy(t);
                    other.move_construct_to(&this->storage_);
                    delete p;
                } catch (...) {
                    *reinterpret_cast<void **>(&this->storage_) = p;
                    this->mark_as_backup();
                    throw;
                }
            });
    }

    template<typename Tag, bool cond = enable_fallback, DESALT_DISJOINT_UNION_REQUIRE(cond)>
    void destroy(Tag t) {
        here::destroy(get_typed(t));
    }
    template<typename Tag, bool cond = enable_fallback, DESALT_DISJOINT_UNION_REQUIRE(!cond), typename = void>
    void destroy(Tag t) {
        if (backedup()) delete &get_backup(t);
        else here::destroy(get_typed(t));
    }
    template<typename Tag, typename ...Args>
    void construct(Tag, Args && ...args) {
        new(&storage_) element<Tag::value>(std::forward<Args>(args)...);
    }
    void set_which(std::size_t which) {
        which_ = which;
    }
    template<bool cond = enable_fallback, DESALT_DISJOINT_UNION_REQUIRE(!cond)>
    bool backedup() const {
        return (which_ & backup_mask) != 0;
    }
    template<bool cond = enable_fallback, DESALT_DISJOINT_UNION_REQUIRE(!cond)>
    void mark_as_backup() {
        which_ |= backup_mask;
    }

    std::size_t which_;
    typename std::conditional<enable_fallback,
        aligned_union_t<0, Ts...>,
        aligned_union_t<0, Ts..., void*>>::type storage_;
};

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

// aligned_union_t
template<std::size_t N, typename ...Ts>
union aligned_union_t {
    aligned_union_t() {}
    ~aligned_union_t() {}
    char pad1[N > 0 ? N : 1];
    aligned_union_impl<Ts...> pad2;
};
// aligned_union_impl
template<>
union aligned_union_impl<> {};
template<typename T, typename ...Ts>
union aligned_union_impl<T, Ts...> {
    aligned_union_impl() {}
    ~aligned_union_impl() {}
    T head;
    aligned_union_impl<Ts...> tail;
};

// find_fallback_type
template<std::size_t I>
struct find_fallback_type<I> : tag_t<I> {};
template<std::size_t I, typename T, typename ...Ts>
struct find_fallback_type<I, T, Ts...>
    : std::conditional<std::is_nothrow_default_constructible<T>::value,
                       tag_t<I>,
                       find_fallback_type<I+1, Ts...>>::type
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
{};

// at
template<typename T, typename ...Ts> struct at_impl<0, T, Ts...> : std::common_type<T> {};
template<std::size_t I, typename T, typename ...Ts> struct at_impl<I, T, Ts...> : at_impl<I-1, Ts...> {};

// recursive
template<typename T>
struct recursive {
    explicit recursive() : p(new T()) {}
    explicit recursive(T const & x) : p(new T(x)) {}
    explicit recursive(T && x) : p(new T(std::move(x))) {}
    template<typename ...Args, DESALT_DISJOINT_UNION_REQUIRE(std::is_constructible<T, Args &&...>::value)>
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

// unwrap
template<typename T> struct unwrap { using type = T; };
template<typename T> struct unwrap<recursive<T>> { using type = T; };

// tie_t
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

    // `f_i` denotes result of `std::get<i>(fs)`,
    //  - if `f_i(std::forward<Args>(args)...)` is valid expression, then it calls `f_i`,
    //  - otherwise this step applies to f_i+1.
    // This behavior is intent to use as pattern matching in functional programming languages.
    template<typename ...Args, typename Tag = find_tag<Args && ...>, DESALT_DISJOINT_UNION_REQUIRE(Tag::value != sizeof...(Fs))>
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

// fix
template<typename F>
auto fix(F f) {
    return [f=std::move(f)] (auto && ...args) {
            return f(here::fix(f), std::forward<decltype(args)>(args)...);
        };
}

#undef DESALT_DISJOINT_UNION_REQUIRE
#undef DESALT_DISJOINT_VALID_EXPR

} // namespace detail {

using detail::disjoint_union;
using detail::tag_t;
using detail::recursive;
using detail::tie;
using detail::fix;
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

}} // namespace desalt { namespace disjoint_union {
