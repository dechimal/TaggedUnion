
#include <array>
#include <cassert>
#include <tuple>
#include <vector>

#include "desalt/datatypes.hpp"

using desalt::datatypes::_0;
using desalt::datatypes::_1;
using desalt::datatypes::_2;
using desalt::datatypes::_3;
using desalt::datatypes::sum;
using desalt::datatypes::rec_guard;
using desalt::datatypes::tag;
using desalt::datatypes::tie;
using desalt::datatypes::fix;
using desalt::datatypes::_;
using desalt::datatypes::rec;
using desalt::datatypes::type_fun;
using desalt::datatypes::extend;
using desalt::datatypes::extend_right;

struct hoge {
    hoge(int x) : x(x) {}
    hoge(hoge const & other) : b(other.b), x(other.x) {
        if (b) throw 0;
    }
    hoge(hoge && other) : b(other.b), x(other.x) {
        if (b) throw 0;
    }
    hoge & operator=(hoge const & other) {
        if (other.b) throw 0;
        x = other.x;
        b = other.b;
        return *this;
    }
    hoge & operator=(hoge && other) {
        *this = other;
        return *this;
    }

    friend bool operator==(hoge const & a, hoge const & b) {
        return a.x == b.x;
    }
    bool b = false;
    int x;
};

template<typename T, std::size_t N, typename U>
struct non_type_parameter_template {
    T x[N];
    U y[N];
};
template<typename T, std::size_t N, typename U>
struct make_non_type_parameter_template {
    template<template<typename> typename F>
    using apply = non_type_parameter_template<
        F<T>, N, F<U>
    >;
};

template<typename>
[[deprecated]] void print() {}

