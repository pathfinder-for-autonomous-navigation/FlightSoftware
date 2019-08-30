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
    InternalStateFieldBase(const std::string &name) : StateFieldBase(name) {}
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
    bool init() {
        return StateField<T>::init(false, false);
    }
};

/**
 * @brief Empty base class for serializable state fields.
 */
class SerializableStateFieldBase : public StateFieldBase {
   protected:
    /**
     * @brief Constructor. Should not be used.
     */
    SerializableStateFieldBase(const std::string &name) : StateFieldBase(name) {}
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
class SerializableStateField : public StateField<T> {
   protected:
    std::shared_ptr<Serializer<T, U, csz>> _serializer;

   public:
    SerializableStateField(const std::string &name) : StateField<T>(name), _serializer(nullptr) {}

    /**
     * @brief Initialize a new Serializable State Field object (definitely ground
     * readable, but may or may not be ground writable.)
     *
     * @param s The serializer to use for serializing the field.
     */
    bool init(bool gw, const std::shared_ptr<Serializer<T, U, csz>> &s) {
        _serializer = s;
        return StateField<T>::init(true, gw);
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
class ReadableStateFieldBase : public StateFieldBase {
   protected:
    /**
     * @brief Constructor. Should not be used.
     */
    ReadableStateFieldBase(const std::string &name) : StateFieldBase(name) {}
};

/**
 * @brief A state field that is readable only, i.e. whose value cannot be
 * modified via uplink.
 *
 * @tparam T Type of state field.
 * @tparam compressed_size Size, in bits, of field when its value is compressed.
 */
template <typename T, typename U, unsigned int csz>
class ReadableStateField : public SerializableStateField<T, U, csz> {
   public:
    ReadableStateField(const std::string &name) : SerializableStateField<T, U, csz>(name) {}

    /**
     * @brief Initialize a new Readable State Field object (readable from ground
     * but not writable.)
     */
    bool init(const std::shared_ptr<Serializer<T, U, csz>> &s) {
        return SerializableStateField<T, U, csz>::init(false, s);
    }
};

/**
 * @brief Empty base class for ground-writable state fields.
 */
class WritableStateFieldBase : public StateFieldBase {
   protected:
    /**
     * @brief Constructor. Should not be used.
     */
    WritableStateFieldBase(const std::string &name) : StateFieldBase(name) {}
};

/**
 * @brief A state field that is writable, i.e. whose value can be modified via
 * uplink.
 *
 * @tparam T Type of state field.
 * @tparam compressed_size Size, in bits, of field when its value is compressed.
 */
template <typename T, typename U, unsigned int csz>
class WritableStateField : public SerializableStateField<T, U, csz> {
   public:
    WritableStateField(const std::string &name) : SerializableStateField<T, U, csz>(name) {}

    /**
     * @brief Initialize a new Writable State Field object (readable and writable
     * from ground.)
     */
    bool init(const std::shared_ptr<Serializer<T, U, csz>> &s) {
        return SerializableStateField<T, U, csz>::init(true, s);
    }
};
