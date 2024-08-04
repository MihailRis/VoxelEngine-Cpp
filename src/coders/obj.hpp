#ifndef CODERS_OBJ_HPP_
#define CODERS_OBJ_HPP_

#include <memory>
#include <string>

/// Wavefont OBJ files parser

namespace model {
    struct Model;
}

namespace obj {
    std::unique_ptr<model::Model> parse(
        const std::string_view file, const std::string_view src
    );
}

#endif  // CODERS_OBJ_HPP_
