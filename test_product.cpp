
#include <tuple>
#include <cassert>
#include <vector>
#include <type_traits>
#include <functional>
#include <desalt/datatypes.hpp>

using desalt::datatypes::_0;
using desalt::datatypes::_1;
using desalt::datatypes::_2;
using desalt::datatypes::_3;
using desalt::datatypes::product;
using desalt::datatypes::sum;
using desalt::datatypes::rec_guard;
using desalt::datatypes::tag;
using desalt::datatypes::fix;
using desalt::datatypes::_;
using desalt::datatypes::rec;

int main() {
    {
        // basic
        auto v1 = product<int, int const>{1, 2};
        auto const v2 = v1;
        static_assert(std::is_same<decltype(v1.get(_0)), int &>{}, "failed to basic 1");
        static_assert(std::is_same<decltype(v1.get(_1)), int const &>{}, "failed to basic 2");
        static_assert(std::is_same<decltype(v2.get(_0)), int const &>{}, "failed to basic 3");
        static_assert(std::is_same<decltype(v2.get(_1)), int const &>{}, "failed to basic 4");
        assert(v1.get(_0) == 1);
        assert(v1.get(_1) == 2);
    }
    {
        struct u {};
        using state = sum<u, int>;
        using stream = product<int, std::function<_()>>;
        auto f = fix([] (auto f, auto st) -> std::function<stream()> {
            return [f, st] {
                return st.when([&] (tag<0>, u) {
                    return stream{0, f(state(_1, 0))};
                }, [&] (tag<1>, int i) {
                    return stream{i + 1, f(state(_1, i + 1))};
                });
            };
        });
        auto s = f(state(_0))();
        assert(s.get(_0) == 0);
        s = s.get(_1)();
        assert(s.get(_0) == 1);
    }
}
