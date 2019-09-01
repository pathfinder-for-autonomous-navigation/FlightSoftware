
#include "Serializer.hpp"

/**
 * @brief Specialization of Serializer for booleans.
 */
template <>
class Serializer<bool, bool, SerializerConstants::bool_sz> : public SerializerBase<bool> {
   public:
    Serializer() : SerializerBase<bool>(false, true) {}

    bool init() {
        // Already initialized.
        return true;
    }

    bool serialize(const bool &src, std::shared_ptr<std::bitset<bool_sz>> &dest) {
        if (!is_initialized()) return false;
        (*dest)[0] = src;
        return true;
    }

    bool deserialize(const std::bitset<bool_sz> &src, std::shared_ptr<bool> &dest) {
        if (!is_initialized()) return false;
        *dest = src[0];
        return true;
    }

    bool print(const bool &src, std::shared_ptr<std::string> &dest) {
        if (!is_initialized()) return false;
        *dest = src ? "true" : "false";
        return true;
    }
};

/**
 * @brief Specialization of Serializer for unsigned ints.
 */
template <size_t csz>
class Serializer<unsigned int, unsigned int, csz> : public SerializerBase<unsigned int> {
   public:
    Serializer() : SerializerBase<unsigned int>(0, 0) {}

    unsigned int _resolution() const {
        return (unsigned int)lround(ceil((_max - _min) / pow(2.0f, csz)));
    }

    bool serialize(const unsigned int &src, std::shared_ptr<std::bitset<csz>> &dest) {
        if (!is_initialized()) return false;

        unsigned int src_copy = src;
        if (src_copy > _max) src_copy = _max;
        if (src_copy < _min) src_copy = _min;
        unsigned int result_int = (src_copy - _min) / _resolution();
        std::bitset<csz> result_copy(result_int);
        *dest = result_copy;

        return true;
    }

    bool deserialize(const std::bitset<csz> &src, std::shared_ptr<unsigned int> &dest) {
        if (!is_initialized()) return false;
        *dest = _min + src.to_ulong() * _resolution();
        return true;
    }

    bool print(const unsigned int &src, std::shared_ptr<std::string> &dest) {
        if (!is_initialized()) return false;

        char buf[20];
        sprintf(buf, "%d", src);
        *dest = buf;
        return true;
    }
};

/**
 * @brief Specialization of Serializer for signed integers.
 */
template <size_t csz>
class Serializer<signed int, signed int, csz> : public SerializerBase<signed int> {
   public:
    Serializer() : SerializerBase<signed int>(0, 0) {}

    unsigned int _resolution() const {
        return (unsigned int)lround(ceil((_max - _min) / pow(2.0f, csz)));
    }

    bool serialize(const signed int &src, std::shared_ptr<std::bitset<csz>> &dest) {
        if (!is_initialized()) return false;

        unsigned int src_copy = src;
        if (src_copy > _max) src_copy = _max;
        if (src_copy < _min) src_copy = _min;
        unsigned int result_int = (src_copy - _min) / _resolution();
        std::bitset<csz> result_copy(result_int);
        *dest = result_copy;

        return true;
    }

    bool deserialize(const std::bitset<csz> &src, std::shared_ptr<signed int> &dest) {
        if (!is_initialized()) return false;
        *dest = _min + src.to_ulong() * _resolution();
        return true;
    }

    bool print(const signed int &src, std::shared_ptr<std::string> &dest) {
        if (!is_initialized()) return false;

        char buf[20];
        sprintf(buf, "%d", src);
        *dest = buf;
        return false;
    }
};

/**
 * @brief Specialization of Serializer for floats.
 */
template <size_t csz>
class Serializer<float, float, csz> : public SerializerBase<float> {
   public:
    Serializer() : SerializerBase<float>(0.0f, 0.0f) {}

    bool init(float min, float max) {
        _min = min;
        _max = max;
        return InitializationRequired::init();
    }

    bool serialize(const float &src, std::shared_ptr<std::bitset<csz>> &dest) {
        if (!is_initialized()) return false;

        float src_copy = src;
        if (src_copy > _max) src_copy = _max;
        if (src_copy < _min) src_copy = _min;
        float resolution = (_max - _min) / pow(2, csz);
        unsigned int result_int = (unsigned int)((src_copy - _min) / resolution);
        std::bitset<csz> result_copy(result_int);
        *dest = result_copy;

        return true;
    }

    bool deserialize(const std::bitset<csz> &src, std::shared_ptr<float> &dest) {
        if (!is_initialized()) return false;

        unsigned long f_bits = src.to_ullong();
        float resolution = (_max - _min) / pow(2, csz);
        *dest = _min + resolution * f_bits;

        return true;
    }

