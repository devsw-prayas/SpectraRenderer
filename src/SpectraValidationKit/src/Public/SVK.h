#pragma once

#include <Stratum.h>

#if defined(SVK_SHARED)
    #if STRATUM_COMPILER_MSVC
        #if defined(SVK_BUILDING_RUNTIME)
            #define SVK_API __declspec(dllexport)
        #else
            #define SVK_API __declspec(dllimport)
        #endif
    #elif STRATUM_COMPILER_CLANG || STRATUM_COMPILER_GCC
        #define SVK_API __attribute__((visibility("default")))
    #else
        #define SVK_API
    #endif
#else
    #define SVK_API 
#endif

namespace Spectra::Validation {
    static constexpr const char* SVK_COMPONENT = "ValidationKit";
}
