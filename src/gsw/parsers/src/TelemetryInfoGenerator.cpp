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
    else if (std::is_same<f_vector_t, T>::value) return "std float vector";
    else if (std::is_same<lin::Vector3f, T>::value) return "lin float vector";
    else if (std::is_same<d_vector_t, T>::value) return "std double vector";
    else if (std::is_same<lin::Vector3d, T>::value) return "lin double vector";
    else if (std::is_same<f_quat_t, T>::value) return "std float quaternion";
    else if (std::is_same<lin::Vector4f, T>::value) return "lin float quaternion";
    else if (std::is_same<d_quat_t, T>::value) return "std double quaternion";
    else if (std::is_same<lin::Vector4d, T>::value) return "lin double quaternion";
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
bool try_collect_field_info(const StateFieldBaseType* field, TelemetryInfoGenerator::FieldData& field_info) {
    const StateFieldType<UnderlyingType>* ptr = dynamic_cast<const StateFieldType<UnderlyingType>*>(field);
    if (!ptr) return false;

    field_info.type = type_name<UnderlyingType>();
    field_info.min = std::to_string(ptr->get_serializer_min());
    field_info.max = std::to_string(ptr->get_serializer_max());
    return true;
}

template<template<typename> class StateFieldType,
         typename UnderlyingType,
         class StateFieldBaseType>
bool try_collect_vector_field_info(const StateFieldBaseType* field, TelemetryInfoGenerator::FieldData& field_info) {
    static_assert(std::is_floating_point<UnderlyingType>::value,
        "Can't collect vector field info for a vector of non-float or non-double type.");
    
    using UnderlyingArrayVectorType = std::array<UnderlyingType, 3>;
    using UnderlyingLinVectorType = lin::Vector<UnderlyingType, 3>;

    const StateFieldType<UnderlyingArrayVectorType>* ptr1 =
        dynamic_cast<const StateFieldType<UnderlyingArrayVectorType>*>(field);
    const StateFieldType<UnderlyingLinVectorType>* ptr2 =
        dynamic_cast<const StateFieldType<UnderlyingLinVectorType>*>(field);
    if (ptr1) {
        field_info.type = type_name<UnderlyingArrayVectorType>();
        field_info.min = std::to_string(ptr1->get_serializer_min()[0]);
        field_info.max = std::to_string(ptr1->get_serializer_max()[0]);
    }
    else if (ptr2) {
        field_info.type = type_name<UnderlyingLinVectorType>();
        field_info.min = std::to_string(ptr2->get_serializer_min()(0));
        field_info.max = std::to_string(ptr2->get_serializer_max()(0));
    }
    else return false;

    return true;
}

template<template<typename> class StateFieldType,
         typename UnderlyingType,
         class StateFieldBaseType>
bool try_collect_unbounded_field_info(const StateFieldBaseType* field, TelemetryInfoGenerator::FieldData& field_info) {
    static_assert(std::is_same<UnderlyingType, gps_time_t>::value
                  || std::is_same<UnderlyingType, bool>::value
                  || std::is_same<UnderlyingType, d_quat_t>::value
                  || std::is_same<UnderlyingType, f_quat_t>::value
                  || std::is_same<UnderlyingType, lin::Vector4d>::value
                  || std::is_same<UnderlyingType, lin::Vector4f>::value,
        "Can't collect unbounded field info for a non-bool, non-GPS time, or non-quaternion type.");

    const StateFieldType<UnderlyingType>* ptr = dynamic_cast<const StateFieldType<UnderlyingType>*>(field);
    if (!ptr) return false;

    field_info.type = type_name<UnderlyingType>();
    return true;
}

template<template<typename> class StateFieldType, class StateFieldBaseType>
TelemetryInfoGenerator::FieldData get_field_info(const StateFieldBaseType* field) {
    TelemetryInfoGenerator::FieldData field_info;
    field_info.bitsize = field->bitsize();

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
    found_field_type |= try_collect_unbounded_field_info<StateFieldType, lin::Vector4f, StateFieldBaseType>(field, field_info);
    found_field_type |= try_collect_unbounded_field_info<StateFieldType, d_quat_t, StateFieldBaseType>(field, field_info);
    found_field_type |= try_collect_unbounded_field_info<StateFieldType, lin::Vector4d, StateFieldBaseType>(field, field_info);
    
    if(!found_field_type) {
        std::cout << "Could not find field type for field: " << field->name() << std::endl;
        assert(false);
    }

    return field_info;
}

TelemetryInfoGenerator::FieldData get_writable_field_info(const WritableStateFieldBase* field) {
    TelemetryInfoGenerator::FieldData field_info = get_field_info<WritableStateField, WritableStateFieldBase>(field);
    field_info.writable = true;
    return field_info;
}

