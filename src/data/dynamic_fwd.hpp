#pragma once

#include <functional>
#include <memory>
#include <string>
#include <variant>

#include "typedefs.hpp"

namespace dynamic {
    class Map;
    class List;

    using Map_sptr = std::shared_ptr<Map>;
    using List_sptr = std::shared_ptr<List>;

    struct none {};

    inline constexpr none NONE = {};

    using Value = std::variant<
        none,
        Map_sptr,
        List_sptr,
        std::string,
        number_t,
        bool,
        integer_t>;

    using to_string_func = std::function<std::string(const Value&)>;
}
