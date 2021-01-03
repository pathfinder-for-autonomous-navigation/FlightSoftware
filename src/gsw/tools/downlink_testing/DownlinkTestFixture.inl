#include "DownlinkTestFixture.hpp"
#include <sstream>

// Wrapper functions for string-to-type conversion. These are used in all sorts of places
// in the following code.
static std::function<float(const std::string &)> stof_fn = [](const std::string &s) { return std::stof(s); };
static std::function<double(const std::string &)> stod_fn = [](const std::string &s) { return std::stod(s); };
static std::function<unsigned int(const std::string &)> stoui_fn = [](const std::string &s) { return std::stoul(s); };
static std::function<unsigned char(const std::string &)> stouc_fn = [](const std::string &s) { return std::stoul(s); };
static std::function<signed int(const std::string &)> stosi_fn = [](const std::string &s) { return std::stoi(s); };
static std::function<signed char(const std::string &)> stosc_fn = [](const std::string &s) { return std::stoi(s); };

/******************************************************************************
 ************************* Input value generation *****************************
 *****************************************************************************/

#define TEST_INPUT_GENERATOR(type) \
    template <>                   \
    inline std::string DownlinkTestFixture::generate_test_input<type>(const TelemetryInfoGenerator::FieldData &f) \

static inline std::string unsigned_val(const TelemetryInfoGenerator::FieldData &f)
{
    return std::to_string(std::stoul(f.min) + (std::rand() % (std::stoul(f.max) - std::stoul(f.min) + 1)));
}
static inline std::string signed_val(const TelemetryInfoGenerator::FieldData &f)
{
    return std::to_string(std::stoi(f.min) + (std::rand() % (std::stoi(f.max) - std::stoi(f.min) + 1)));
}
TEST_INPUT_GENERATOR(unsigned int) { return unsigned_val(f); }
TEST_INPUT_GENERATOR(signed int) { return signed_val(f); }
TEST_INPUT_GENERATOR(unsigned char) { return unsigned_val(f); }
TEST_INPUT_GENERATOR(signed char) { return signed_val(f); }

template <typename T> inline
static std::string float_val(const TelemetryInfoGenerator::FieldData &f, std::function<T(const std::string &)> &strfn)
{
    return std::to_string(strfn(f.min) + std::rand() * (strfn(f.max) - strfn(f.min)) / RAND_MAX);
}
TEST_INPUT_GENERATOR(float) { return float_val<float>(f, stof_fn); }
TEST_INPUT_GENERATOR(double) { return float_val<double>(f, stod_fn); }

TEST_INPUT_GENERATOR(bool) { return std::rand() % 2 == 0 ? "true" : "false"; }

TEST_INPUT_GENERATOR(gps_time_t)
{
    unsigned short int wn = 2000 + std::rand() % 1000;
    unsigned int tow = std::rand();
    unsigned int ns = std::rand();
    std::stringstream gps_str;
    gps_str << wn << "," << tow << "," << ns;
    return gps_str.str();
}

template <typename T>
inline static std::string vector_val(const TelemetryInfoGenerator::FieldData &f, std::function<T(const std::string &)> &strfn)
{
    T r = std::stod(f.min) + std::rand() * (strfn(f.max) - strfn(f.min)) / RAND_MAX;
    T t = VectorSerializer<T>::pi * rand() / T(RAND_MAX);
    T p = 2 * VectorSerializer<T>::pi * rand() / T(RAND_MAX);
    std::stringstream vector_str;
    vector_str << r * std::sin(t) * std::cos(p) << "," << r * std::sin(t) * std::sin(p) << "," << r * std::cos(t) << ",";
    return vector_str.str();
}
TEST_INPUT_GENERATOR(f_vector_t) { return vector_val<float>(f, stof_fn); }
TEST_INPUT_GENERATOR(d_vector_t) { return vector_val<double>(f, stod_fn); }
TEST_INPUT_GENERATOR(lin::Vector3f) { return vector_val<float>(f, stof_fn); }
TEST_INPUT_GENERATOR(lin::Vector3d) { return vector_val<double>(f, stod_fn); }

template <typename T> inline
static std::string quat_val() {
    T t = VectorSerializer<T>::pi * rand() / T(RAND_MAX);
    T p = 2 * VectorSerializer<T>::pi * rand() / T(RAND_MAX);
    T a = VectorSerializer<T>::pi * rand() / T(RAND_MAX);
    std::stringstream quat_str;
    quat_str << std::sin(t) * std::cos(p) * std::sin(a / 2) << "," << std::sin(t) * std::sin(p) * std::sin(a / 2)
             << "," << std::cos(t) * std::sin(a / 2) << "," << std::cos(a / 2) << ",";
    return quat_str.str();
}
TEST_INPUT_GENERATOR(f_quat_t) { return quat_val<float>(); }
TEST_INPUT_GENERATOR(d_quat_t) { return quat_val<double>(); }
TEST_INPUT_GENERATOR(lin::Vector4f) { return quat_val<float>(); }
TEST_INPUT_GENERATOR(lin::Vector4d) { return quat_val<double>(); }