    bool print(const float &src, std::shared_ptr<std::string> &dest) {
        if (!is_initialized()) return false;

        char buf[20];
        sprintf(buf, "%f", src);
        *dest = buf;
        return true;
    }
};

/**
 * @brief Specialization of Serializer for doubles.
 */
template <size_t csz>
class Serializer<double, double, csz> : public SerializerBase<double> {
   public:
    Serializer() : SerializerBase<double>(0, 0) {}

    bool serialize(const double &src, std::shared_ptr<std::bitset<csz>> &dest) {
        if (!is_initialized()) return false;

        double src_copy = src;
        if (src_copy > _max) src_copy = _max;
        if (src_copy < _min) src_copy = _min;
        double resolution = (_max - _min) / pow(2, csz);
        unsigned int result_int = (unsigned int)((src_copy - _min) / resolution);
        std::bitset<csz> result_copy(result_int);
        *dest = result_copy;

        return true;
    }

    bool deserialize(const std::bitset<csz> &src, std::shared_ptr<double> &dest) {
        if (!is_initialized()) return false;

        unsigned long f_bits = src.to_ullong();
        double resolution = (_max - _min) / pow(2, csz);
        return _min + resolution * f_bits;

        return true;
    }

    bool print(const double &src, std::shared_ptr<std::string> &dest) {
        if (!is_initialized()) return false;

        char buf[20];
        sprintf(buf, "%f", src);
        *dest = buf;
        return true;
    }
};

/**
 * @brief Specialization of Serializer for float vector.
 */
template <size_t csz>
class Serializer<f_vector_t, float, csz> : public SerializerBase<float> {
   public:
    Serializer() : SerializerBase(0.0f, 0.0f) {}

    bool serialize(const f_vector_t &src, std::shared_ptr<std::bitset<csz>> &dest) {
        if (!is_initialized()) return false;

        float mag = sqrtf(powf(src[0], 2.0f) + pow(src[1], 2.0f) + pow(src[2], 2.0f));
        constexpr unsigned int magnitude_bitsize = csz - f_vec_min_sz;
        std::bitset<magnitude_bitsize> magnitude_representation;

        Serializer<float, float, magnitude_bitsize> magnitude_serializer;
        magnitude_serializer.init(_min, _max);
        magnitude_serializer.serialize(mag, &magnitude_representation);

        // Compress unit vector into two compressed floats
        std::bitset<f_vec_min_sz> vec_representation;
        std::bitset<f_vec_component_sz> vec_element_representations[2];
        std::array<float, 3> v_mags;  // Magnitudes of elements in vector
        for (size_t i = 0; i < 3; i++) v_mags[i] = std::abs(src[i]);

        int max_element_idx = 0;
        float max_element_value = std::max(std::max(v_mags[0], v_mags[1]), v_mags[2]);
        if (v_mags[0] == max_element_value) max_element_idx = 0;
        if (v_mags[1] == max_element_value) max_element_idx = 1;
        if (v_mags[2] == max_element_value) max_element_idx = 2;
        int vec_number = 0;  // The current compressed vector bitset that we're modifying
        for (size_t i = 0; i < 3; i++) {
            if (i == max_element_idx) {
                std::bitset<2> largest_element_representation(i);
                vec_representation.set(0, largest_element_representation[0]);
                vec_representation.set(1, largest_element_representation[1]);
            } else {
                float v_element_scaled = src[vec_number] / mag;

                Serializer<float, float, f_vec_component_sz> vector_element_serializer;
                auto vec_element_representation_ptr =
                    std::shared_ptr<std::bitset<f_vec_component_sz>>(
                        &vec_element_representations[vec_number]);
                vector_element_serializer.init(0, sqrtf(2.0f));
                vector_element_serializer.serialize(v_element_scaled,
                                                    vec_element_representation_ptr);
                vec_number++;
            }
        }
        // Add floats to final bit representation
        for (size_t i = 0; i < f_vec_component_sz; i++) {
            vec_representation.set(i + 2, vec_element_representations[0][i]);
            vec_representation.set(i + 11, vec_element_representations[1][i]);
        }
        for (size_t i = 0; i < magnitude_bitsize; i++) (*dest).set(i, magnitude_representation[i]);
        for (size_t i = 0; i < f_vec_min_sz; i++)
            (*dest).set(i + magnitude_bitsize, vec_representation[i]);

        return true;
    }

