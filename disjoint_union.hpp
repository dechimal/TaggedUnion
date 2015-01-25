#include <type_traits>
#include <utility>
#include <stdexcept>

namespace desalt {

namespace detail { namespace disjoint_union {

namespace here = disjoint_union;

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
template<typename T, DESALT_DISJOINT_UNION_VALID_EXPR(std::declval<T>() == std::declval<T>())> std::true_type equality_comparable(int);
template<typename> std::false_type equality_comparable(...);
template<typename T, DESALT_DISJOINT_UNION_VALID_EXPR(std::declval<T>() < std::declval<T>())> std::true_type less_than_comparable(int);
template<typename> std::false_type less_than_comparable(...);
template<std::size_t, typename ...> struct at;


// implementations

// disjoint_union
template<typename ...Ts>
class disjoint_union {
    using fallback_tag = typename find_fallback_type<0, Ts...>::type;
    static constexpr std::size_t element_size = sizeof...(Ts);
    static constexpr bool enable_fallback = fallback_tag::value != element_size;
    static constexpr std::size_t backup_mask = ~(~(std::size_t)0 >> 1);
    template<std::size_t I> using element = typename here::at<I, Ts...>::type;

    static_assert(((element_size + enable_fallback) & backup_mask) == 0, "too many elements.");

public:
    template<std::size_t I, typename ...Args,
             DESALT_DISJOINT_UNION_REQUIRE(std::is_constructible<element<I>, Args &&...>::value)>
    disjoint_union(tag_t<I> t, Args && ...args) : which_(t.value) {
        this->construct(t, std::forward<Args>(args)...);
    }
    template<std::size_t I>
    disjoint_union(tag_t<I> t, element<I> const & x) : which_(t.value) {
        this->construct(t, x);
    }
    template<std::size_t I>
    disjoint_union(tag_t<I> t, element<I> && x) : which_(t.value) {
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
                other.copy_construct_to(&storage_);
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
                other.move_construct_to(&storage_);
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
    auto match(F f) -> decltype(auto) {
        return this->dispatch([&] (auto t) {
                return f(t.value, get_unchecked(t));
            });
    }

    friend bool operator==(disjoint_union const & a, disjoint_union const & b) {
        static_assert(here::all(decltype(here::equality_comparable<Ts>(0))::value...), "each element type must be equality comparable.");
        if (a.which() != b.which()) return false;
        return a.dispatch([&] (auto t) {
                return a.get_unchecked(t) == b.get_unchecked(t);
            });
    }
    friend bool operator!=(disjoint_union const & a, disjoint_union const & b) {
        return !(a == b);
    }
    friend bool operator<(disjoint_union const & a, disjoint_union const & b) {
        static_assert(here::all(decltype(here::less_than_comparable<Ts>(0))::value...), "each element type must be less than comparable.");
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
        if (t.value == which()) return get_unchecked(t);
        else throw std::invalid_argument("bad index.");
    }
    template<std::size_t I>
    element<I> const & get_impl(tag_t<I> t) const {
        if (t.value == which()) return get_unchecked(t);
        else throw std::invalid_argument("bad index.");
    }
    template<std::size_t I, bool cond = enable_fallback, DESALT_DISJOINT_UNION_REQUIRE(cond)>
    element<I> & get_unchecked_impl(tag_t<I> t) {
        static_assert(t.value < element_size, "index is too large.");
        return get_typed(t);
    }
    template<std::size_t I, bool cond = enable_fallback, DESALT_DISJOINT_UNION_REQUIRE(!cond), typename = void>
    element<I> & get_unchecked_impl(tag_t<I> t) {
        static_assert(t.value < element_size, "index is too large.");
        if (!backedup()) return get_typed(t);
        else return get_backup(t);
    }
    template<std::size_t I, bool cond = enable_fallback, DESALT_DISJOINT_UNION_REQUIRE(cond)>
    element<I> const & get_unchecked_impl(tag_t<I> t) const {
        static_assert(t.value < element_size, "index is too large.");
        return get_typed(t);
    }
    template<std::size_t I, bool cond = enable_fallback, DESALT_DISJOINT_UNION_REQUIRE(!cond), typename = void>
    element<I> const & get_unchecked_impl(tag_t<I> t) const {
        static_assert(t.value < element_size, "index is too large.");
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

    template<typename F>
    auto dispatch(F f) const -> decltype(auto) {
        // using iseq = std::index_sequence_for<Ts...>;
        // return here::visitor_table<F, iseq>::table[which()](f);
        return here::visitor_table<F, std::index_sequence_for<Ts...>>::table[which()](f);
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
                auto p = new element<t.value>(this->get_unchecked(t));
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
                auto p = new element<t.value>(std::move(this->get_unchecked(t)));
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
template<typename T, typename ...Ts> struct at<0, T, Ts...> { using type = T; };
template<std::size_t I, typename T, typename ...Ts> struct at<I, T, Ts...> : at<I-1, Ts...> {};

#undef DESALT_DISJOINT_UNION_REQUIRE
#undef DESALT_DISJOINT_VALID_EXPR

}} // namespace detail { namespace disjoint_union {

using detail::disjoint_union::disjoint_union;
using detail::disjoint_union::tag_t;
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

}
