#pragma once

#include <functional>
#include <memory>
#include <string>
#include <variant>

#include "typedefs.hpp"
#include "util/Buffer.hpp"

namespace dynamic {
    class Map;
    class List;

    using ByteBuffer = util::Buffer<ubyte>;
    using Map_sptr = std::shared_ptr<Map>;
    using List_sptr = std::shared_ptr<List>;
    using ByteBuffer_sptr = std::shared_ptr<ByteBuffer>;

    struct none {};

    inline constexpr none NONE = {};

    using Value = std::variant<
        none,
        Map_sptr,
        List_sptr,
        ByteBuffer_sptr,
        std::string,
        number_t,
        bool,
        integer_t>;

    using to_string_func = std::function<std::string(const Value&)>;
}