int main() {
    {
        // construct, assign, exception safety
        sum<int, double, hoge, hoge> a(_1, 10.0);
        assert(a.get(_1) == 10.0);
        auto b = a;
        assert(b.get(_1) == 10.0);
        a = { _0, 5 };
        assert(a.get(_0) == 5);
        a = { _2, hoge{42} };
        a.get(_2).b = true;
        try {
            b = a;
            assert(false);
        } catch (int) {
            assert(b.get(_1) == 10.0);
        }
        a.get(_2).b = false;
        a = { _3, hoge{24} };
        assert(a.get(_3).x == 24);
        try {
            b = { _2, hoge{36} };
            assert(b.get(_2).x == 36);
            b.get(_2).b = true;
            a = b;
            assert(false);
        } catch (int) {
            assert(a.get(_0) == 0);
        }
    }
    {
        // strong guarantee without fallback type
        sum<hoge, hoge> a(_0, hoge{42});
        sum<hoge, hoge> b(_1, hoge{24});
        b.get(_1).b = true;
        try {
            a = b;
            assert(false);
        } catch (int) {
            assert(a.get(_0).x == 42);
        }
    }
    {
        // equality
        sum<int, hoge> a(_0, 42);
        sum<int, hoge> b(_1, hoge{42});
        assert(a != b);
        a = { _1, hoge{42} };
        assert(a == b);
    }
    {
        // strong guarantee without nothrow constructible type
        sum<hoge, hoge> a(_0, hoge{42});
        sum<hoge, hoge> b(_1, hoge{42});
        b.get(_1).b = true;
        try {
            a = b;
            assert(false);
        } catch (int) {
            assert(a.get(_0) == b.get(_1));
        }
    }
    {
        // less
        sum<int, double> a(_0, 42);
        sum<int, double> b(_1, 10.0);
        assert(a < b);
    }
    {
        // move
        sum<int, std::vector<int>> a(_0, 10);
        sum<int, std::vector<int>> b(_1, {1, 2, 3});
        a = std::move(b);
        auto & v1 = a.get(_1);
        auto v2 = {1, 2, 3};
        assert(std::equal(v1.begin(), v1.end(), v2.begin(), v2.end()));
        auto & v3 = b.get(_1);
        assert(v3.empty());
    }
    {
        // recursive type using class member
        struct leaf {};
        struct node {
            node(int x, leaf l, leaf r) : x(x), l(_0, l), r(_0, r) {}
            node(int x, node l, leaf r) : x(x), l(_1, l), r(_0, r) {}
            node(int x, leaf l, node r) : x(x), l(_0, l), r(_1, r) {}
            node(int x, node l, node r) : x(x), l(_1, l), r(_1, r) {}
            int x;
            sum<leaf, rec_guard<node>> l; // rec_guard<T> prevent cyclic class definition.
            sum<leaf, rec_guard<node>> r;
        };
        node n(1,
               leaf{},
               node(2,
                    node(3,
                         leaf{},
                         leaf{}),
                    node(4,
                         node(5,
                              leaf{},
                              leaf{}),
                         leaf{})));
        auto g = ::fix(::tie(
            [] (auto, tag<0>, leaf) {
                return 0;
            }, [] (auto f, tag<1>, node const & n) -> int {
                return n.x + n.l.when(f) + n.r.when(f);
            }));
        assert(n.x + n.l.when(g) + n.r.when(g) == 15);
    }
    {
        // direct recursive type
        struct leaf {};
        using node = sum<leaf, std::tuple<int, _, _>>;
        auto make_leaf = [] () { return node(_0, leaf{}); };
        auto make_node = [] (int v, node l, node r) {
                return node(_1, std::make_tuple(v, std::move(l), std::move(r)));
            };
        auto n = make_node(1,
                           make_leaf(),
                           make_node(2,
                                     make_node(3,
                                               make_leaf(),
                                               make_leaf()),
                                     make_node(4,
                                               make_node(5,
                                                         make_leaf(),
                                                         make_leaf()),
                                               make_leaf())));
        auto g = ::fix(::tie(
            [] (auto, tag<0>, leaf) {
                return 0;
            }, [] (auto f, tag<1>, std::tuple<int, node, node> const & tup) -> int {
                return std::get<0>(tup) + std::get<1>(tup).when(f) + std::get<2>(tup).when(f);
            }));
        assert(n.when(g) == 15);
    }
    {
        // recursion with nest
        using u1 = sum<int, _>;
        using u2 = sum<_, u1, char>;
        static_assert(std::is_same<decltype(std::declval<u1&>().get(_1)), u1&>::value, "failed at recursion type 1");
        static_assert(std::is_same<decltype(std::declval<u2&>().get(_0)), u2&>::value, "failed at recursion type 2");
        u2 x = u2(_1, u1(_1, u1(_0, 42)));
        assert(x.get(_1).get(_1).get(_0) == 42);
    }
    {
        // fallback type in recursion type
        using u = sum<std::tuple<_>, char>;
        u x(_1, 'a');
        assert(x == x);
    }
    {
        // recursion with nested and outer placeholder
        using u = sum<sum<_, rec<1>, char>, int>;
        static_assert(std::is_same<decltype(std::declval<u&>().get(_0)), sum<_, u, char>&>::value, "failed at recursion with nested and outer placeholder 1");
        static_assert(std::is_same<decltype(std::declval<u&>().get(_0).get(_1)), u&>::value, "failed at recursion with nested and outer placeholder 2");
        u a(_1, 42);
        u b(_0, { _0, { _2, 'a' } });
        u c(_0, { _1, { _1, 42 } });
        u d(_0, { _2, 'a' });
        assert(a == c.get(_0).get(_1));
        assert(b.get(_0).get(_0) == d.get(_0));
        a = { _0, { _1, d } };
        assert(a.get(_0).get(_1).get(_0) == b.get(_0).get(_0));
    }
    {
        // implicit conversions
        struct unit {
            bool operator==(unit) const { return true; }
        };
        using ilist = sum<unit, std::tuple<int, _>>;
        using clist = sum<unit, std::tuple<char, _>>;

        ilist xs(_1, std::make_tuple(42, ilist(_0)));
        clist ys(_1, std::make_tuple(42, clist(_0)));
        assert(xs == ys);
    }
    {
        // recursion with non-type parameter using traits
        // (traits of std::array is already defined in this implementation)
        using ternary_tree = sum<int, std::array<_, 3>>;
        auto n = ternary_tree(_1, {{{_0, 1}, {_0, 2}, {_0, 3}}});
        n.when(::tie(
            [] (tag<0>, int) {
                assert(false);
            },
            [] (tag<1>, std::array<ternary_tree, 3> const & ar) {
                assert(ar[0].get(_0) == 1 && ar[1].get(_0) == 2 && ar[2].get(_0) == 3);
            }));
        // Following is also OK (same with above).
        n.when(
            [] (tag<0>, int) {
                assert(false);
            },
            [] (tag<1>, std::array<ternary_tree, 3> const & ar) {
                assert(ar[0].get(_0) == 1 && ar[1].get(_0) == 2 && ar[2].get(_0) == 3);
            });
    }
    {
        // recursion with non-type parameter (ad-hoc way)
        using type = type_fun<make_non_type_parameter_template<_, 2, int>>;
        using u = sum<rec_guard<type>, int>;
        // print<decltype(std::declval<u>().get(_0))>;
        u x(_1, 42);
        using expected = non_type_parameter_template<u, 2, int>;
        static_assert(std::is_same<decltype(std::declval<u>().get(_0)), expected &&>::value, "failed at recursion with non-type parameter (ad-hoc way)");
    }
    {
        // return type deduction of dispatch/when member function
        using u = sum<int, int, int, int>;
        struct hogera{};
        struct piyo{ piyo()=default; piyo(hogera){} };
        struct fuga{ fuga()=default; fuga(piyo){} };
        struct hoge{ hoge()=default; hoge(fuga){} };

        u x(_0, {});
        x.dispatch(::tie([] (tag<0>) {
            return hoge{};
        }, [] (tag<1>) {
            return fuga{};
        }, [] (tag<2>) {
            return piyo{};
        }, [] (tag<3>) {
            return hogera{};
        }));
    }
    {
        // extension
        using u = sum<int, int>;
        auto x = u(_0, 42);
        auto y = ::extend<tag<1>, char, tag<0>>(x);
        static_assert(std::is_same<decltype(y), sum<int, char, int>>::value, "failed at extension");
        assert(y.which() == 2 && y.get(_2) == 42);
        auto z = ::extend_right<short>(x);
        static_assert(std::is_same<decltype(z), sum<short, int, int>>::value, "failed at extension");
        assert(z.which() == 1 && z.get(_1) == 42);
    }
}
