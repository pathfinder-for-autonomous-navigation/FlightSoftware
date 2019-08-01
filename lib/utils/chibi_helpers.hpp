/**
 * @file chibi_helpers.hpp
 * @author Tanishq Aggarwal (ta335@cornell.edu)
 * @brief File containing macros and functions for extending ChibiOS to custom
 * use cases.
 * @date 2019-05-27
 */

#ifndef CHIBI_HELPERS_HPP_
#define CHIBI_HELPERS_HPP_

#include <Arduino.h>
#include <ChRt.h>
#include <new>

// Helper function for ChibiOS-specific placement-new
inline void *operator new(size_t size, memory_heap_t &heap) { return chHeapAlloc(&heap, size); }
// Helper function for ChibiOS-specific delete
inline void operator delete(void *ptr) { chHeapFree(ptr); }

#endif