
#include <cassert>
#include <vector>
#include "disjoint_union.hpp"

using desalt::_0;
using desalt::_1;
using desalt::_2;
using desalt::_3;
using desalt::disjoint_union;

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

int main() {
    {
        disjoint_union<int, double, hoge, hoge> a(_1, 10.0);
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
        disjoint_union<hoge, hoge> a(_0, hoge{42});
        disjoint_union<hoge, hoge> b(_1, hoge{24});
        b.get(_1).b = true;
        try {
            a = b;
            assert(false);
        } catch (int) {
            assert(a.get(_0).x == 42);
        }
    }
    {
        disjoint_union<int, hoge> a(_0, 42);
        disjoint_union<int, hoge> b(_1, hoge{42});
        assert(a != b);
        a = { _1, hoge{42} };
        assert(a == b);
    }
    {
        disjoint_union<hoge, hoge> a(_0, hoge{42});
        disjoint_union<hoge, hoge> b(_1, hoge{42});
        b.get(_1).b = true;
        try {
            a = b;
            assert(false);
        } catch (int) {
            assert(a.get(_0) == b.get(_1));
        }
    }
    {
        disjoint_union<int, double> a(_0, 42);
        disjoint_union<int, double> b(_1, 10.0);
        assert(a < b);
    }
    {
        disjoint_union<int, std::vector<int>> a(_0, 10);
        disjoint_union<int, std::vector<int>> b(_1, {1, 2, 3});
        a = std::move(b);
        auto & v1 = a.get(_1);
        auto v2 = {1, 2, 3};
        assert(std::equal(v1.begin(), v1.end(), v2.begin(), v2.end()));
        auto & v3 = b.get(_1);
        assert(v3.empty());
    }
}
