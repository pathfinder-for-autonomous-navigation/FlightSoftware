#ifndef CASTS_HPP_
#define CASTS_HPP_

// Default definitions for casting utilites
#define STATIC_CAST(type, var) static_cast<type>(var)
#define DYNAMIC_CAST(type, var) STATIC_CAST(type, var)

// Ensure GNU or Clang
#if !defined(__GNUC__) && !defined(__clang__)
  #error "Please use a GNU or Clang compiler."
#endif

// Handle GNU compilers
#ifdef __GNUC__
  #ifdef __GXX_RTTI
    #undef DYNAMIC_CAST
    #define DYNAMIC_CAST(type, var) dynamic_cast<type>(var)
  #endif
#endif

// Handle Clang compilers
#ifdef __clang__
  #if __has_feature(cxx_rtti)
    #undef DYNAMIC_CAST
    #define DYNAMIC_CAST(type, var) dynamic_cast<type>(var)
  #endif
#endif

#endif
