/**
 * @author Tanishq Aggarwal (ta335@cornell.edu)
 * @date 2019-05-30
 */

#ifndef STATE_FIELD_HPP_
#define STATE_FIELD_HPP_

#include <memory>

#include <ChRt.h>
#include <rwmutex.hpp>
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
class StateField : public StateFieldBase, public Debuggable, public InitializationRequired {
   public:
    /**
     * @brief Type definition for a pointer to a function that fetches
     * the value of the state field.
     *
     * This is useful if the value of the state field is directly
     * accessed from some peripheral or is computed by combining the values
     * of several state fields.
     */
    typedef T (*fetch_f)();

    /**
     * @brief A default fetch utility, which just returns the current
     * value of the state.
     *
     * @return T Current value of the state.
     */
    static T null_fetcher() {
        static T val;
        return val;
    }

    /**
     * @brief Type definition for a pointer to function that
     * checks sanity of state field.
     */
    typedef bool (*sanity_check_f)(const T &val);

    /**
     * @brief Default sanity checker. Always returns true.
     */
    static bool null_sanity_check(const T &val) { return true; }

   protected:
    bool _ground_readable;
    bool _ground_writable;
    T _val;
    fetch_f _fetcher;
    sanity_check_f _checker;

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
          _val(),
          _fetcher(nullptr),
          _checker(nullptr) {}

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
    virtual bool init(bool gr, bool gw, fetch_f fetcher = null_fetcher,
                      sanity_check_f checker = null_sanity_check) {
        _ground_readable = gr;
        _ground_writable = gw;
        _fetcher = fetcher;
        _checker = checker;
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
        if (!this->is_initialized()) return nullptr;
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
        if (!is_initialized()) return _val;
        if (getter->can_read(std::shared_ptr<StateFieldBase>(this)))
            printf(debug_severity::ALERT,
                   "ControlTaskBase %s illegally tried to read state field %s.",
                   getter->name().c_str(), this->name().c_str());
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
        if (!is_initialized()) return false;
        if (!setter->can_write(std::shared_ptr<StateFieldBase>(this)))
            printf(debug_severity::ALERT, "Task %s illegally tried to read state field %s.",
                   setter->name().c_str(), this->name().c_str());
        _val = t;
        return true;
    }

    /**
     * @brief Fetch value by calling the fetcher function that may be provided in
     * the constructor. If no fetcher was provided, the null_fetcher is called
     * (since that is the default value of _fetcher).
     *
     * The fetch function is synchronized using the fetch_lock lock that was
     * supplied during state field construction. This ensures that access to
     * shared resources (e.g. a device peripheral) that are used by the fetch
     * function is atomic.
     *
     * Note that the calling thread must have write permissions for the state
     * field because this function internally calls set(). No error will be
     * produced, and the fetch will still happen, but a debug message will be
     * written to the console indicating which thread was the offender.
     *
     * @return True if succeeded, false if field is not initialized.
     */
    template <typename U>
    bool fetch(const std::shared_ptr<ControlTask<U>> &setter) {
        return set(setter, _fetcher());
    }

    /**
     * @brief Checks the sanity of the internally contained value. Note that the
     * calling thread must have read permissions for the state field because this
     * function internally calls get(). No error will be produced, and the sanity
     * will still be evaluated, but a debug message will be written to the console
     * indicating which thread was the offender.
     *
     * @return True if the field value is valid, false if the field value is
     * invalid or is uninitialized.
     */
    template <typename U>
    bool sanity_check(const std::shared_ptr<ControlTask<U>> &getter) const {
        return _checker(get(getter));
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