TelemetryInfoGenerator::FieldData get_readable_field_info(const ReadableStateFieldBase* field) {
    TelemetryInfoGenerator::FieldData field_info = get_field_info<ReadableStateField, ReadableStateFieldBase>(field);
    field_info.writable = false;
    return field_info;
}
/************** End helper functions. ***********/

TelemetryInfoGenerator::TelemetryInfo
TelemetryInfoGenerator::generate_telemetry_info()
{
    TelemetryInfo ret;

    // Get field data
    for(const WritableStateFieldBase* wf : r.writable_fields) {
        const std::string& field_name = wf->name();
        ret.field_data.insert({field_name, get_writable_field_info(wf)});

        if (wf->eeprom_save_period() > 0)
            ret.eeprom_saved_fields[field_name] = wf->eeprom_save_period();
    }

    for(const ReadableStateFieldBase* rf : r.readable_fields) {
        const std::string& field_name = rf->name();
        if (ret.field_data.find(field_name) != ret.field_data.end()) continue;
        ret.field_data.insert({field_name, get_readable_field_info(rf)});
        
        if (rf->eeprom_save_period() > 0)
            ret.eeprom_saved_fields[field_name] = rf->eeprom_save_period();
    }

    // Get flow data
    ret.flow_data = flow_data;
    for(size_t i = 0; i < flow_data.size(); i++) {
        const DownlinkProducer::FlowData& f = flow_data[i];
        for (const std::string& field_name : f.field_list) {
            ret.field_data[field_name].flow_id = f.id;
        }
    }

    return ret;
}

nlohmann::json TelemetryInfoGenerator::generate_telemetry_info_json()
{
    json j;
    to_json(j, generate_telemetry_info());
    return j;
}

void to_json(json& j, const TelemetryInfoGenerator::FieldData& d)
{
    j = json{
        {"type", d.type},
        {"writable", d.writable},
        {"bitsize", d.bitsize}};

    if (d.flow_id > 0) j["flow_id"] = d.flow_id;
    else j["flow_id"] = "undefined";
    
    if (d.type == "unsigned int" || d.type == "unsigned char")
    {
        j["min"] = std::stoul(d.min);
        j["max"] = std::stoul(d.max);
    }
    else if (d.type == "signed int" || d.type == "signed char")
    {
        j["min"] = std::stoi(d.min);
        j["max"] = std::stoi(d.max);
    }
    else if (d.type == "float"
             || d.type == "double"
             || d.type == "std float vector"
             || d.type == "std double vector"
             || d.type == "lin float vector"
             || d.type == "lin double vector"
             || d.type == "std float quaternion"
             || d.type == "std double quaternion"
             || d.type == "lin float quaternion"
             || d.type == "lin double quaternion")
    {
        try {
            j["min"] = std::stod(d.min);
            j["max"] = std::stod(d.max);
        }
        catch(const std::exception& e) {}
    }
}
void from_json(const json& j, TelemetryInfoGenerator::FieldData& d)
{
    d.type = j["type"].get<std::string>();
    d.flow_id = j["flow_id"].get<unsigned char>();
    d.writable = j["writable"].get<bool>();
    d.writable = j["bitsize"].get<bool>();
    d.min = j["min"].get<std::string>();
    d.max = j["max"].get<std::string>();
}

void to_json(json& j, const DownlinkProducer::FlowData& d)
{
    j = json({{"active", d.is_active}, {"fields", d.field_list}, {"id", d.id}, {"priority", d.id-1}});
}

void from_json(const json& j, DownlinkProducer::FlowData& d)
{
    d.is_active = j["active"].get<bool>();
    for(auto const& field : j["fields"])
        d.field_list.push_back(field);
    d.id = j["id"].get<unsigned char>();
}

void to_json(json& j, const TelemetryInfoGenerator::TelemetryInfo& d)
{
    j = json({{"eeprom_saved_fields", d.eeprom_saved_fields}, {"fields", d.field_data}, {"flow_data", d.flow_data}});
}

void from_json(const json& j, TelemetryInfoGenerator::TelemetryInfo& d)
{
    for(auto const& field : j["eeprom_saved_fields"].items())
    {
        d.eeprom_saved_fields.insert({field.key(), field.value().get<unsigned int>()});
    }
    for(auto const& field : j["field_data"].items())
    {
        TelemetryInfoGenerator::FieldData fd;
        from_json(field.value(), fd);
        d.field_data.insert({field.key(), fd});
    }
    for(auto const& field : j["flow_data"].items())
    {
        DownlinkProducer::FlowData fd;
        from_json(field.value(), fd);
        d.flow_data.push_back(fd);
    }
}
