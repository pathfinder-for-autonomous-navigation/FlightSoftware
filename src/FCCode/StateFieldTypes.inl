#include "StateField.hpp"

/**
 * @brief Empty base class for internal state fields.
 */
class InternalStateFieldBase {
   protected:
    /**
     * @brief Constructor. Should not be used.
     */
    InternalStateFieldBase() {}
};

/**
 * @brief A state field that is not accessible from ground; it is
 * purely used for internal state management. For example, this could be
 * some kind of matrix value within a controller which we don't care about.
 *
 * @tparam T Type of state field.
 */
template <typename T>
class InternalStateField : public StateField<T>, public InternalStateFieldBase {
   public:
    InternalStateField(const std::string &name) : StateField<T>(name), InternalStateFieldBase() {}

    /**
     * @brief Construct a new Internal State Field object (not ground readable or
     * writable.)
     */
    bool init(typename StateField<T>::fetch_f fetcher = StateField<T>::null_fetcher,
              typename StateField<T>::sanity_check_f checker = StateField<T>::null_sanity_check) {
        return StateField<T>::init(false, false, fetcher, checker);
    }
};

/**
 * @brief Empty base class for serializable state fields.
 */
class SerializableStateFieldBase {
   protected:
    /**
     * @brief Constructor. Should not be used.
     */
    SerializableStateFieldBase() {}
};

/**
 * @brief A state field that is serializable, i.e. capable of being
 * converted into a compressed format that enables transfer over radio.
 *
 * @tparam T Type of state field.
 * @tparam U An additional type that is used to serialize the field. For example,
 *           if T is a float vector, U might be a float, since the vector magnitude
 *           is stored separately from the vector direction when the vector is
 *           serialized.
 * @tparam compressed_size Size, in bits, of field when its value is compressed.
 */
template <typename T, typename U, size_t csz>
class SerializableStateField : public StateField<T>, public SerializableStateFieldBase {
   protected:
    std::shared_ptr<Serializer<T, U, csz>> _serializer;

   public:
    SerializableStateField(const std::string &name)
        : StateField<T>(name), SerializableStateFieldBase(), _serializer(nullptr) {}

    /**
     * @brief Initialize a new Serializable State Field object (definitely ground
     * readable, but may or may not be ground writable.)
     *
     * @param s The serializer to use for serializing the field.
     */
    bool init(bool gw, const std::shared_ptr<Serializer<T, U, csz>> &s,
              typename StateField<T>::fetch_f fetcher = StateField<T>::null_fetcher,
              typename StateField<T>::sanity_check_f checker = StateField<T>::null_sanity_check) {
        if (!s->is_initialized()) return false;
        _serializer = s;
        return StateField<T>::init(true, gw, fetcher, checker);
    }

    /**
     * @brief Serialize field data into the provided bitset.
     *
     * @param dest
     * @return true  If serialization was possible within the given bitset.
     * @return false If the given bitset is too small, serialization will not be
     * possible. Also returns false if the serializer or the state field was not
     * initialized.
     */
    bool serialize(std::bitset<csz> *dest) {
        return this->_is_initialized && (this->_serializer).serialize(this->_val, dest);
    }

    /**
     * @brief Deserialize field data from the provided bitset.
     *
     * @param src
     * @return true  If serialization was possible within the given bitset.
     * @return false If the given bitset is too small, serialization will not be
     * possible. Also returns false if the serializer or the state field was not
     * initialized.
     */
    bool deserialize(const std::bitset<csz> &src) {
        return this->_is_initialized && (this->_serializer).deserialize(src, &(this->_val));
    }

    /**
     * @brief Write human-readable value of state field to a supplied string.
     *
     * @param dest
     * @return True if print succeeded, false if field is uninitialized.
     */
    bool print(std::string *dest) {
        return this->_is_initialized && (this->_serializer).print(this->_val, dest);
    }
};

/**
 * @brief Empty base class for ground-readable state fields.
 */
class ReadableStateFieldBase {
   protected:
    /**
     * @brief Constructor. Should not be used.
     */
    ReadableStateFieldBase() {}
};

/**
 * @brief A state field that is readable only, i.e. whose value cannot be
 * modified via uplink.
 *
 * @tparam T Type of state field.
 * @tparam compressed_size Size, in bits, of field when its value is compressed.
 */
template <typename T, typename U, unsigned int csz>
class ReadableStateField : public SerializableStateField<T, U, csz>,
                           public ReadableStateFieldBase {
   public:
    ReadableStateField(const std::string &name)
        : SerializableStateField<T, U, csz>(name), ReadableStateFieldBase() {}

    /**
     * @brief Initialize a new Readable State Field object (readable from ground
     * but not writable.)
     */
    bool init(const std::shared_ptr<Serializer<T, U, csz>> &s,
              typename StateField<T>::fetch_f fetcher = StateField<T>::null_fetcher,
              typename StateField<T>::sanity_check_f checker = StateField<T>::null_sanity_check) {
        return SerializableStateField<T, U, csz>::init(false, s, fetcher, checker);
    }
};

/**
 * @brief Empty base class for ground-writable state fields.
 */
class WritableStateFieldBase {
   protected:
    /**
     * @brief Constructor. Should not be used.
     */
    WritableStateFieldBase() {}
};

/**
 * @brief A state field that is writable, i.e. whose value can be modified via
 * uplink.
 *
 * @tparam T Type of state field.
 * @tparam compressed_size Size, in bits, of field when its value is compressed.
 */
template <typename T, typename U, unsigned int csz>
class WritableStateField : public SerializableStateField<T, U, csz>,
                           public WritableStateFieldBase {
   public:
    WritableStateField(const std::string &name)
        : SerializableStateField<T, U, csz>(name), WritableStateFieldBase() {}

    /**
     * @brief Initialize a new Writable State Field object (readable and writable
     * from ground.)
     */
    bool init(const std::shared_ptr<Serializer<T, U, csz>> &s,
              typename StateField<T>::fetch_f fetcher = StateField<T>::null_fetcher,
              typename StateField<T>::sanity_check_f checker = StateField<T>::null_sanity_check) {
        return SerializableStateField<T, U, csz>::init(true, s, fetcher, checker);
    }
};