    bool deserialize(const std::bitset<csz> &src, std::shared_ptr<f_vector_t> &dest) {
        if (!is_initialized()) return false;

        constexpr size_t magnitude_bitsize = csz - f_vec_min_sz;
        std::bitset<magnitude_bitsize> magnitude_packed;
        for (size_t i = 0; i < magnitude_bitsize; i++) magnitude_packed.set(i, src[i]);

        float magnitude = 0.0f;
        Serializer<float, float, magnitude_bitsize> magnitude_serializer;
        magnitude_serializer.init(_min, _max);
        magnitude_serializer.deserialize(magnitude_packed, &magnitude);

        unsigned int missing_component = (src[magnitude_bitsize] << 1) + src[magnitude_bitsize + 1];
        (*dest)[missing_component] = 1;
        int j = 0;  // Index of current component being processed
        for (size_t i = 0; i < 3; i++) {
            if (i != missing_component) {
                std::bitset<f_vec_component_sz> vec_component_packed;
                for (int k = 0; k < f_vec_component_sz; k++)
                    vec_component_packed.set(
                        k, src[csz - f_vec_min_sz + 2 + j * f_vec_component_sz + k]);

                Serializer<float, float, f_vec_component_sz> vec_component_serializer;
                vec_component_serializer.init(0.0f, sqrtf(2.0f));
                auto component_ptr = std::shared_ptr<float>(&(*dest)[i]);
                vec_component_serializer.deserialize(vec_component_packed, component_ptr);
                j++;
            }
        }
        (*dest)[missing_component] = sqrt((*dest)[missing_component]);

        for (size_t i = 0; i < 3; i++) (*dest)[i] *= magnitude;

        return true;
    }

    bool print(const f_vector_t &src, std::shared_ptr<std::string> &dest) {
        if (!is_initialized()) return false;
        // TODO
        return true;
    }
};

/**
 * @brief Specialization of Serializer for double vector.
 */
template <size_t csz>
class Serializer<d_vector_t, double, csz> : public SerializerBase<double> {
   public:
    Serializer() : SerializerBase<double>(0.0, 0.0) {}

    bool serialize(const d_vector_t &src, std::shared_ptr<std::bitset<csz>> &dest) {
        if (!is_initialized()) return false;

        double mag = sqrt(pow(src[0], 2) + pow(src[1], 2) + pow(src[2], 2));
        constexpr unsigned int magnitude_bitsize = csz - d_vec_min_sz;
        std::bitset<magnitude_bitsize> magnitude_representation;

        Serializer<double, double, magnitude_bitsize> magnitude_serializer;
        magnitude_serializer.init(_min, _max);
        magnitude_serializer.serialize(mag, &magnitude_representation);

        // Compress unit vector into two compressed doubles
        std::bitset<d_vec_min_sz> vec_representation;
        std::bitset<d_vec_component_sz> vec_element_representations[2];
        std::array<double, 3> v_mags;  // Magnitudes of elements in vector
        for (size_t i = 0; i < 3; i++) v_mags[i] = std::abs(src[i]);

        int max_element_idx = 0;
        double max_element_value = std::max(std::max(v_mags[0], v_mags[1]), v_mags[2]);
        if (v_mags[0] == max_element_value) max_element_idx = 0;
        if (v_mags[1] == max_element_value) max_element_idx = 1;
        if (v_mags[2] == max_element_value) max_element_idx = 2;
        int vec_number = 0;  // The current compressed vector bitset that we're modifying
        for (size_t i = 0; i < 3; i++) {
            if (i == max_element_idx) {
                std::bitset<2> largest_element_representation(i);
                vec_representation.set(0, largest_element_representation[0]);
                vec_representation.set(1, largest_element_representation[1]);
            } else {
                double v_element_scaled = src[vec_number] / mag;
                Serializer<double, double, d_vec_component_sz> vector_element_serializer;
                auto vec_element_representation_ptr =
                    std::shared_ptr<std::bitset<d_vec_component_sz>>(
                        &vec_element_representations[vec_number]);
                vector_element_serializer.init(0, sqrt(2.0));
                vector_element_serializer.serialize(v_element_scaled,
                                                    vec_element_representation_ptr);
                vec_number++;
            }
        }
        // Add doubles to final bit representation
        for (size_t i = 0; i < d_vec_component_sz; i++) {
            vec_representation.set(i + 2, vec_element_representations[0][i]);
            vec_representation.set(i + 11, vec_element_representations[1][i]);
        }
        for (size_t i = 0; i < magnitude_bitsize; i++) (*dest).set(i, magnitude_representation[i]);
        for (size_t i = 0; i < d_vec_min_sz; i++)
            (*dest).set(i + magnitude_bitsize, vec_representation[i]);

        return true;
    }

