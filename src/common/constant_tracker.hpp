#ifndef CONSTANT_TRACKER_HPP_
#define CONSTANT_TRACKER_HPP_

/**
 * @brief Macro that can be grepped for by a Python script to track the set of
 * constants in Flight Software.
 * 
 * The macro requires a semicolon after its invocation.
 */
#define TRACKED_CONSTANT(type, name, val) type name = val; static_assert(true, "")

// Convenience macros for static, static constexpr, and constexpr values.
#define TRACKED_CONSTANT_S(type, name, val) static type name = val; static_assert(true, "")
#define TRACKED_CONSTANT_C(type, name, val) constexpr type name = val; static_assert(true, "")
#define TRACKED_CONSTANT_SC(type, name, val) static constexpr type name = val; static_assert(true, "")

#endif
