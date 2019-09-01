#ifndef STATE_FIELD_BASE_HPP_
#define STATE_FIELD_BASE_HPP_

#include <Nameable.hpp>

/**
 * @brief Dummy class so that we can create pointers of type StateFieldBase that point to objects of
 * type StateField<T>. See "StateField.hpp"
 */
class StateFieldBase : public Nameable {
   protected:
    bool _ground_readable;
    bool _ground_writable;

    /**
     * @brief Constructor. Should not be used.
     */
    StateFieldBase(const std::string &name, const bool ground_readable, const bool ground_writable)
        : Nameable(name), _ground_readable(ground_readable), _ground_writable(ground_writable) {}
};

#endif