    bool deserialize(const std::bitset<csz> &src, std::shared_ptr<d_vector_t> &dest) {
        if (!is_initialized()) return false;

        constexpr size_t magnitude_bitsize = csz - d_vec_min_sz;
        std::bitset<magnitude_bitsize> magnitude_packed;
        for (size_t i = 0; i < magnitude_bitsize; i++) magnitude_packed.set(i, src[i]);

        double magnitude = 0.0;
        Serializer<double, double, magnitude_bitsize> magnitude_serializer;
        magnitude_serializer.init(_min, _max);
        magnitude_serializer.deserialize(magnitude_packed, &magnitude);

        unsigned int missing_component = (src[magnitude_bitsize] << 1) + src[magnitude_bitsize + 1];
        (*dest)[missing_component] = 1;
        int j = 0;  // Index of current component being processed
        for (size_t i = 0; i < 3; i++) {
            if (i != missing_component) {
                std::bitset<d_vec_component_sz> vec_component_packed;
                for (int k = 0; k < d_vec_component_sz; k++)
                    vec_component_packed.set(
                        k, src[csz - d_vec_component_sz + 2 + j * d_vec_component_sz + k]);

                Serializer<double, double, magnitude_bitsize> vec_component_serializer;
                vec_component_serializer.init(0.0, sqrt(2.0));
                vec_component_serializer.deserialize(vec_component_packed, &((*dest)[i]));
                (*dest)[missing_component] -= pow((*dest)[i], 2);
                j++;
            }
        }
        (*dest)[missing_component] = sqrt((*dest)[missing_component]);

        for (size_t i = 0; i < 3; i++) (*dest)[i] *= magnitude;

        return true;
    }

    bool print(const d_vector_t &src, std::shared_ptr<std::string> &dest) {
        if (!is_initialized()) return false;
        // TODO
        return true;
    }
};

/**
 * @brief Specialization of Serializer for float quaternion.
 */
template <>
class Serializer<f_quat_t, float, SerializerConstants::f_quat_sz> : public SerializerBase<float> {
   public:
    Serializer() : SerializerBase<float>(0.0f, 0.0f) {}

    bool serialize(const f_quat_t &src, std::shared_ptr<std::bitset<f_quat_sz>> &dest) {
        if (!is_initialized()) return false;

        std::bitset<f_quat_component_sz> quat_element_representations[3];
        std::array<float, 4> q_mags;  // Magnitudes of elements in quaternion
        for (size_t i = 0; i < 4; i++) q_mags[i] = std::abs(src[i]);

        size_t max_element_idx = 0;
        float max_element_value =
            std::max(std::max(q_mags[0], q_mags[1]), std::max(q_mags[2], q_mags[3]));
        if (q_mags[0] == max_element_value) max_element_idx = 0;
        if (q_mags[1] == max_element_value) max_element_idx = 1;
        if (q_mags[2] == max_element_value) max_element_idx = 2;
        if (q_mags[3] == max_element_value) max_element_idx = 3;
        size_t quat_number = 0;  // The current compressed vector bitset that we're modifying
        for (size_t i = 0; i < 4; i++) {
            if (i == max_element_idx) {
                std::bitset<2> largest_element_representation(i);
                dest->set(0, largest_element_representation[0]);
                dest->set(1, largest_element_representation[1]);
            } else {
                Serializer<float, float, f_quat_component_sz> quat_element_serializer;
                auto quat_element_representation_ptr =
                    std::shared_ptr<std::bitset<f_quat_component_sz>>(
                        &quat_element_representations[quat_number]);
                quat_element_serializer.init(0.0f, sqrtf(2.0f));
                quat_element_serializer.serialize(src[quat_number],
                                                  quat_element_representation_ptr);
                quat_number++;
            }
        }
        for (size_t i = 0; i < f_quat_component_sz; i++) {
            dest->set(i + 2, quat_element_representations[0][i]);
            dest->set(i + 2 + 1 * f_quat_component_sz, quat_element_representations[1][i]);
            dest->set(i + 2 + 2 * f_quat_component_sz, quat_element_representations[2][i]);
        }

        return true;
    }

