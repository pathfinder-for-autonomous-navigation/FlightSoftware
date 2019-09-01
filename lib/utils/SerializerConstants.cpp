/**
 * @author Tanishq Aggarwal (ta335@cornell.edu)
 * @brief Defines serializer constants.
 * @date 2019-09-01
 * 
 */

#include "Serializer.hpp"

const gps_time_t SerializerConstants::dummy_gpstime;

const std::vector<std::string> serializable_types = {
    "bool", "f_quat", "f_vec", "d_quat", "d_vec", "gpstime",
    "int",  "uint",   "float",  "double"};
