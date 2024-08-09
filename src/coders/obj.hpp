#pragma once

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
