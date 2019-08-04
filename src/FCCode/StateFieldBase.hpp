#ifndef STATE_FIELD_BASE_HPP_
#define STATE_FIELD_BASE_HPP_

#include <Nameable.hpp>

/**
 * @brief Dummy class so that we can create pointers of type StateFieldBase that
 * point to objects of type StateField<T>. See "StateField.hpp"
 */
class StateFieldBase : public Nameable {
   protected:
    /**
     * @brief Constructor. Should not be used.
     */
    StateFieldBase(const std::string &name) : Nameable(name) {}
};

#endif