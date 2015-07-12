
#include <tuple>
#include <cassert>
#include <vector>
#include "disjoint_union.hpp"

using desalt::tagged_union::_0;
using desalt::tagged_union::_1;
using desalt::tagged_union::_2;
using desalt::tagged_union::_3;
using desalt::tagged_union::tagged_union;
using desalt::tagged_union::recursive;
using desalt::tagged_union::tag_t;
using desalt::tagged_union::tie;
using desalt::tagged_union::fix;
using desalt::tagged_union::_;
using desalt::tagged_union::_r;
using desalt::tagged_union::type_fun;

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

template<typename Tag, typename T, typename N, typename U>
struct make_non_type_parameter_template;
template<typename T, typename N, typename U>
struct make_non_type_parameter_template<type_fun, T, N, U> {
    using type = non_type_parameter_template<T, N::value, U>;
};

int main() {
    {
        // construct, assign, exception safety
        tagged_union<int, double, hoge, hoge> a(_1, 10.0);
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
        tagged_union<hoge, hoge> a(_0, hoge{42});
        tagged_union<hoge, hoge> b(_1, hoge{24});
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
        tagged_union<int, hoge> a(_0, 42);
        tagged_union<int, hoge> b(_1, hoge{42});
        assert(a != b);
        a = { _1, hoge{42} };
        assert(a == b);
    }
    {
        // strong guarantee without nothrow constructible type
        tagged_union<hoge, hoge> a(_0, hoge{42});
        tagged_union<hoge, hoge> b(_1, hoge{42});
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
        tagged_union<int, double> a(_0, 42);
        tagged_union<int, double> b(_1, 10.0);
        assert(a < b);
    }
    {
        // move
        tagged_union<int, std::vector<int>> a(_0, 10);
        tagged_union<int, std::vector<int>> b(_1, {1, 2, 3});
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
            tagged_union<leaf, recursive<node>> l; // recursive<T> prevent to cyclic class definition.
            tagged_union<leaf, recursive<node>> r;
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
            [] (auto, tag_t<0>, leaf) {
                return 0;
            }, [] (auto f, tag_t<1>, node const & n) -> int {
                return n.x + n.l.when(f) + n.r.when(f);
            }));
        assert(n.x + n.l.when(g) + n.r.when(g) == 15);
    }
    {
        // direct recursive type
        struct leaf {};
        using node = tagged_union<leaf, std::tuple<int, _, _>>;
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
            [] (auto, tag_t<0>, leaf) {
                return 0;
            }, [] (auto f, tag_t<1>, std::tuple<int, node, node> const & tup) -> int {
                return std::get<0>(tup) + std::get<1>(tup).when(f) + std::get<2>(tup).when(f);
            }));
        assert(n.when(g) == 15);
    }
    {
        // recursion with nest
        using u1 = tagged_union<int, _>;
        using u2 = tagged_union<_, u1, char>;
        static_assert(std::is_same<decltype(std::declval<u1&>().get(_1)), u1&>::value, "failed at recursion type 1");
        static_assert(std::is_same<decltype(std::declval<u2&>().get(_0)), u2&>::value, "failed at recursion type 2");
        u2 x = u2(_1, u1(_1, u1(_0, 42)));
        assert(x.get(_1).get(_1).get(_0) == 42);
    }
    {
        // fallback type in recursion type
        using u = tagged_union<std::tuple<_>, char>;
        u x(_1, 'a');
        assert(x == x);
    }
    {
        // recursion with nested and outer placeholder
        using u = tagged_union<tagged_union<_, _r<1>, char>, int>;
        static_assert(std::is_same<decltype(std::declval<u&>().get(_0)), tagged_union<_, u, char>&>::value, "failed at recursion with nested and outer placeholder 1");
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
        using ilist = tagged_union<unit, std::tuple<int, _>>;
        using clist = tagged_union<unit, std::tuple<char, _>>;

        ilist xs(_1, std::make_tuple(42, ilist(_0)));
        clist ys(_1, std::make_tuple(42, clist(_0)));
        assert(xs == ys);
    }
    {
        // recursion with non-type parameter using traits
        // (traits of std::array is already defined in this implementation)
        using ternary_tree = tagged_union<int, std::array<_, 3>>;
        auto n = ternary_tree(_1, {{{_0, 1}, {_0, 2}, {_0, 3}}});
        n.when(::tie(
            [] (tag_t<0>, int) {
                assert(false);
            },
            [] (tag_t<1>, std::array<ternary_tree, 3> const & ar) {
                assert(ar[0].get(_0) == 1 && ar[1].get(_0) == 2 && ar[2].get(_0) == 3);
            }));
    }
    {
        // recursion with non-type parameter (ad-hoc way)
        using type = make_non_type_parameter_template<type_fun, _, std::integral_constant<std::size_t, 2>, int>;
        using u = tagged_union<recursive<type>, int>;
        u x(_1, 42);
        using expected = non_type_parameter_template<u, 2, int>;
        static_assert(std::is_same<decltype(std::declval<u>().get(_0)), expected &&>::value, "failed at recursion with non-type parameter (ad-hoc way)");
    }
}
