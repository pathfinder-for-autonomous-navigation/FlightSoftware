#include "Serializer.hpp"
#include "StateField.hpp"

/**
 * @brief Empty base class for internal state fields.
 */
class InternalStateFieldBase : public StateFieldBase {
   protected:
    /**
     * @brief Constructor. Should not be used.
     */
    InternalStateFieldBase(const std::string &name) : StateFieldBase(name, false, false) {}
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
    InternalStateField(const std::string &name)
        : StateField<T>(name, false, false), InternalStateFieldBase() {}
};

/**
 * @brief Empty base class for serializable state fields.
 */
class SerializableStateFieldBase : public StateFieldBase {
   protected:
    /**
     * @brief Constructor. Should not be used.
     */
    SerializableStateFieldBase(const std::string &name, const bool ground_writable) : StateFieldBase(name, true, ground_writable) {}
};

/**
 * @brief A state field that is serializable, i.e. capable of being
 * converted into a compressed format that enables transfer over radio.
 *
 * @tparam T Type of state field.
 */
template <typename T>
class SerializableStateField : public StateField<T> {
   protected:
    std::shared_ptr<Serializer<T>> _serializer;

   public:
    SerializableStateField(const std::string &name, const bool ground_writable,
                           const std::shared_ptr<Serializer<T>> &s)
        : StateField<T>(name, false, ground_writable), _serializer(s) {}

    /**
     * @brief Serialize field data into the provided bitset.
     *
     * @param dest
     * @return true  If serialization was possible within the given bitset.
     * @return false If the given bitset is too small, serialization will not be
     * possible. Also returns false if the serializer or the state field was not
     * initialized.
     */
    bool serialize(bit_array *dest) {
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
    bool deserialize(const bit_array &src) {
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
class ReadableStateFieldBase : public StateFieldBase {
   protected:
    /**
     * @brief Constructor. Should not be used.
     */
    ReadableStateFieldBase(const std::string &name) : StateFieldBase(name, true, false) {}
};

/**
 * @brief A state field that is readable only, i.e. whose value cannot be
 * modified via uplink.
 *
 * @tparam T Type of state field.
 * @tparam compressed_size Size, in bits, of field when its value is compressed.
 */
template <typename T>
class ReadableStateField : public SerializableStateField<T> {
   public:
    ReadableStateField(const std::string &name, const std::shared_ptr<Serializer<T>> &s) : SerializableStateField<T>(name, false, s) {}
};

/**
 * @brief Empty base class for ground-writable state fields.
 */
class WritableStateFieldBase : public StateFieldBase {
   protected:
    /**
     * @brief Constructor. Should not be used.
     */
    WritableStateFieldBase(const std::string &name) : StateFieldBase(name, true, true) {}
};

/**
 * @brief A state field that is writable, i.e. whose value can be modified via
 * uplink.
 *
 * @tparam T Type of state field.
 * @tparam compressed_size Size, in bits, of field when its value is compressed.
 */
template <typename T>
class WritableStateField : public SerializableStateField<T> {
   public:
    WritableStateField(const std::string &name, const std::shared_ptr<Serializer<T>> &s) : SerializableStateField<T>(name, true, s) {}
};
