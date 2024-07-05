#ifndef DATA_DYNAMIC_FWD_HPP_
#define DATA_DYNAMIC_FWD_HPP_

#include "../typedefs.hpp"

#include <memory>
#include <variant>

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
        integer_t
    >;
}

#endif // DATA_DYNAMIC_FWD_HPP_
