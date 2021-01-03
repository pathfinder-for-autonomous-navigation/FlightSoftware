#include "DownlinkTestFixture.hpp"
#include <sstream>
#include <fstream>
#include <algorithm>
#include <random>

#define loop_over_types(fn) \
    fn("unsigned int", unsigned int) \
    fn("signed int", signed int) \
    fn("unsigned char", unsigned char) \
    fn("signed char", signed char) \
    fn("float", float) \
    fn("double", double) \
    fn("bool", bool) \
    fn("gps_time_t", gps_time_t) \
    fn("std float vector", f_vector_t) \
    fn("std double vector", d_vector_t) \
    fn("lin float vector", lin::Vector3f) \
    fn("lin double vector", lin::Vector3d) \
    fn("std float quaternion", f_quat_t) \
    fn("std double quaternion", d_quat_t) \
    fn("lin float quaternion", lin::Vector4f) \
    fn("lin double quaternion", lin::Vector4d)

DownlinkTestFixture::DownlinkTestFixture(const TelemetryInfoGenerator::TelemetryInfo& data) : test_data(data)
{
    create_state_fields();
    downlink_producer = std::make_unique<DownlinkProducer>(registry, 0);
    downlink_producer->init_flows(data.flow_data);
    downlink_parser = std::make_unique<DownlinkParser>(registry, downlink_producer->get_flows());
}

void DownlinkTestFixture::parse(const DownlinkTestFixture::test_input_t& input,
    DownlinkTestFixture::test_output_t& output)
{
    apply_input(input);
    downlink_producer->execute();

    char* snapshot_chars = registry.find_internal_field_t<char*>("downlink.ptr")->get();
    size_t snapshot_size = registry.find_internal_field_t<size_t>("downlink.snap_size")->get();
    std::vector<char> snapshot(snapshot_size);
    for(size_t i = 0; i < snapshot_size; i++) snapshot[i] = snapshot_chars[i];
    
    downlink_parser->process_downlink_packet(snapshot);
    std::vector<char> empty_packet{char(0x80), char(0x00), char(0x00), char(0x00), char(0x00)};
    DownlinkParser::DownlinkData parser_output = downlink_parser->process_downlink_packet(empty_packet);
    output.values = parser_output.field_data;
}

void DownlinkTestFixture::generate_test_input(DownlinkTestFixture::test_input_t& input) const
{
    for(auto const& field : test_data.field_data)
    {
        const TelemetryInfoGenerator::FieldData& f = field.second;

        std::string val;

        #define type_based_generation(strtype, realtype) \
            if (f.type == strtype) generate_test_input<realtype>(f);
        loop_over_types(type_based_generation);
        #undef type_based_generation

        input.insert({f.name, val});
    }
}

/**
 * Helper function to randomly generate bounds for telemetry metadata.
 */
template<typename T>
void generate_random_bounds(std::function<T()>& randomfn, TelemetryInfoGenerator::FieldData& data)
{
    T min = 0;
    T max = 0;
    do {
        min = randomfn();
        max = randomfn();
    } while(max <= min);
    data.min = std::to_string(min);
    data.max = std::to_string(max);
}

