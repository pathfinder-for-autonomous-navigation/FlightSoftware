/**
 * @file StateField.hpp
 * @author Tanishq Aggarwal (ta335@cornell.edu)
 * @date 2019-05-30
 */

#ifndef STATE_FIELD_HPP_
#define STATE_FIELD_HPP_

#include "StateFieldRegistry.hpp"
#include "ControlTask.hpp"
#include "Serializer.hpp"
#include <vector>
#include <ChRt.h>
#include <rwmutex.hpp>

/**
 * @brief A lightweight container around state fields that allows thread-safe access
 * and serialization utilities.
 * 
 * @tparam T Type of the state field.
 * @tparam compressed_size Size of the state field when being sent in a downlink or uplink packet.
 */
template<typename T>
class StateField : public DataField, Debuggable {
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
    T null_fetcher();

    /**
     * @brief Type definition for a pointer to function that 
     * checks sanity of state field.
     */
    typedef bool (StateField<T>::*sanity_check_f)(const T& val) const;
    /**
     * @brief Default sanity checker. Always returns true.
     */
    bool null_sanity_check(const T& val) const;

    /**
     * @brief Construct a new StateField object
     * 
     * @param name Name of state field. Useful for debugging.
     * @param l Lock that synchronizes access to state field.
     */
    StateField(const std::string& name, debug_console& dbg_console);

    /**
     * @brief Initialize a State Field object
     * 
     * @param gr If true, this field is sent via downlink to the ground.
     * @param gw If true, this field can be set via uplink from the ground.
     * @param reg The state field registry. This is needed so that the StateField
     * can check which threads are allowed to read/write its value.
     * @param dbg_console Reference to a debug console, which may be used to write error messages.
     * @param fetcher A function to fetch this value from some device. By default,
     * there is no fetcher.
     * @param f_l Lock that synchronizes access to resources that may be used within the 
     * fetch function (e.g. a device peripheral.) We require that the lock already be initialized,
     * or else the program will abort when tested.
     */
    virtual void init(bool gr,
                      bool gw,
                      StateFieldRegistry& reg,
                      typename StateField<T>::fetch_f fetcher = &StateField<T>::null_fetcher,
                      typename StateField<T>::sanity_check_f checker = &StateField<T>::null_sanity_check);

    std::string& name() const;

    /**
     * @brief Returns a generic pointer to this state field. Useful for creating
     * collections of StateFields.
     * 
     * @return DataField* Generic pointer to this state field.
     */
    DataField* ptr();

    /**
     * @brief Checks registry for read access.
     * 
     * @param r 
     * @return true If Control Task has read access to state field.
     * @return false If Control Task does not have read access to state field.
     */
    bool can_read(Task& r);

    /**
     * @brief Checks registry for write access.
     * 
     * @param r 
     * @return true If Control Task has write access to state field.
     * @return false If Control Task does not have write access to state field.
     */
    bool can_write(Task& r);

    /**
     * @brief Provides a thread with write permissions to this state field.
     * 
     * @param w Thread to add as writer.
     */
    void add_writer(Task& w);

    /**
     * @brief Provides a thread with read permissions to this state field.
     * 
     * @param r Thread to add as reader.
     */
    void add_reader(Task& r);

    /**
     * @brief Returns a copy of field data. The calling thread must have
     * read permissions on the state field. No error will be produced, and the get
     * request will still be fulfilled, but a debug message will be written to the 
     * console indicating which thread was the offender.
     * 
     * @return T Copy of of field value.
     */
    T get(Task* getter);

    /**
     * @brief Sets value of field data to provided value. The calling thread must have
     * write permissions on the state field. No error will be produced, and the set 
     * request will still be fulfilled, but a debug message will be written to the 
     * console indicating which thread was the offender.
     * 
     * @param t 
     */
    void set(Task* setter, const T& t);

    /**
     * @brief Fetch value by calling the fetcher function that may be provided in the 
     * constructor. If no fetcher was provided, the null_fetcher is called (since that 
     * is the default value of _fetcher).
     * 
     * The fetch function is synchronized using the fetch_lock lock that was supplied during
     * state field construction. This ensures that access to shared resources (e.g. a device peripheral)
     * that are used by the fetch function is atomic.
     * 
     * Note that the calling thread must have write permissions for the state field because
     * this function internally calls set(). No error will be produced, and the fetch 
     * will still happen, but a debug message will be written to the console indicating 
     * which thread was the offender.
     */
    void fetch(Task* setter);

    /**
     * @brief Checks the sanity of the internally contained value. Note that the
     * calling thread must have read permissions for the state field because this function
     * internally calls get(). No error will be produced, and the sanity will still be evaluated,
     * but a debug message will be written to the console indicating which thread was the offender.
     */
    bool sanity_check(Task* getter) const;

  protected:
    bool _ground_readable;
    bool _ground_writable;
    T _val;
    StateFieldRegistry& _registry;
    fetch_f _fetcher;
    sanity_check_f _checker;
};

/**
 * @brief A state field that is not accessible from ground; it is 
 * purely used for internal state management. For example, this could be
 * some kind of matrix value within a controller which we don't care about.
 *
 * @tparam T Type of state field.
 */
