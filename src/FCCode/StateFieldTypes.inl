#include "Serializer.hpp"
#include "StateField.hpp"

/**
 * @brief Empty base class for internal state fields.
 */
class InternalStateFieldBase : virtual public StateFieldBase {};

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
    InternalStateField(const std::string &name) : StateField<T>(name, false, false) {}
};

/**
 * @brief Empty base class for serializable state fields.
 */
class SerializableStateFieldBase : virtual public StateFieldBase {
   public:
    virtual void serialize() = 0;
    virtual void deserialize() = 0;
    virtual bool deserialize(const char *val) = 0;
    virtual const char *print() const = 0;
};

/**
 * @brief A state field that is serializable, i.e. capable of being
 * converted into a compressed format that enables transfer over radio.
 *
 * @tparam T Type of state field.
 */
template <typename T>
class SerializableStateField : public StateField<T>, virtual public SerializableStateFieldBase {
   protected:
    std::shared_ptr<Serializer<T>> _serializer;

   public:
    SerializableStateField(const std::string &name, const bool ground_writable,
                           const Serializer<T> &s)
        : StateField<T>(name, false, ground_writable) {
        _serializer = std::make_shared<Serializer<T>>(s);
    }

    /**
     * @brief Get the stored bit array containing the serialized value.
     *
     * @return const bit_array&
     */
    const Serializer<T> &get_serializer() const { return *_serializer; }

    /**
     * @brief Serialize field data and store the serialized result into the internal
     * bitset.
     */
    void serialize() override { _serializer->serialize(this->_val); }

    /**
     * @brief Deserialize field data from the internally contained bitset and store
     * into the state field value.
     */
    void deserialize() override {
        std::shared_ptr<T> val_ptr(&(this->_val));
        _serializer->deserialize(val_ptr);
    }

    /**
     * @brief Deserialize field data from the provided character array and store
     * into the state field value.
     *
     * @param val Provided character array.
     */
    bool deserialize(const char *val) override {
        std::shared_ptr<T> val_ptr(&(this->_val));
        return _serializer->deserialize(val, val_ptr);
    }

    /**
     * @brief Write human-readable value of state field to a supplied string.
     *
     * @return C-style string containing printed version of state field.
     */
    const char *print() const override { return _serializer->print(this->_val); }
};

/**
 * @brief Empty base class for ground-readable state fields.
 */
class ReadableStateFieldBase : virtual public SerializableStateFieldBase {};

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
    ReadableStateField(const std::string &name, const Serializer<T> &s)
        : SerializableStateField<T>(name, false, s) {}
};

/**
 * @brief Empty base class for ground-writable state fields.
 */
class WritableStateFieldBase : virtual public ReadableStateFieldBase {};

/**
 * @brief A state field that is writable, i.e. whose value can be modified via
 * uplink.
 *
 * @tparam T Type of state field.
 * @tparam compressed_size Size, in bits, of field when its value is compressed.
 */
template <typename T>
class WritableStateField : public SerializableStateField<T>, public WritableStateFieldBase {
   public:
    WritableStateField(const std::string &name, const Serializer<T> &s)
        : SerializableStateField<T>(name, true, s) {}
};
