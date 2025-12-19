#pragma once

#ifdef __has_cpp_attribute
    #if __has_cpp_attribute(nodiscard)
        #define WORSE_NODISCARD [[nodiscard]]
    #endif
    #if __has_cpp_attribute(noreturn)
        #define WORSE_NORETURN [[noreturn]]
    #endif
    #if defined(_MSC_VER) && __has_cpp_attribute(msvc::no_unique_address)
        // See https://en.cppreference.com/w/cpp/language/attributes/no_unique_address Notes.
        #define WORSE_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
    #elif __has_cpp_attribute(no_unique_address)
        #define WORSE_NO_UNIQUE_ADDRESS [[no_unique_address]]
    #endif
#endif
#ifndef WORSE_NODISCARD
    #define WORSE_NODISCARD
#endif
#ifndef WORSE_NORETURN
    #define WORSE_NORETURN
#endif
#ifndef WORSE_NO_UNIQUE_ADDRESS
    #define WORSE_NO_UNIQUE_ADDRESS
#endif