#ifndef LOGIC_SCRIPTING_SCRIPTING_FUNCTIONAL_HPP_
#define LOGIC_SCRIPTING_SCRIPTING_FUNCTIONAL_HPP_

#include <glm/glm.hpp>
#include <string>
#include "../../typedefs.hpp"
#include "../../delegates.hpp"

namespace scripting {
    runnable create_runnable(
        const scriptenv& env,
        const std::string& src,
        const std::string& file="<string>"
    );
    
    wstringconsumer create_wstring_consumer(
        const scriptenv& env,
        const std::string& src,
        const std::string& file="<string>"
    );

    wstringsupplier create_wstring_supplier(
        const scriptenv& env,
        const std::string& src,
        const std::string& file="<string>"
    );

    wstringchecker create_wstring_validator(
        const scriptenv& env,
        const std::string& src,
        const std::string& file="<string>"
    );

    boolconsumer create_bool_consumer(
        const scriptenv& env,
        const std::string& src,
        const std::string& file="<string>"
    );

    boolsupplier create_bool_supplier(
        const scriptenv& env,
        const std::string& src,
        const std::string& file="<string>"
    );

    doubleconsumer create_number_consumer(
        const scriptenv& env,
        const std::string& src,
        const std::string& file="<string>"
    );

    doublesupplier create_number_supplier(
        const scriptenv& env,
        const std::string& src,
        const std::string& file="<string>"
    );

    int_array_consumer create_int_array_consumer(
        const scriptenv& env,
        const std::string& src, 
        const std::string& file="<string>"
    );
    
    vec2supplier create_vec2_supplier(
        const scriptenv& env,
        const std::string& src, 
        const std::string& file="<string>"
    );
}

#endif // LOGIC_SCRIPTING_SCRIPTING_FUNCTIONAL_HPP_