void DownlinkTestFixture::generate_telemetry_info(TelemetryInfoGenerator::TelemetryInfo& info)
{
    static constexpr unsigned int field_limit = 200;
    static constexpr unsigned int flow_limit = 20;

    /**
     * Set maximum sizes for this particular telemetry information.
     */
    unsigned int num_fields = 1 + std::rand() % field_limit;
    unsigned int num_flows = 1 + std::rand() % flow_limit;

    /**
     * Create the flows and randomly assign them an activity level.
     */
    for(size_t i = 0; i < num_flows; i++)
    {
        bool flow_active = std::rand() % 2 == 0;
        info.flow_data.push_back({static_cast<unsigned char>(i+1), flow_active, {}});
    }

    // TODO add vector and quaternion types to this vector
    std::vector<std::string> datatypes = 
        {"unsigned int", "signed int", "unsigned char", "signed char", "float", "double", "gps_time_t", "bool",
         "lin float vector", "lin double vector", "lin float quaternion", "lin double quaternion",
         "std float vector", "std double vector", "std float quaternion", "std double quaternion"}; 

    info.field_data.insert({
        "pan.cycle_no",
        {"pan.cycle_no", "unsigned int", 0, false, "0", "4294967295", 32}
    });

    /**
     * Generates a field, randomly choosing the field's datatypes,
     * bounds, bitsizes, and flow.
     */
    for(size_t i = 0; i < num_fields; i++)
    {
        TelemetryInfoGenerator::FieldData data;

        data.name = "field" + std::to_string(i);
        data.type = datatypes[std::rand() % datatypes.size()];

        #define type_based_telem_info_generation(strtype, realtype) \
            if (data.type == strtype) generate_telemetry_info<realtype>(data);
        
        loop_over_types(type_based_telem_info_generation)

        #undef type_based_telem_info_generation

        // Assign a flow.
        std::vector<int> flow_ids; for(size_t i = 1; i <= num_flows; i++) flow_ids.push_back(i);
        auto rng = std::default_random_engine{}; std::shuffle(flow_ids.begin(),flow_ids.end(), rng);
        for(int flow_id : flow_ids)
        {
            size_t flow_id_size = std::ceil(std::log(num_flows) / std::log(2)); // Number of bits required to represent flow ID

            size_t flow_size = flow_id_size;
            for(const std::string& field : info.flow_data[flow_id - 1].field_list)
                flow_size += info.field_data[field].bitsize;

            if (flow_size + data.bitsize <= DownlinkProducer::num_bits_in_packet - 1 - 32)
            {
                data.flow_id = flow_id;
                info.flow_data[flow_id - 1].field_list.push_back(data.name);
                break;
            }
        }

        info.field_data.insert({data.name, data});
    }
}

void DownlinkTestFixture::apply_input(const DownlinkTestFixture::test_input_t& input)
{
    for(auto const& inputs : input)
    {
        ReadableStateFieldBase* field = registry.find_readable_field(inputs.first);
        field->deserialize(inputs.second.c_str());
    }
}

void DownlinkTestFixture::create_state_fields()
{
    for(auto const& field : test_data.field_data)
    {
        const TelemetryInfoGenerator::FieldData& f = field.second;

        #define type_based_creation(strtype, realtype) \
            if (f.type == strtype) create_state_field<realtype>(f, registry);
        
        loop_over_types(type_based_creation)
        
        #undef type_based_creation
    }
}

