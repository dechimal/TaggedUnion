#if !defined DESALT_DATATYPES_PRODUCT_HPP_INCLUDED_
#define      DESALT_DATATYPES_PRODUCT_HPP_INCLUDED_

#include <tuple>
#include <utility>
#include <desalt/datatypes/recursion.hpp>

namespace desalt::datatypes {
namespace detail::product {
namespace here = product;

template<typename ...Ts>
class product : private std::tuple<rec::unwrap<rec::unfold<product<Ts...>, Ts>>...> {
    using base_type = std::tuple<rec::unwrap<rec::unfold<product, Ts>>...>;
public:
    product() = default;
    product(product const &) = default;
    product(product &&) = default;
    template<typename ...Us> product(Us && ...xs)
        : base_type(std::forward<Us>(xs)...)
    {}

    product & operator=(product const & other) {
        base().operator=(other);
        return *this;
    }
    product & operator=(product && other) {
        base().operator=(std::move(other));
        return *this;
    }

    template<typename Tag> decltype(auto) get(Tag)       &  { return std::get<Tag::value>(base()); }
    template<typename Tag> decltype(auto) get(Tag) const &  { return std::get<Tag::value>(base()); }
    template<typename Tag> decltype(auto) get(Tag)       && { return std::get<Tag::value>(base()); }
    template<typename Tag> decltype(auto) get(Tag) const && { return std::get<Tag::value>(base()); }
private:
    base_type       &  base()       &  { return *this; }
    base_type const &  base() const &  { return *this; }
    base_type       && base()       && { return *this; }
    base_type const && base() const && { return *this; }
};

} // namespace detail::product {

using detail::product::product;

} // namespace desalt::datatypes {

#endif
