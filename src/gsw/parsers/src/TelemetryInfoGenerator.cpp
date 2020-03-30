#include "TelemetryInfoGenerator.hpp"
#include <type_traits>
#include <typeinfo>
#include <array>

TelemetryInfoGenerator::TelemetryInfoGenerator(
    const std::vector<DownlinkProducer::FlowData>& _flow_data) :
        r(), fcp(r, _flow_data), flow_data(_flow_data) {}

/************** Helper functions for telemetry info generation. ***********/
using nlohmann::json;

template<typename T>
std::string type_name() {
    if      (std::is_same<unsigned int, T>::value) return "unsigned int";
    else if (std::is_same<signed int, T>::value) return "signed int";
    else if (std::is_same<unsigned char, T>::value) return "unsigned char";
    else if (std::is_same<signed char, T>::value) return "signed char";
    else if (std::is_same<float, T>::value) return "float";
    else if (std::is_same<double, T>::value) return "double";
    else if (std::is_same<f_vector_t, T>::value) return "float vector";
    else if (std::is_same<d_vector_t, T>::value) return "double vector";
    else if (std::is_same<f_quat_t, T>::value) return "float quaternion";
    else if (std::is_same<d_quat_t, T>::value) return "double quaternion";
    else if (std::is_same<bool, T>::value) return "bool";
    else if (std::is_same<gps_time_t, T>::value) return "gps_time_t";
    else {
        std::cout << "Invalid typename specified to stringify: " << typeid(T).name() << std::endl;
        assert(false);
    }
}

template<template<typename> class StateFieldType,
         typename UnderlyingType,
         class StateFieldBaseType>
bool try_collect_field_info(const StateFieldBaseType* field, json& field_info) {
    const StateFieldType<UnderlyingType>* ptr = dynamic_cast<const StateFieldType<UnderlyingType>*>(field);
    if (!ptr) return false;

    field_info["type"] = type_name<UnderlyingType>();
    field_info["min"] = ptr->get_serializer_min();
    field_info["max"] = ptr->get_serializer_max();
    return true;
}

template<template<typename> class StateFieldType,
         typename UnderlyingType,
         class StateFieldBaseType>
bool try_collect_vector_field_info(const StateFieldBaseType* field, json& field_info) {
    static_assert(std::is_floating_point<UnderlyingType>::value,
        "Can't collect vector field info for a vector of non-float or non-double type.");
    
    using UnderlyingVectorType = std::array<UnderlyingType, 3>;

    const StateFieldType<UnderlyingVectorType>* ptr =
        dynamic_cast<const StateFieldType<UnderlyingVectorType>*>(field);
    if (!ptr) return false;

    field_info["type"] = type_name<UnderlyingVectorType>();
    field_info["min"] = ptr->get_serializer_min()[0];
    field_info["max"] = ptr->get_serializer_max()[0];
    return true;
}

template<template<typename> class StateFieldType,
         typename UnderlyingType,
         class StateFieldBaseType>
bool try_collect_unbounded_field_info(const StateFieldBaseType* field, json& field_info) {
    static_assert(std::is_same<UnderlyingType, gps_time_t>::value
                  || std::is_same<UnderlyingType, bool>::value
                  || std::is_same<UnderlyingType, d_quat_t>::value
                  || std::is_same<UnderlyingType, f_quat_t>::value
                  || std::is_same<UnderlyingType, lin::Vector4d>::value
                  || std::is_same<UnderlyingType, lin::Vector4f>::value,
        "Can't collect unbounded field info for a non-bool, non-GPS time, or non-quaternion type.");

    const StateFieldType<UnderlyingType>* ptr = dynamic_cast<const StateFieldType<UnderlyingType>*>(field);
    if (!ptr) return false;

    field_info["type"] = type_name<UnderlyingType>();
    field_info["min"] = "N/A";
    field_info["max"] = "N/A";
    return true;
}