template<typename T>
class InternalStateField : public StateField<T> {
  public:
    using StateField<T>::StateField;
    /**
     * @brief Construct a new Internal State Field object
     *
     * @param name Name of state field. Useful for debugging.
     * @param l Lock that synchronizes access to state field.
     * @param reg The state field registry. This is needed so that the StateField
     * can check which threads are allowed to read/write its value.
     * @param dbg_console Reference to a debug console, which may be used to write error messages.
     * @param fetcher A function to fetch this value from some device. By default,
     * there is no fetcher.
     * @param f_l Lock that synchronizes access to resources that may be used within the 
     * fetch function (e.g. a device peripheral.)
     */
    void init(StateFieldRegistry& reg,
              typename StateField<T>::fetch_f fetcher = &StateField<T>::null_fetcher,
              typename StateField<T>::sanity_check_f checker = &StateField<T>::null_sanity_check);
};

/**
 * @brief A state field that is serializable, i.e. capable of being 
 * converted into a compressed format that enables transfer over radio.
 * 
 * @tparam T Type of state field.
 * @tparam compressed_size Size, in bits, of field when its value is compressed.
 */
template<typename T, typename U, unsigned int compressed_sz>
class SerializableStateField : public StateField<T> {
  protected:
    Serializer<T, U, compressed_sz>& _serializer;
  public:
    using StateField<T>::StateField;
    /**
     * @brief Initialize a new Serializable State Field object
     * 
     * @param gw If true, this field can be set via uplink from the ground.
     * @param reg The state field registry. This is needed so that the StateField
     * can check which threads are allowed to read/write its value.
     * @param s  Serializer object that handles serialization/deserialization of this state field.
     * @param dbg_console Reference to a debug console, which may be used to write error messages.
     * @param fetcher A function to fetch this value from some device. By default,
     * there is no fetcher.
     * @param f_l Lock that synchronizes access to resources that may be used within the 
     * fetch function (e.g. a device peripheral.)
     */
    void init(bool gw,
              StateFieldRegistry& reg,
              Serializer<T, U, compressed_sz>& s,
              typename StateField<T>::fetch_f fetcher = &StateField<T>::null_fetcher,
              typename StateField<T>::sanity_check_f checker = &StateField<T>::null_sanity_check);
    
    /**
     * @brief Serialize field data into the provided bitset.
     * 
     * @param dest 
     * @return true If serialization was possible within the given bitset.
     * @return false If the given bitset is too small, serialization will not be possible.
     */
    void serialize(std::bitset<compressed_sz>* dest);

    /**
     * @brief Deserialize field data from the provided bitset.
     * 
     * @param src 
     * @return true If serialization was possible within the given bitset.
     * @return false If the given bitset is too small, serialization will not be possible.
     */
    void deserialize(const std::bitset<compressed_sz>& src);

    /**
     * @brief Write human-readable value of state field to a supplied string.
     * 
     * @param dest 
     */
    void print(std::string* dest);
};

/**
 * @brief A state field that is readable only, i.e. whose value cannot be modified via uplink.
 * 
 * @tparam T Type of state field.
 * @tparam compressed_size Size, in bits, of field when its value is compressed.
 */
template <typename T, typename U, unsigned int compressed_sz>
class ReadableStateField : public SerializableStateField<T, U, compressed_sz>
{
public:
  using StateField<T>::StateField;
  /**
   * @brief Initialize a new Readable State Field object
   * 
   * @param reg The state field registry. This is needed so that the StateField
   * can check which threads are allowed to read/write its value.
   * @param s Serializer object that handles serialization/deserialization of this state field.
   * @param dbg_console Reference to a debug console, which may be used to write error messages.
   * @param fetcher A function to fetch this value from some device. By default,
   * there is no fetcher.
   * @param f_l Lock that synchronizes access to resources that may be used within the 
   * fetch function (e.g. a device peripheral.)
   */
  void init(StateFieldRegistry &reg,
            Serializer<T, U, compressed_sz> &s,
            typename StateField<T>::fetch_f fetcher = &StateField<T>::null_fetcher,
            typename StateField<T>::sanity_check_f checker = &StateField<T>::null_sanity_check);
};

/**
 * @brief A state field that is writable, i.e. whose value can be modified via uplink.
 * 
 * @tparam T Type of state field.
 * @tparam compressed_size Size, in bits, of field when its value is compressed.
 */
template<typename T, typename U, unsigned int compressed_sz>
class WritableStateField : public SerializableStateField<T, U, compressed_sz> {
  public:
    using StateField<T>::StateField;
    /**
     * @brief Initialize a new Writable State Field object
     * 
     * @param reg The state field registry. This is needed so that the StateField
     * can check which threads are allowed to read/write its value.
     * @param s Serializer object that handles serialization/deserialization of this state field.
     * @param fetcher A function to fetch this value from some device. By default,
     * there is no fetcher.
     * @param f_l Lock that synchronizes access to resources that may be used within the 
     * fetch function (e.g. a device peripheral.)
     */
    void init(StateFieldRegistry& reg,
              Serializer<T, U, compressed_sz>& s,
              typename StateField<T>::fetch_f fetcher = &StateField<T>::null_fetcher,
              typename StateField<T>::sanity_check_f checker = &StateField<T>::null_sanity_check);
};

#include "StateField.inl"

#endif