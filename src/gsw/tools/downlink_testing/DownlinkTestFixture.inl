#include "DownlinkTestFixture.hpp"
#include <sstream>

/******************************************************************************
 ************************* Input value generation *****************************
 *****************************************************************************/

inline std::string unsigned_val(const TelemetryInfoGenerator::FieldData &f)
{
    return std::to_string(std::stoul(f.min) + (std::rand() % (std::stoul(f.max) - std::stoul(f.min) + 1)));
}
inline std::string signed_val(const TelemetryInfoGenerator::FieldData &f)
{
    return std::to_string(std::stoi(f.min) + (std::rand() % (std::stoi(f.max) - std::stoi(f.min) + 1)));
}
template <> inline std::string DownlinkTestFixture::generate_test_input<unsigned int>(const TelemetryInfoGenerator::FieldData &f) { return unsigned_val(f); }
template <> inline std::string DownlinkTestFixture::generate_test_input<signed int>(const TelemetryInfoGenerator::FieldData &f) { return signed_val(f); }
template <> inline std::string DownlinkTestFixture::generate_test_input<unsigned char>(const TelemetryInfoGenerator::FieldData &f) { return unsigned_val(f); }
template <> inline std::string DownlinkTestFixture::generate_test_input<signed char>(const TelemetryInfoGenerator::FieldData &f) { return signed_val(f); }

template <> inline std::string DownlinkTestFixture::generate_test_input<float>(const TelemetryInfoGenerator::FieldData& f) {
    return std::to_string(std::stof(f.min) + std::rand() * (std::stof(f.max) - std::stof(f.min)) / RAND_MAX);
}
template <> inline std::string DownlinkTestFixture::generate_test_input<double>(const TelemetryInfoGenerator::FieldData &f) {
    return std::to_string(std::stod(f.min) + std::rand() * (std::stod(f.max) - std::stod(f.min)) / RAND_MAX);
}

template <> inline std::string DownlinkTestFixture::generate_test_input<bool>(const TelemetryInfoGenerator::FieldData &f) {
    return std::rand() % 2 == 0 ? "true" : "false";
}

template <> inline std::string DownlinkTestFixture::generate_test_input<gps_time_t>(const TelemetryInfoGenerator::FieldData &f)
{
    unsigned short int wn = 2000 + std::rand() % 1000;
    unsigned int tow = std::rand();
    unsigned int ns = std::rand();
    std::stringstream gps_str;
    gps_str << wn << "," << tow << "," << ns;
    return gps_str.str();
}

template <typename T> inline
std::string vector_val(const TelemetryInfoGenerator::FieldData &f, std::function<T(const std::string&)> &strfn)
{
    T r = std::stod(f.min) + std::rand() * (strfn(f.max) - strfn(f.min)) / RAND_MAX;
    T t = VectorSerializer<T>::pi * rand() / T(RAND_MAX);
    T p = 2 * VectorSerializer<T>::pi * rand() / T(RAND_MAX);
    std::stringstream vector_str;
    vector_str << r * std::sin(t) * std::cos(p) << "," << r * std::sin(t) * std::sin(p) << "," << r * std::cos(t) << ",";
    return vector_str.str();
}
template <> inline std::string DownlinkTestFixture::generate_test_input<f_vector_t>(const TelemetryInfoGenerator::FieldData& f) {
    std::function<float(const std::string&)> strfn = [](const std::string& s) { return std::stof(s); };
    return vector_val<float>(f, strfn);
}
template <>
inline std::string DownlinkTestFixture::generate_test_input<d_vector_t>(const TelemetryInfoGenerator::FieldData &f)
{
    std::function<double(const std::string&)> strfn = [](const std::string &s) { return std::stod(s); };
    return vector_val<double>(f, strfn);
}
template <>
inline std::string DownlinkTestFixture::generate_test_input<lin::Vector3f>(const TelemetryInfoGenerator::FieldData &f) { return DownlinkTestFixture::generate_test_input<f_vector_t>(f); }
template <>
inline std::string DownlinkTestFixture::generate_test_input<lin::Vector3d>(const TelemetryInfoGenerator::FieldData &f) { return DownlinkTestFixture::generate_test_input<d_vector_t>(f); }

template<typename T> inline
std::string quat_val() {
    T t = VectorSerializer<T>::pi * rand() / T(RAND_MAX);
    T p = 2 * VectorSerializer<T>::pi * rand() / T(RAND_MAX);
    T a = VectorSerializer<T>::pi * rand() / T(RAND_MAX);
    std::stringstream quat_str;
    quat_str << std::sin(t) * std::cos(p) * std::sin(a / 2) << "," << std::sin(t) * std::sin(p) * std::sin(a / 2)
             << "," << std::cos(t) * std::sin(a / 2) << "," << std::cos(a / 2) << ",";
    return quat_str.str();
}
template <> inline std::string DownlinkTestFixture::generate_test_input<f_quat_t>(const TelemetryInfoGenerator::FieldData &f) { return quat_val<float>(); }
template <> inline std::string DownlinkTestFixture::generate_test_input<d_quat_t>(const TelemetryInfoGenerator::FieldData &f) { return quat_val<double>(); }
template <> inline std::string DownlinkTestFixture::generate_test_input<lin::Vector4f>(const TelemetryInfoGenerator::FieldData& f) { return DownlinkTestFixture::generate_test_input<f_quat_t>(f); }
template <> inline std::string DownlinkTestFixture::generate_test_input<lin::Vector4d>(const TelemetryInfoGenerator::FieldData &f) { return DownlinkTestFixture::generate_test_input<d_quat_t>(f); }

/******************************************************************************
 *********************** Input metadata generation ****************************
 *****************************************************************************/

