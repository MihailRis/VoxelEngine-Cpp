#ifndef LOGIC_SCRIPTING_SCRIPTING_FUNCTIONAL_H_
#define LOGIC_SCRIPTING_SCRIPTING_FUNCTIONAL_H_

#include <glm/glm.hpp>
#include <string>
#include "../../delegates.h"

namespace scripting {
    runnable create_runnable(
        int env,
        const std::string& src,
        const std::string& file="<string>"
    );
    
    wstringconsumer create_wstring_consumer(
        int env,
        const std::string& src,
        const std::string& file="<string>"
    );

    wstringsupplier create_wstring_supplier(
        int env,
        const std::string& src,
        const std::string& file="<string>"
    );

    boolconsumer create_bool_consumer(
        int env,
        const std::string& src,
        const std::string& file="<string>"
    );

    boolsupplier create_bool_supplier(
        int env,
        const std::string& src,
        const std::string& file="<string>"
    );

    doubleconsumer create_number_consumer(
        int env,
        const std::string& src,
        const std::string& file="<string>"
    );

    doublesupplier create_number_supplier(
        int env,
        const std::string& src,
        const std::string& file="<string>"
    );

    int_array_consumer create_int_array_consumer(
        int env,
        const std::string& src, 
        const std::string& file="<string>"
    );
    
    vec2supplier create_vec2_supplier(
        int env,
        const std::string& src, 
        const std::string& file="<string>"
    );
}

#endif // LOGIC_SCRIPTING_SCRIPTING_FUNCTIONAL_H_