void DownlinkTestFixture::compare(const DownlinkTestFixture::test_input_t& input,
    const DownlinkTestFixture::test_output_t& output,
    DownlinkTestFixture::test_result_t& result) const
{
    /*
     * Iterate over all fields in the input set and check:
     * - Field is present in the output set if and only if the flow that the
     *   field belongs to is active.
     * - The input value of the field matches the output value of the field up
     *   to a tolerance specified by the bounds and bitsize.
     */
    for(auto const& input_field : input)
    {
        const std::string& name = input_field.first;
        const std::string& input_valstr = input_field.second;
        const TelemetryInfoGenerator::FieldData& data = test_data.field_data.at(name);
        if (data.flow_id == 0) continue;
        const DownlinkProducer::FlowData flow_data = test_data.flow_data[data.flow_id-1];

        /*
         * Check field is present in the output if and only if the flow
         * that the field belongs to is active. Report an error otherwise.
         */
        if (output.values.find(name) == output.values.end() && !flow_data.is_active) continue;
        if (output.values.find(name) == output.values.end() && flow_data.is_active)
        {
            result.errors.insert({name, {input_valstr,"",-1}});
            continue;
        }
        if (output.values.find(name) != output.values.end() && !flow_data.is_active)
        {
            const std::string& output_valstr = output.values.at(name);
            result.errors.insert({name, {"",output_valstr,-1}});
            continue;
        }

        const std::string& output_valstr = output.values.at(name);

        #define compare_vec(T, strfn) \
        { \
            T min = strfn(data.min); \
            T max = strfn(data.max); \
            T tolerance = (max - min) / std::pow(2, data.bitsize); \
            auto str_to_vec = [](const std::string& s) { \
                std::vector<std::string> result; \
                std::stringstream ss(s); \
                while( ss.good() ) \
                { \
                    std::string substr; \
                    getline( ss, substr, ',' ); \
                    result.push_back( substr ); \
                } \
                return lin::Vector<T, 3>({strfn(result[0]), strfn(result[1]), strfn(result[2])}); \
            }; \
            T err = lin::norm(str_to_vec(input_valstr) - str_to_vec(output_valstr)); \
            if (err >= tolerance) result.errors.insert({name, {input_valstr, output_valstr, tolerance}}); \
        }
        
        /*
         * Check the input value of the field matches the output value of the
         * field up to a tolerance specified by the bounds and bitsize.
         */
        if (data.type == "unsigned int" || data.type == "signed int"
            || data.type == "unsigned char" || data.type == "signed char")
        {
            long min = std::stol(data.min);
            long max = std::stol(data.max);
            double tolerance = Serializer<unsigned int>::resolution(min, max, data.bitsize);

            long input_val = std::stol(input_valstr);
            long output_val = std::stol(output_valstr);
            if (std::abs(output_val - input_val) > tolerance)
                result.errors.insert({name, {input_valstr, output_valstr, tolerance}});
        }
        else if (data.type == "float" || data.type == "double")
        {
            double min = std::stod(data.min);
            double max = std::stod(data.max);
            unsigned long long num_intervals = std::pow(2, data.bitsize) - 1;
            double tolerance = (max - min) / num_intervals;

            long input_val = std::stod(input_valstr);
            long output_val = std::stod(output_valstr);
            if (std::abs(output_val - input_val) > tolerance)
                result.errors.insert({name, {input_valstr, output_valstr, tolerance}});
        }
        else if (data.type == "bool" && input_valstr != output_valstr)
            result.errors.insert({name, {input_valstr, output_valstr, 0}});
        else if (data.type == "gps_time_t")
        {
            Serializer<gps_time_t> s;
            gps_time_t input_time, output_time;
            s.deserialize(input_valstr.c_str(), &input_time);
            s.deserialize(output_valstr.c_str(), &output_time);

            bool error = false;
            // Check if there is greater than 1 ms error in the GPS time reporting.
            error |= input_time > output_time && static_cast<unsigned long long>(input_time - output_time) > 1000;
            error |= output_time > input_time && static_cast<unsigned long long>(output_time - input_time) > 1000;
            if (error)
                result.errors.insert({name, {input_valstr, output_valstr, 1e-6}});
        }
        else if (data.type == "lin float vector" || data.type == "std float vector")
        {
            compare_vec(float, std::stof);
        }
        else if (data.type == "lin double vector" || data.type == "std double vector")
        {
            compare_vec(double, std::stod);
        }
        else if (data.type == "lin float quaternion"
                 || data.type == "std float quaternion"
                 || data.type == "lin double quaternion"
                 || data.type == "std double quaternion")
        {
            auto str_to_quat = [](const std::string& s) {
                std::vector<std::string> result;
                std::stringstream ss(s);
                while( ss.good() )
                {
                    std::string substr;
                    getline( ss, substr, ',' );
                    result.push_back( substr );
                }
                return lin::Vector<double, 4>({std::stod(result[0]), std::stod(result[1]), std::stod(result[2]), std::stod(result[3])});
            };
            double err = std::acos(std::abs(lin::dot(str_to_quat(input_valstr), str_to_quat(output_valstr))))*2.0;
            double tolerance = 1.0 * 2 * gnc::constant::pi / 360.0;
            if (err >= tolerance) result.errors.insert({name, {input_valstr, output_valstr, tolerance}});
        }

        #undef compare_vec
    }

    /*
     * Ensure that there are no fields in the output telemetry set that
     * were not in the input telemetry set.
     */
    for(auto const& output_field : output.values)
    {
        const std::string& name = output_field.first;
        const std::string& output_valstr = output_field.second;
        if (input.find(name) == input.end())
            result.errors.insert({name, {"",output_valstr,-1}});
    }

    if (result.errors.size() != 0) result.success = false;
    else result.success = true;
}

void to_json(nlohmann::json& j, const DownlinkTestFixture::test_error_t& e)
{
    j["expected"] = e.expected;
    j["actual"] = e.actual;
    j["tolerance"] = e.tolerance;
}
void from_json(const nlohmann::json& j, DownlinkTestFixture::test_error_t& e)
{
    e.expected = j["expected"].get<std::string>();
    e.actual = j["actual"].get<std::string>();
    e.tolerance = j["tolerance"].get<double>();
}

void to_json(nlohmann::json& j, const DownlinkTestFixture::test_result_t& e)
{
    j["success"] = e.success;
    j["errors"] = e.errors;
}
void from_json(const nlohmann::json& j, DownlinkTestFixture::test_result_t& e)
{
    e.success = j["success"].get<bool>();
    for(auto const& err : j["errors"].items())
    {
        e.errors.insert({err.key(), err.value().get<DownlinkTestFixture::test_error_t>()});
    }
}
