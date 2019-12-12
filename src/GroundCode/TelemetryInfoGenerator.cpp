#include "TelemetryInfoGenerator.hpp"
#include <type_traits>
#include <typeinfo>
#include <array>

TelemetryInfoGenerator::TelemetryInfoGenerator(
    const std::vector<DownlinkProducer::FlowData>& _flow_data) :
        r(), fcp(r, _flow_data), flow_data(_flow_data) {}

using nlohmann::json;

template<template<typename> class StateFieldType,
         typename UnderlyingType,
         class StateFieldBaseType>
void collect_field_info(const StateFieldBaseType* field, json& field_info) {
    if (typeid(field) == typeid(StateFieldType<UnderlyingType>*)) {
        const StateFieldType<UnderlyingType>* ptr = static_cast<const StateFieldType<UnderlyingType>*>(field);
        field_info["type"] = typeid(UnderlyingType).name();
        field_info["min"] = ptr->get_serializer_min();
        field_info["max"] = ptr->get_serializer_max();
    }
}

template<template<typename> class StateFieldType,
         typename UnderlyingType,
         class StateFieldBaseType>
void collect_vector_field_info(const StateFieldBaseType* field, json& field_info) {
    static_assert(std::is_same<UnderlyingType, double>::value 
                  || std::is_same<UnderlyingType, float>::value,
        "Can't collect vector field info for a vector of non-float or non-double type.");

    if (typeid(field) == typeid(StateFieldType<std::array<UnderlyingType, 3>>*)) {
        const StateFieldType<std::array<UnderlyingType, 3>>* ptr =
            static_cast<const StateFieldType<std::array<UnderlyingType, 3>>*>(field);
        field_info["type"] = typeid(UnderlyingType).name() + std::string(" vector");
        field_info["min"] = ptr->get_serializer_min()[0];
        field_info["max"] = ptr->get_serializer_max()[0];
    }
}

template<template<typename> class StateFieldType,
         typename UnderlyingType,
         class StateFieldBaseType>
void collect_unbounded_field_info(const StateFieldBaseType* field, json& field_info) {
    static_assert(std::is_same<UnderlyingType, gps_time_t>::value
                  || std::is_same<UnderlyingType, bool>::value
                  || std::is_same<UnderlyingType, d_quat_t>::value
                  || std::is_same<UnderlyingType, f_quat_t>::value,
        "Can't collect unbounded field info for a non-bool, non-GPS time, or non-quaternion type.");

    if (typeid(field) == typeid(StateFieldType<UnderlyingType>*)) {
        field_info["type"] = typeid(UnderlyingType).name();
    }
}

template<template<typename> class StateFieldType, class StateFieldBaseType>
json get_field_info(const StateFieldBaseType* field) {
    json field_info;
    field_info["bitsize"] = field->bitsize();
    collect_field_info<StateFieldType, unsigned int, StateFieldBaseType>(field, field_info);
    collect_field_info<StateFieldType, signed int, StateFieldBaseType>(field, field_info);
    collect_field_info<StateFieldType, unsigned char, StateFieldBaseType>(field, field_info);
    collect_field_info<StateFieldType, signed char, StateFieldBaseType>(field, field_info);
    collect_field_info<StateFieldType, float, StateFieldBaseType>(field, field_info);
    collect_field_info<StateFieldType, double, StateFieldBaseType>(field, field_info);
    collect_vector_field_info<StateFieldType, float, StateFieldBaseType>(field, field_info);
    collect_vector_field_info<StateFieldType, double, StateFieldBaseType>(field, field_info);
    collect_unbounded_field_info<StateFieldType, bool, StateFieldBaseType>(field, field_info);
    collect_unbounded_field_info<StateFieldType, gps_time_t, StateFieldBaseType>(field, field_info);
    collect_unbounded_field_info<StateFieldType, f_quat_t, StateFieldBaseType>(field, field_info);
    collect_unbounded_field_info<StateFieldType, d_quat_t, StateFieldBaseType>(field, field_info);
    return field_info;
}

json get_readable_field_info(const ReadableStateFieldBase* field) {
    json field_info = get_field_info<ReadableStateField, ReadableStateFieldBase>(field);
    field_info["writable"] = false;
    return field_info;
}
json get_writable_field_info(const WritableStateFieldBase* field) {
    json field_info = get_field_info<WritableStateField, WritableStateFieldBase>(field);
    field_info["writable"] = true;
    return field_info;
}

json TelemetryInfoGenerator::generate_telemetry_info() {
    json ret;

    // Get field data
    ret["fields"] = json::object();
    for(const WritableStateFieldBase* wf : r.writable_fields) {
        const std::string& field_name = wf->name();
        if (ret["fields"].find(field_name) != ret["fields"].end()) continue;
        ret["fields"][field_name] = get_writable_field_info(wf);;
    }

    for(const ReadableStateFieldBase* rf : r.readable_fields) {
        const std::string& field_name = rf->name();
        if (ret["fields"].find(field_name) != ret["fields"].end()) continue;
        ret["fields"][field_name] = get_readable_field_info(rf);
    }

    // Get flow data
    ret["flows"] = json::array();
    for(int i = 0; i < flow_data.size(); i++) {
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

    return ret;
}

#undef collect_field_info
#undef get_field_info
#undef get_readable_field_info
#undef get_writable_field_info