#undef TEST_INPUT_GENERATOR

/******************************************************************************
 *********************** Input metadata generation ****************************
 *****************************************************************************/

#define TELEMETRY_INFO_GENERATOR(type) \
    template <> inline \
    void DownlinkTestFixture::generate_telemetry_info<type>(TelemetryInfoGenerator::FieldData& f)

template <typename T>
inline static std::pair<T, T> generate_random_bounds(std::function<T()> &randomfn, TelemetryInfoGenerator::FieldData &f)
{
    T min = 0;
    T max = 0;
    do
    {
        min = randomfn();
        max = randomfn();
    } while (max <= min);
    return {min, max};
}

template <typename T>
inline static void generate_int_info(std::function<T()> &randomfn, TelemetryInfoGenerator::FieldData &f)
{
    auto bounds = generate_random_bounds<T>(randomfn, f);
    f.min = std::to_string(bounds.first);
    f.max = std::to_string(bounds.second);
    f.bitsize = 1 + std::rand() % (sizeof(T)*8-1);
}

TELEMETRY_INFO_GENERATOR(unsigned int)
{
    std::function<unsigned int()> randint = []() { return static_cast<unsigned int>(std::rand()); };
    generate_int_info<unsigned int>(randint, f);
}
TELEMETRY_INFO_GENERATOR(signed int)
{
    std::function<signed int()> randint = []() {
        int x = static_cast<signed int>(std::rand());
        int y = static_cast<signed int>(std::rand());
        return x * ((y % 2 == 0) ? (-1) : 1);
    };
    generate_int_info<signed int>(randint, f);
}
TELEMETRY_INFO_GENERATOR(unsigned char)
{
    std::function<unsigned char()> randint = []() { return static_cast<unsigned char>(std::rand()); };
    generate_int_info<unsigned char>(randint, f);
}
TELEMETRY_INFO_GENERATOR(signed char)
{
    std::function<signed char()> randint = []() { return static_cast<signed char>(-256 + static_cast<unsigned char>(std::rand()) % 256); };
    generate_int_info<signed char>(randint, f);
}

template<typename T>
inline static void float_telem_info(TelemetryInfoGenerator::FieldData& f)
{
    static constexpr T rand_min = -1e9;
    static constexpr T rand_max = 1e9;
    std::function<T()> randfloat = [&]() {
        return static_cast<T>(rand_min + static_cast<T>(rand()) / (static_cast<T>(RAND_MAX / (rand_max - rand_min))));
    };
    auto bounds = generate_random_bounds<T>(randfloat, f);
    f.min = std::to_string(bounds.first);
    f.max = std::to_string(bounds.second);
    f.bitsize = 1 + std::rand() % (sizeof(T)*8-1);
}
TELEMETRY_INFO_GENERATOR(float) { float_telem_info<float>(f); }
TELEMETRY_INFO_GENERATOR(double) { float_telem_info<double>(f); }

TELEMETRY_INFO_GENERATOR(bool) { f.bitsize = 1; }
TELEMETRY_INFO_GENERATOR(gps_time_t) { f.bitsize = Serializer<gps_time_t>::gps_time_sz(); }
TELEMETRY_INFO_GENERATOR(f_quat_t) { f.bitsize = QuaternionSerializer<float>::quat_sz(); }
TELEMETRY_INFO_GENERATOR(d_quat_t) { f.bitsize = QuaternionSerializer<double>::quat_sz(); }
TELEMETRY_INFO_GENERATOR(lin::Vector4f) { f.bitsize = QuaternionSerializer<float>::quat_sz(); }
TELEMETRY_INFO_GENERATOR(lin::Vector4d) { f.bitsize = QuaternionSerializer<double>::quat_sz(); }

template <typename T>
inline static void vector_telem_info(TelemetryInfoGenerator::FieldData &f)
{
    static constexpr T rand_min = 0;
    static constexpr T rand_max = 1e9;
    std::function<T()> randfloat = [&]() {
        return static_cast<T>(rand_min + static_cast<T>(rand()) / (static_cast<T>(RAND_MAX / (rand_max - rand_min))));
    };
    auto bounds = generate_random_bounds<T>(randfloat, f);
    unsigned int precision = 1 + std::rand() % (sizeof(T) * 8 - 1);
    f.bitsize = VectorSerializer<T>::total_size(bounds.first, bounds.second, precision);
}
TELEMETRY_INFO_GENERATOR(f_vector_t) { vector_telem_info<float>(f); }
TELEMETRY_INFO_GENERATOR(d_vector_t) { vector_telem_info<double>(f); }
TELEMETRY_INFO_GENERATOR(lin::Vector3f) { vector_telem_info<float>(f); }
TELEMETRY_INFO_GENERATOR(lin::Vector3d) { vector_telem_info<double>(f); }

#undef TELEMETRY_INFO_GENERATOR

/******************************************************************************
 ************************** State field creation ******************************
 *****************************************************************************/

