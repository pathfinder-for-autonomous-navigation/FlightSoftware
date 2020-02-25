/**
 * @author Tanishq Aggarwal (ta335@cornell.edu)
 * @brief Defines serializer types.
 * @date 2019-09-01
 *
 */

#include "Serializer.hpp"

const std::vector<std::string> SerializerType::serializable_types = {
    "bool", "f_quat", "f_vec", "d_quat", "d_vec", "gpstime", "int", "uint", "float", "double"};

const gps_time_t Serializer<gps_time_t>::dummy_gpstime;
