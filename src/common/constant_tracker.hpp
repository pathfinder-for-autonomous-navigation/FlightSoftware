#ifndef CONSTANT_TRACKER_HPP_
#define CONSTANT_TRACKER_HPP_

// Macro that can be grepped for by a Python script to track the set of constants in
// Flight Software
#define TRACKED_CONSTANT(type, name, val) type name = val;

// Convenience macros for static, static constexpr, and constexpr values.
#define TRACKED_CONSTANT_S(type, name, val) TRACKED_CONSTANT(static type, name, val)
#define TRACKED_CONSTANT_C(type, name, val) TRACKED_CONSTANT(constexpr type, name, val)
#define TRACKED_CONSTANT_SC(type, name, val) TRACKED_CONSTANT(static constexpr type, name, val)

#endif
