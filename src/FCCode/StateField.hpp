/**
 * @author Tanishq Aggarwal (ta335@cornell.edu)
 * @date 2019-05-30
 */

#ifndef STATE_FIELD_HPP_
#define STATE_FIELD_HPP_

#include <memory>

#include <vector>
#include "ControlTask.hpp"
#include "InitializationRequired.hpp"
#include "Serializer.hpp"
#include "StateFieldBase.hpp"
#include "StateFieldRegistry.hpp"

/**
 * @brief A lightweight container around state fields that allows thread-safe
 * access and serialization utilities.
 *
 * @tparam T Type of the state field.
 * @tparam compressed_size Size of the state field when being sent in a downlink
 * or uplink packet.
 */
template <typename T>
class StateField : public StateFieldBase, public Debuggable {
   protected:
    bool _ground_readable;
    bool _ground_writable;
    T _val;

   public:
    /**
     * @brief Construct a new StateField object
     *
     * @param name Name of state field. Useful for debugging.
     * @param l Lock that synchronizes access to state field.
     * @param reg The state field registry. This is needed so that the StateField
     * can check which threads are allowed to read/write its value.
     */
    StateField(const std::string &name)
        : StateFieldBase(name),
          Debuggable(),
          _ground_readable(false),
          _ground_writable(false),
          _val() {}

    /**
     * @brief Initialize a State Field object
     *
     * @param gr If true, this field is sent via downlink to the ground.
     * @param gw If true, this field can be set via uplink from the ground.
     * @param dbg_console Reference to a debug console, which may be used to write
     * error messages.
     * @param fetcher A function to fetch this value from some device. By default,
     * there is no fetcher.
     * @param f_l Lock that synchronizes access to resources that may be used
     * within the fetch function (e.g. a device peripheral.) We require that the
     * lock already be initialized, or else the program will abort when tested.
     *
     * @bool Returns true if succeed, false if state field registry is
     * uninitialized.
     */
    virtual bool init(bool gr, bool gw) {
        _ground_readable = gr;
        _ground_writable = gw;
        return Debuggable::init();
    }

    /**
     * @brief Returns a generic pointer to this state field. Useful for creating
     * collections of StateFields. If the field is not initialized, returns a null
     * pointer.
     *
     * @return StateFieldBase* Generic pointer to this state field.
     */
    std::shared_ptr<StateFieldBase> ptr() {
        return this;
    }

    /**
     * @brief Returns a copy of field data. The calling thread must have
     * read permissions on the state field. No error will be produced, and the get
     * request will still be fulfilled, but a debug message will be written to the
     * console indicating which thread was the offender.
     *
     * @return T Copy of of field value. If the field is not initialized, this
     * function fails silently (returns nonsense.)
     */
    template <typename U>
    T get(const std::shared_ptr<ControlTask<U>> &getter) {
        return _val;
    }

    /**
     * @brief Sets value of field data to provided value. The calling thread must
     * have write permissions on the state field. No error will be produced, and
     * the set request will still be fulfilled, but a debug message will be
     * written to the console indicating which thread was the offender.
     *
     * @param t
     * @return True if succeeded, false if field is not initialized.
     */
    template <typename U>
    bool set(const std::shared_ptr<ControlTask<U>> &setter, const T &t) {
        _val = t;
        return true;
    }

    /**
     * @brief Accessors.
     *
     * @{
     */
    bool is_readable() const { return _ground_readable; }
    bool is_writable() const { return _ground_writable; }
    /**
     * @}
     */
};

#include "StateFieldTypes.inl"

#endif