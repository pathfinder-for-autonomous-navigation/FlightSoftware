#include "StateField.hpp"
#include "Serializer.hpp"

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

   public:
    virtual void serialize() = 0;
    virtual void deserialize() = 0;
    virtual char* print() const = 0;
};

/**
 * @brief A state field that is serializable, i.e. capable of being
 * converted into a compressed format that enables transfer over radio.
 *
 * @tparam T Type of state field.
 */
template <typename T>
class SerializableStateField : public StateField<T>, public SerializableStateFieldBase {
   protected:
    std::shared_ptr<Serializer<T>> _serializer;

   public:
    SerializableStateField(const std::string &name, const bool ground_writable,
                           const std::shared_ptr<Serializer<T>> &s)
        : StateField<T>(name, false, ground_writable), SerializableStateFieldBase(name, ground_writable), _serializer(s) {}

    /**
     * @brief Get the stored bit array containing the serialized value.
     *
     * @return const bit_array&
     */
    const bit_array &get_bit_array() const { return _serializer->get_bit_array(); }

    /**
     * @brief Return size of bit array held by this serializer.
     *
     * @return size_t
     */
    size_t bitsize() const { return _serializer->bitsize(); }

    /**
     * @brief Get string length that is necessary to print the value
     * of the stored contents.
     *
     * @return size_t
     */
    size_t strlen() const { return _serializer->strlen(); }

    /**
     * @brief Set the internally stored serialized value. Do nothing if the source bit arary does
     * not have the same size as the internally stored bit array.
     */
    void set_bit_array(const bit_array &src) { _serializer->set_bit_array(src); }

    /**
     * @brief Serialize field data into the provided bitset.
     *
     * @param dest
     * @return true  If serialization was possible within the given bitset.
     * @return false If the given bitset is too small, serialization will not be
     * possible. Also returns false if the serializer or the state field was not
     * initialized.
     */
    void serialize() { (this->_serializer)->serialize(this->_val); }

    /**
     * @brief Deserialize field data from the provided bitset.
     *
     * @param src
     * @return true  If serialization was possible within the given bitset.
     * @return false If the given bitset is too small, serialization will not be
     * possible. Also returns false if the serializer or the state field was not
     * initialized.
     */
    void deserialize() {
        std::shared_ptr<T> val_ptr(&(this->_val));
        (this->_serializer)->deserialize(val_ptr);
    }

    /**
     * @brief Write human-readable value of state field to a supplied string.
     *
     * @return True if print succeeded, false if field is uninitialized.
     */
    char* print() const { return (this->_serializer)->print(this->_val); }
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
class ReadableStateField : public SerializableStateField<T>, public ReadableStateFieldBase {
   public:
    ReadableStateField(const std::string &name, const std::shared_ptr<Serializer<T>> &s) : SerializableStateField<T>(name, false, s), ReadableStateFieldBase(name) {}
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
class WritableStateField : public SerializableStateField<T>, WritableStateFieldBase {
   public:
    WritableStateField(const std::string &name, const std::shared_ptr<Serializer<T>> &s) : SerializableStateField<T>(name, true, s), WritableStateFieldBase(name) {}
};
