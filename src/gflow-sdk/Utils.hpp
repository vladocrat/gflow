#pragma once

#include <ranges>

namespace gflow
{

template<typename Container>
struct Closure {
    template<std::ranges::range Range>
    friend Container operator|(Range&& r, [[maybe_unused]] Closure /*self*/) {
        return Container(std::ranges::begin(r), std::ranges::end(r));
    }
};

template<typename Container>
auto to() -> Closure<Container> {
    return {};
}

} // namespae gflow
