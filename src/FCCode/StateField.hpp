/**
 * @author Tanishq Aggarwal (ta335@cornell.edu)
 * @date 2019-05-30
 */

#ifndef STATE_FIELD_HPP_
#define STATE_FIELD_HPP_

#include "Serializer.hpp"
#include "StateFieldBase.hpp"

#include <vector>
#include <memory>

/**
 * @brief A lightweight container around state fields that allows thread-safe
 * access and serialization utilities.
 *
 * @tparam T Type of the state field.
 * @tparam compressed_size Size of the state field when being sent in a downlink
 * or uplink packet.
 */
template <typename T>
class StateField : public virtual StateFieldBase {
   protected:
    const std::string _name;
    T _val;
    bool _ground_readable;
    bool _ground_writable;

   public:
    /**
     * @brief Construct a new StateField object
     *
     * @param name Name of state field. Useful for debugging.
     */
    StateField(const std::string &name, const bool ground_readable, const bool ground_writable)
        : StateFieldBase(), _name(name), _val(), _ground_readable(ground_readable), _ground_writable(ground_writable) {}

    const std::string& name() const override { return _name; }

    /**
     * @brief Returns a generic pointer to this state field. Useful for creating
     * collections of StateFields. If the field is not initialized, returns a null
     * pointer.
     *
     * @return StateFieldBase* Generic pointer to this state field.
     */
    std::shared_ptr<StateFieldBase> ptr() { return this; }

    /**
     * @brief Returns a copy of field data.
     *
     * @return T Copy of of field value. If the field is not initialized, this
     * function fails silently (returns nonsense.)
     */
    template <typename U>
    T get() const {
        return _val;
    }

    /**
     * @brief Sets value of field data to provided value.
     *
     * @param t
     */
    template <typename U>
    void set(const T &t) {
        _val = t;
    }

    /**
     * @brief Accessors.
     *
     * @{
     */
    bool is_readable() const override { return _ground_readable; }
    bool is_writable() const override { return _ground_writable; }
    /**
     * @}
     */
};

#include "StateFieldTypes.inl"

#endif