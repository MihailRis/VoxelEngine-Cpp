#include "alutil.hpp"

#include <cstring>
#include <fstream>
#include <memory>
#include <type_traits>

#include "debug/Logger.hpp"

#ifdef __APPLE__
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif

static debug::Logger logger("open-al");

bool AL::check_errors(
    const std::string& filename, const std::uint_fast32_t line
) {
    ALenum error = alGetError();
    if (error != AL_NO_ERROR) {
        logger.error() << filename << ": " << line;
        switch (error) {
            case AL_INVALID_NAME:
                logger.error()
                    << "a bad name (ID) was passed to an OpenAL function";
                break;
            case AL_INVALID_ENUM:
                logger.error()
                    << "an invalid enum value was passed to an OpenAL function";
                break;
            case AL_INVALID_VALUE:
                logger.error()
                    << "an invalid value was passed to an OpenAL function";
                break;
            case AL_INVALID_OPERATION:
                logger.error() << "the requested operation is not valid";
                break;
            case AL_OUT_OF_MEMORY:
                logger.error() << "the requested operation resulted in OpenAL "
                                  "running out of memory";
                break;
            default:
                logger.error() << "UNKNOWN AL ERROR: " << error;
        }
        return false;
    }
    return true;
}