    bool deserialize(const std::bitset<f_quat_sz> &src, std::shared_ptr<f_quat_t> &dest) {
        if (!is_initialized()) return false;

        size_t missing_element = (src[0] << 1) + src[1];
        (*dest)[missing_element] = 1;
        int j = 0;  // Currently processed packed quaternion element
        for (size_t i = 0; i < 4; i++) {
            if (i != missing_element) {
                std::bitset<f_quat_component_sz> quat_element_packed;
                for (int k = 0; k < 9; k++) quat_element_packed.set(k, src[2 + j * 9 + k]);

                Serializer<float, float, f_quat_component_sz> quat_element_serializer;
                auto quat_element_ptr = std::shared_ptr<float>(&(*dest)[i]);
                quat_element_serializer.init(0.0f, sqrtf(2.0f));
                quat_element_serializer.deserialize(quat_element_packed, quat_element_ptr);
                (*dest)[missing_element] -= pow((*dest)[i], 2);
            }
        }
        (*dest)[missing_element] = sqrt((*dest)[missing_element]);

        return true;
    }

    bool print(const f_quat_t &src, std::shared_ptr<std::string> &dest) {
        if (!is_initialized()) return false;
        // TODO
        return true;
    }
};

/**
 * @brief Specialization of Serializer for double quaternion.
 */
template <>
class Serializer<d_quat_t, double, SerializerConstants::d_quat_sz> : public SerializerBase<double> {
   public:
    Serializer() : SerializerBase<double>(0.0, 0.0) {}

    bool serialize(const d_quat_t &src, std::shared_ptr<std::bitset<d_quat_sz>> &dest) {
        if (!is_initialized()) return false;
        // TODO
        return true;
    }

    bool deserialize(const std::bitset<d_quat_sz> &src, std::shared_ptr<d_quat_t> &dest) {
        if (!is_initialized()) return false;
        // TODO
        return true;
    }

    bool print(const d_quat_t &src, std::shared_ptr<std::string> &dest) {
        if (!is_initialized()) return false;
        // TODO
        return true;
    }
};

/**
 * @brief Specialization of Serializer for GPS time.
 */
template <>
class Serializer<gps_time_t, bool, SerializerConstants::gps_time_sz> : public SerializerBase<bool> {
   public:
    Serializer() : SerializerBase<bool>(false, false) {}

    bool init() {
        // No need for initialization.
        return true;
    }

    bool serialize(const gps_time_t &src, std::shared_ptr<std::bitset<gps_time_sz>> &dest) {
        if (!is_initialized()) return false;

        if (src.is_not_set) {
            dest->set(0, false);
            return false;
        }
        dest->set(0, true);
        std::bitset<16> wn((unsigned short int)src.gpstime.wn);
        std::bitset<32> tow(src.gpstime.tow);
        for (size_t i = 0; i < 16; i++) dest->set(i + 1, wn[i]);
        for (size_t i = 0; i < 32; i++) dest->set(i + 17, tow[i]);

        return true;
    }

    bool deserialize(const std::bitset<gps_time_sz> &src, std::shared_ptr<gps_time_t> &dest) {
        if (!is_initialized()) return false;

        std::bitset<16> wn;
        std::bitset<32> tow;
        for (size_t i = 0; i < 16; i++) wn.set(i + 1, src[i]);
        for (size_t i = 0; i < 32; i++) tow.set(i + 1, src[16 + i]);
        dest->is_not_set = false;
        dest->gpstime.wn = (unsigned int)wn.to_ulong();
        dest->gpstime.tow = (unsigned int)tow.to_ulong();

        return true;
    }

    bool print(const gps_time_t &src, std::shared_ptr<std::string> &dest) {
        if (!is_initialized()) return false;
        // TODO
        return true;
    }
};

/**
 * @brief Specialization of Serializer for temperature values.
 */
template <>
class Serializer<temperature_t, temperature_t, SerializerConstants::temp_sz>
    : public SerializerBase<temperature_t> {
   public:
    static constexpr temperature_t TEMPERATURE_MIN = static_cast<temperature_t>(-40);
    static constexpr temperature_t TEMPERATURE_MAX = static_cast<temperature_t>(125);

    Serializer() : SerializerBase<temperature_t>(TEMPERATURE_MIN, TEMPERATURE_MAX) {}

    bool init() {
        // Already initialized by constructor.
        return true;
    }

    bool serialize(const temperature_t &src, std::shared_ptr<std::bitset<temp_sz>> &dest) {
        if (!is_initialized()) return false;
        // TODO
        return true;
    }

    bool deserialize(const std::bitset<temp_sz> &src, std::shared_ptr<temperature_t> &dest) {
        if (!is_initialized()) return false;
        // TODO
        return true;
    }

    bool print(const temperature_t &src, std::shared_ptr<std::string> &dest) {
        // TODO
        return true;
    }
};