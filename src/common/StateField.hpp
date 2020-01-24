/**
 * @author Tanishq Aggarwal (ta335@cornell.edu)
 * @date 2019-05-30
 */

#ifndef STATE_FIELD_HPP_
#define STATE_FIELD_HPP_

#include "StateFieldBase.hpp"

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
     * @param ground_readable Specifies if field can be downlinked to ground.
     * @param ground_writable Specifies if field can be uplinked from ground.
     */
    StateField(const std::string &name, const bool ground_readable, const bool ground_writable)
        : StateFieldBase(),
          _name(name),
          _val(),
          _ground_readable(ground_readable),
          _ground_writable(ground_writable) {}

    const std::string &name() const override { return _name; }

    /**
     * @brief Returns a copy of field data.
     *
     * @return T Copy of of field value. If the field is not initialized, this
     * function fails silently (returns nonsense.)
     */
    T get() const { return _val; }

    /**
     * @brief Sets value of field data to provided value.
     *
     * @param t
     */
    void set(const T &t) { _val = t; }

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