#define STATE_FIELD_CREATOR(type) \
    template <> inline \
    void DownlinkTestFixture::create_state_field<type>(const TelemetryInfoGenerator::FieldData &f, StateFieldRegistryMock &r)

template <typename T>
inline static void generic_creator(const TelemetryInfoGenerator::FieldData &f, std::function<T(const std::string &)> &strfn, StateFieldRegistryMock &r)
{
    T min = strfn(f.min);
    T max = strfn(f.max);
    r.create_readable_field<T>(f.name, min, max, f.bitsize);
}

STATE_FIELD_CREATOR(unsigned int) { generic_creator<unsigned int>(f, stoui_fn, r); }
STATE_FIELD_CREATOR(signed int) { generic_creator<signed int>(f, stosi_fn, r); }
STATE_FIELD_CREATOR(unsigned char) { generic_creator<unsigned char>(f, stouc_fn, r); }
STATE_FIELD_CREATOR(signed char) { generic_creator<signed char>(f, stosc_fn, r); }
STATE_FIELD_CREATOR(float) { generic_creator<float>(f, stof_fn, r); }
STATE_FIELD_CREATOR(double) { generic_creator<double>(f, stod_fn, r); }

STATE_FIELD_CREATOR(bool) { r.create_readable_field<bool>(f.name); }
STATE_FIELD_CREATOR(gps_time_t) { r.create_readable_field<gps_time_t>(f.name); }
STATE_FIELD_CREATOR(f_quat_t) { r.create_readable_field<f_quat_t>(f.name); }
STATE_FIELD_CREATOR(d_quat_t) { r.create_readable_field<d_quat_t>(f.name); }
STATE_FIELD_CREATOR(lin::Vector4f) { r.create_readable_field<lin::Vector4f>(f.name); }
STATE_FIELD_CREATOR(lin::Vector4d) { r.create_readable_field<lin::Vector4d>(f.name); }

template <typename T>
static inline void lin_vector_creator(const TelemetryInfoGenerator::FieldData &f,
    std::function<T(const std::string &)> &strfn, StateFieldRegistryMock &r)
{
    T min = strfn(f.min);
    T max = strfn(f.max);

    // The bitsize specified in the field's data is the _total_ bitsize.
    // What needs to be fed into create_readable_field is the "precision"
    // which is explained in SerializerTypes.inl.
    unsigned int precision = VectorSerializer<T>::get_precision(min, max, f.bitsize);

    r.create_readable_lin_vector_field<T>(f.name, min, max, precision);
}
template <typename T>
static inline void std_vector_creator(const TelemetryInfoGenerator::FieldData &f,
    std::function<T(const std::string &)> &strfn, StateFieldRegistryMock &r)
{
    T min = strfn(f.min);
    T max = strfn(f.max);

    // The bitsize specified in the field's data is the _total_ bitsize.
    // What needs to be fed into create_readable_field is the "precision"
    // which is explained in SerializerTypes.inl.
    unsigned int precision = VectorSerializer<T>::get_precision(min, max, f.bitsize);

    r.create_readable_vector_field<T>(f.name, min, max, precision);
}
STATE_FIELD_CREATOR(f_vector_t) { std_vector_creator<float>(f, stof_fn, r); }
STATE_FIELD_CREATOR(d_vector_t) { std_vector_creator<double>(f, stod_fn, r); }
STATE_FIELD_CREATOR(lin::Vector3f) { lin_vector_creator<float>(f, stof_fn, r); }
STATE_FIELD_CREATOR(lin::Vector3d) { lin_vector_creator<double>(f, stod_fn, r); }

#undef STATE_FIELD_CREATOR

/******************************************************************************
 ************************ Input/output comparison *****************************
 *****************************************************************************/

#define COMPARATOR(type) \
    template <> inline \
    DownlinkTestFixture::test_error_t DownlinkTestFixture::compare<type>(const TelemetryInfoGenerator::FieldData& f, const std::string &input, const std::string output)

COMPARATOR(unsigned int) {return test_error_t();}
COMPARATOR(signed int) { return test_error_t(); }
COMPARATOR(unsigned char) { return test_error_t(); }
COMPARATOR(signed char) { return test_error_t(); }
COMPARATOR(float) { return test_error_t(); }
COMPARATOR(double) { return test_error_t(); }
COMPARATOR(bool) { return test_error_t(); }
COMPARATOR(gps_time_t) { return test_error_t(); }
COMPARATOR(f_vector_t) { return test_error_t(); }
COMPARATOR(d_vector_t) { return test_error_t(); }
COMPARATOR(lin::Vector3f) { return test_error_t(); }
COMPARATOR(lin::Vector3d) { return test_error_t(); }
COMPARATOR(f_quat_t) { return test_error_t(); }
COMPARATOR(d_quat_t) { return test_error_t(); }
COMPARATOR(lin::Vector4f) { return test_error_t(); }
COMPARATOR(lin::Vector4d) { return test_error_t(); }

#undef COMPARATOR