template<template<typename> class StateFieldType, class StateFieldBaseType>
json get_field_info(const StateFieldBaseType* field) {
    json field_info;
    field_info["bitsize"] = field->bitsize();

    bool found_field_type = false;
    found_field_type |= try_collect_field_info<StateFieldType, unsigned int, StateFieldBaseType>(field, field_info);
    found_field_type |= try_collect_field_info<StateFieldType, signed int, StateFieldBaseType>(field, field_info);
    found_field_type |= try_collect_field_info<StateFieldType, unsigned char, StateFieldBaseType>(field, field_info);
    found_field_type |= try_collect_field_info<StateFieldType, signed char, StateFieldBaseType>(field, field_info);
    found_field_type |= try_collect_field_info<StateFieldType, float, StateFieldBaseType>(field, field_info);
    found_field_type |= try_collect_field_info<StateFieldType, double, StateFieldBaseType>(field, field_info);
    found_field_type |= try_collect_vector_field_info<StateFieldType, float, StateFieldBaseType>(field, field_info);
    found_field_type |= try_collect_vector_field_info<StateFieldType, double, StateFieldBaseType>(field, field_info);
    found_field_type |= try_collect_unbounded_field_info<StateFieldType, bool, StateFieldBaseType>(field, field_info);
    found_field_type |= try_collect_unbounded_field_info<StateFieldType, gps_time_t, StateFieldBaseType>(field, field_info);
    found_field_type |= try_collect_unbounded_field_info<StateFieldType, f_quat_t, StateFieldBaseType>(field, field_info);
    found_field_type |= try_collect_unbounded_field_info<StateFieldType, d_quat_t, StateFieldBaseType>(field, field_info);
    
    if(!found_field_type) {
        std::cout << "Could not find field type for field: " << field->name() << std::endl;
        assert(false);
    }

    return field_info;
}

json get_writable_field_info(const WritableStateFieldBase* field) {
    json field_info = get_field_info<WritableStateField, WritableStateFieldBase>(field);
    field_info["writable"] = true;
    return field_info;
}

json get_readable_field_info(const ReadableStateFieldBase* field) {
    json field_info = get_field_info<ReadableStateField, ReadableStateFieldBase>(field);
    field_info["writable"] = false;
    return field_info;
}
/************** End helper functions. ***********/

json TelemetryInfoGenerator::generate_telemetry_info() {
    json ret;

    // Get field data
    ret["fields"] = json::object();
    for(const WritableStateFieldBase* wf : r.writable_fields) {
        const std::string& field_name = wf->name();
        if (ret["fields"].find(field_name) != ret["fields"].end()) continue;
        ret["fields"][field_name] = get_writable_field_info(wf);

        if (wf->eeprom_save_period() > 0)
            ret["eeprom_saved_fields"][field_name] = wf->eeprom_save_period();
    }

    for(const ReadableStateFieldBase* rf : r.readable_fields) {
        const std::string& field_name = rf->name();
        if (ret["fields"].find(field_name) != ret["fields"].end()) continue;
        ret["fields"][field_name] = get_readable_field_info(rf);
        
        if (rf->eeprom_save_period() > 0)
            ret["eeprom_saved_fields"][field_name] = rf->eeprom_save_period();
    }

    // Get flow data
    ret["flows"] = json::array();
    for(size_t i = 0; i < flow_data.size(); i++) {
        const DownlinkProducer::FlowData& f = flow_data[i];
        ret["flows"].push_back({
            {"id", f.id},
            {"priority", i},
            {"fields", f.field_list},
            {"active", f.is_active}
        });
        for (const std::string& field_name : f.field_list) {
            ret["fields"][field_name]["flow_id"] = f.id;
        }
    }
    for(auto& field : ret["fields"]) {
        if (field.find("flow_id") == field.end())
            field["flow_id"] = "undefined";
    }

    return ret;
}