template <> inline
TelemetryInfoGenerator::FieldData DownlinkTestFixture::generate_telemetry_info<unsigned int>() { return TelemetryInfoGenerator::FieldData(); }
template <> inline
TelemetryInfoGenerator::FieldData DownlinkTestFixture::generate_telemetry_info<signed int>() { return TelemetryInfoGenerator::FieldData(); }
template <> inline
TelemetryInfoGenerator::FieldData DownlinkTestFixture::generate_telemetry_info<unsigned char>() { return TelemetryInfoGenerator::FieldData(); }
template <> inline
TelemetryInfoGenerator::FieldData DownlinkTestFixture::generate_telemetry_info<signed char>() { return TelemetryInfoGenerator::FieldData(); }
template <> inline
TelemetryInfoGenerator::FieldData DownlinkTestFixture::generate_telemetry_info<float>() { return TelemetryInfoGenerator::FieldData(); }
template <> inline
TelemetryInfoGenerator::FieldData DownlinkTestFixture::generate_telemetry_info<double>() { return TelemetryInfoGenerator::FieldData(); }
template <> inline
TelemetryInfoGenerator::FieldData DownlinkTestFixture::generate_telemetry_info<bool>() { return TelemetryInfoGenerator::FieldData(); }
template <> inline
TelemetryInfoGenerator::FieldData DownlinkTestFixture::generate_telemetry_info<gps_time_t>() { return TelemetryInfoGenerator::FieldData(); }
template <> inline
TelemetryInfoGenerator::FieldData DownlinkTestFixture::generate_telemetry_info<f_vector_t>() { return TelemetryInfoGenerator::FieldData(); }
template <> inline
TelemetryInfoGenerator::FieldData DownlinkTestFixture::generate_telemetry_info<d_vector_t>() { return TelemetryInfoGenerator::FieldData(); }
template <> inline
TelemetryInfoGenerator::FieldData DownlinkTestFixture::generate_telemetry_info<f_quat_t>() { return TelemetryInfoGenerator::FieldData(); }
template <> inline
TelemetryInfoGenerator::FieldData DownlinkTestFixture::generate_telemetry_info<d_quat_t>() { return TelemetryInfoGenerator::FieldData(); }
template <> inline
TelemetryInfoGenerator::FieldData DownlinkTestFixture::generate_telemetry_info<lin::Vector3f>() { return TelemetryInfoGenerator::FieldData(); }
template <> inline
TelemetryInfoGenerator::FieldData DownlinkTestFixture::generate_telemetry_info<lin::Vector3d>() { return TelemetryInfoGenerator::FieldData(); }
template <> inline
TelemetryInfoGenerator::FieldData DownlinkTestFixture::generate_telemetry_info<lin::Vector4f>() { return TelemetryInfoGenerator::FieldData(); }
template <> inline
TelemetryInfoGenerator::FieldData DownlinkTestFixture::generate_telemetry_info<lin::Vector4d>() { return TelemetryInfoGenerator::FieldData(); }

/******************************************************************************
 ************************ Input/output comparison *****************************
 *****************************************************************************/

template <> inline
DownlinkTestFixture::test_error_t DownlinkTestFixture::compare<unsigned int>(const std::string &input, const std::string output) {return test_error_t();}
template <> inline
DownlinkTestFixture::test_error_t DownlinkTestFixture::compare<signed int>(const std::string &input, const std::string output) {return test_error_t();}
template <> inline
DownlinkTestFixture::test_error_t DownlinkTestFixture::compare<unsigned char>(const std::string& input, const std::string output) {return test_error_t();}
template <> inline
DownlinkTestFixture::test_error_t DownlinkTestFixture::compare<signed char>(const std::string &input, const std::string output) {return test_error_t();}
template <> inline
DownlinkTestFixture::test_error_t DownlinkTestFixture::compare<float>(const std::string& input, const std::string output) {return test_error_t();}
template <> inline
DownlinkTestFixture::test_error_t DownlinkTestFixture::compare<double>(const std::string &input, const std::string output) {return test_error_t();}
template <> inline
DownlinkTestFixture::test_error_t DownlinkTestFixture::compare<bool>(const std::string &input, const std::string output) {return test_error_t();}
template <> inline
DownlinkTestFixture::test_error_t DownlinkTestFixture::compare<gps_time_t>(const std::string &input, const std::string output) {return test_error_t();}
template <> inline
DownlinkTestFixture::test_error_t DownlinkTestFixture::compare<f_vector_t>(const std::string &input, const std::string output) {return test_error_t();}
template <> inline
DownlinkTestFixture::test_error_t DownlinkTestFixture::compare<d_vector_t>(const std::string &input, const std::string output) {return test_error_t();}
template <> inline
DownlinkTestFixture::test_error_t DownlinkTestFixture::compare<f_quat_t>(const std::string &input, const std::string output) {return test_error_t();}
template <> inline
DownlinkTestFixture::test_error_t DownlinkTestFixture::compare<d_quat_t>(const std::string& input, const std::string output) {return test_error_t();}
template <> inline
DownlinkTestFixture::test_error_t DownlinkTestFixture::compare<lin::Vector3f>(const std::string &input, const std::string output) {return test_error_t();}
template <> inline
DownlinkTestFixture::test_error_t DownlinkTestFixture::compare<lin::Vector3d>(const std::string &input, const std::string output) {return test_error_t();}
template <> inline
DownlinkTestFixture::test_error_t DownlinkTestFixture::compare<lin::Vector4f>(const std::string& input, const std::string output) {return test_error_t();}
template <> inline
DownlinkTestFixture::test_error_t DownlinkTestFixture::compare<lin::Vector4d>(const std::string &input, const std::string output) {return test_error_t();}