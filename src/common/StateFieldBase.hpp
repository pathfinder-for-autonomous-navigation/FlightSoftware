#ifndef STATE_FIELD_BASE_HPP_
#define STATE_FIELD_BASE_HPP_

#include "Nameable.hpp"

/**
 * @brief Dummy class so that we can create pointers of type StateFieldBase that point to objects of
 * type StateField<T>. See "StateField.hpp"
 */
class StateFieldBase : public Nameable {
   protected:
    virtual bool is_readable() const = 0;
    virtual bool is_writable() const = 0;
};

#endif