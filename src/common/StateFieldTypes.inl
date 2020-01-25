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
    InternalStateField(const std::string &name) : 
        StateField<T>(name, false, false) {}

    virtual ~InternalStateField() {}
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
    virtual size_t bitsize() const = 0;
    virtual const bit_array& get_bit_array() const = 0;
    virtual void set_bit_array(const bit_array& arr) = 0;
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
    Serializer<T> _serializer;

   public:
    SerializableStateField(const std::string &name, const bool ground_writable,
                           const Serializer<T> &s)
        : StateField<T>(name, true, ground_writable),
          _serializer(s) {}

    /**
     * @brief Get the minimum and maximum bounds on the serializer.
     */
    T get_serializer_min() const { return _serializer.min(); };
    T get_serializer_max() const { return _serializer.max(); };

    size_t bitsize() const override { return _serializer.bitsize(); }
    const bit_array& get_bit_array() const override { return _serializer.get_bit_array(); }
    void set_bit_array(const bit_array& arr) override { _serializer.set_bit_array(arr); }

    /**
     * @brief Serialize field data and store the serialized result into the internal
     * bitset.
     */
    void serialize() override { _serializer.serialize(this->_val); }

    /**
     * @brief Deserialize field data from the internally contained bitset and store
     * into the state field value.
     */
    void deserialize() override { _serializer.deserialize(&(this->_val)); }

    /**
     * @brief Deserialize field data from the provided character array and store
     * into the state field value.
     *
     * @param val Provided character array.
     */
    bool deserialize(const char *val) override { return _serializer.deserialize(val, &(this->_val)); }

    /**
     * @brief Write human-readable value of state field to a supplied string.
     *
     * @return C-style string containing printed version of state field.
     */
    const char *print() const override { return _serializer.print(this->_val); }

    virtual ~SerializableStateField() {}
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

    /**
     * @brief Dummy function used by RTTI in telemetry information generation.
     */
    virtual bool is_readable_field() const { return true; }
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

    /**
     * @brief Dummy function used by RTTI in telemetry information generation.
     */
    virtual bool is_writable_field() const { return true; }
};
