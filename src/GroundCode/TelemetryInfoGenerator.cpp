#include "TelemetryInfoGenerator.hpp"
#include "../FCCode/MainControlLoop.hpp"
#include <type_traits>
#include <typeinfo>

TelemetryInfoGenerator::TelemetryInfoGenerator(
    const std::vector<DownlinkProducer::FlowData>& _flow_data) : r(), flow_data(_flow_data)
{
    MainControlLoop fcp(r, flow_data);
}

#define collect_field_info(type, SerializableStateFieldType, field) { \
    typedef SerializableStateFieldType<type> serializer_specialization; \
    if (typeid(field) == typeid(serializer_specialization*)) { \
        const serializer_specialization* ptr = \
            static_cast<serializer_specialization*>(field); \
        field_info["type"] = #type; \
        field_info["min"] = ptr->get_serializer_min(); \
        field_info["max"] = ptr->get_serializer_max(); \
        field_info["bitsize"] = ptr->bitsize(); \
    } \
}

#define collect_vector_field_info(type, SerializableStateFieldType, field) { \
    typedef SerializableStateFieldType<type> serializer_specialization; \
    if (typeid(field) == typeid(serializer_specialization*)) { \
        const serializer_specialization* ptr = \
            static_cast<serializer_specialization*>(field); \
        field_info["type"] = #type; \
        field_info["min"] = ptr->get_serializer_min()[0]; \
        field_info["max"] = ptr->get_serializer_max()[0]; \
        field_info["bitsize"] = ptr->bitsize(); \
    } \
}

#define collect_unbounded_field_info(type, SerializableStateFieldType, field) { \
    typedef SerializableStateFieldType<type> serializer_specialization; \
    if (typeid(field) == typeid(serializer_specialization*)) { \
        field_info["type"] = #type; \
        field_info["bitsize"] = field->bitsize(); \
    } \
}

#define get_field_info(SerializableStateFieldType, field) { \
    collect_field_info(unsigned int, SerializableStateFieldType, field); \
    collect_field_info(signed int, SerializableStateFieldType, field); \
    collect_field_info(unsigned char, SerializableStateFieldType, field); \
    collect_field_info(signed char, SerializableStateFieldType, field); \
    collect_field_info(float, SerializableStateFieldType, field); \
    collect_field_info(double, SerializableStateFieldType, field); \
    collect_field_info(f_quat_t, SerializableStateFieldType, field); \
    collect_field_info(d_quat_t, SerializableStateFieldType, field); \
    collect_vector_field_info(f_vector_t, SerializableStateFieldType, field); \
    collect_vector_field_info(d_vector_t, SerializableStateFieldType, field); \
    collect_unbounded_field_info(bool, SerializableStateFieldType, field); \
    collect_unbounded_field_info(gps_time_t, SerializableStateFieldType, field); \
}

#define get_readable_field_info(field) get_field_info(ReadableStateField, (field))
#define get_writable_field_info(field) get_field_info(WritableStateField, (field))

using nlohmann::json;

json TelemetryInfoGenerator::generate_telemetry_info() {
    json ret;

    // Get field data
    ret["fields"] = json::object();
    for(WritableStateFieldBase* wf : r.writable_fields) {
        const std::string& field_name = wf->name();
        if (ret["fields"].find(field_name) != ret["fields"].end()) continue;
        json field_info;
        get_writable_field_info(wf);
        ret["fields"][field_name] = field_info;
        ret["fields"][field_name]["writable"] = true;
    }

    for(ReadableStateFieldBase* rf : r.readable_fields) {
        const std::string& field_name = rf->name();
        if (ret["fields"].find(field_name) != ret["fields"].end()) continue;
        json field_info;
        get_readable_field_info(rf);
        ret["fields"][field_name] = field_info;
        ret["fields"][field_name]["writable"] = false;
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
