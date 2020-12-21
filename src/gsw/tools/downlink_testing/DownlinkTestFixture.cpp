#include "DownlinkTestFixture.hpp"
#include <sstream>
#include <fstream>
#include <algorithm>
#include <random>

DownlinkTestFixture::DownlinkTestFixture(const TelemetryInfoGenerator::TelemetryInfo& data) : test_data(data)
{
    create_state_fields();
    downlink_parser = std::make_unique<DownlinkParser>(registry, data.flow_data);
    snapshot_ptr_fp = registry.find_internal_field_t<char*>("downlink.ptr");
    snapshot_size_bytes_fp = registry.find_internal_field_t<size_t>(
                                "downlink.snap_size");
}

void DownlinkTestFixture::parse(const DownlinkTestFixture::test_input_t& input,
    DownlinkTestFixture::test_output_t& output)
{
    apply_input(input);
    auto cycle_start_fp = registry.find_writable_field_t<bool>("cycle.start");
    cycle_start_fp->set(true);
    downlink_parser->fcp.execute();

    char* snapshot = snapshot_ptr_fp->get();
    std::string snapshot_str(snapshot);
    output.raw_packet = std::vector<char>(snapshot_str.begin(), snapshot_str.end());

    DownlinkParser::DownlinkData processing_output = downlink_parser->process_downlink_packet(output.raw_packet);
    output.values = processing_output.field_data;
}

void DownlinkTestFixture::generate_test_input(DownlinkTestFixture::test_input_t& input) const
{
    for(auto const& field : test_data.field_data)
    {
        const TelemetryInfoGenerator::FieldData& f = field.second;

        std::string val;
        if (f.type == "unsigned int" || f.type == "unsigned char")
            val = std::to_string(std::stoul(f.min) + ( std::rand() % ( std::stoul(f.max) - std::stoul(f.min) + 1 ) ));
        if (f.type == "signed int" || f.type == "signed char")
            val = std::to_string(std::stoi(f.min) + ( std::rand() % ( std::stoi(f.max) - std::stoi(f.min) + 1 ) ));
        else if (f.type == "float")
            val = std::to_string(std::stof(f.min) + std::rand() * (std::stof(f.max) - std::stof(f.min)) / RAND_MAX);
        else if (f.type == "double")
            val = std::to_string(std::stod(f.min) + std::rand() * (std::stod(f.max) - std::stod(f.min)) / RAND_MAX);
        else if (f.type == "bool")
            val = std::rand() % 2 == 0 ? "true" : "false";
        else if (f.type == "gps_time_t") {
            unsigned short int wn = 2000 + std::rand() % 1000;
            unsigned int tow = std::rand();
            unsigned int ns = std::rand();
            std::stringstream gps_str;
            gps_str << wn << "," << tow << "," << ns;
            val = gps_str.str();
        }

        // TODO implement vector and quaternion autogenerators
        else if (f.type == "lin float vector") continue;
        else if (f.type == "lin double vector") continue;
        else if (f.type == "lin float quaternion") continue;
        else if (f.type == "lin double quaternion") continue;

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
    for(int i = 0; i < num_flows; i++)
    {
        bool flow_active = std::rand() % 2 == 0;
        info.flow_data.push_back({static_cast<unsigned char>(i+1), flow_active, {}});
    }

    // TODO add vector and quaternion types to this vector
    std::vector<std::string> datatypes = 
        {"unsigned int", "signed int", "unsigned char", "signed char", "float", "double", "gps_time_t", "bool"}; 

    /**
     * Generates a field, randomly choosing the field's datatypes,
     * bounds, bitsizes, and flow.
     */
    for(int i = 0; i < num_fields; i++)
    {
        TelemetryInfoGenerator::FieldData data;

        data.name = "field" + std::to_string(i);
        data.type = datatypes[std::rand() % datatypes.size()];

        if (data.type == "unsigned int")
        {
            std::function<unsigned int()> randint = []() { return std::rand(); };
            generate_random_bounds<unsigned int>(randint, data);
            data.bitsize = 1 + std::rand() % 31;
        }
        else if (data.type == "signed int")
        {
            std::function<signed int()> randint = []() {
                int x = std::rand(); int y = std::rand();
                return x * ((y % 2 == 0) ? (-1) : 1);
            };
            generate_random_bounds<signed int>(randint, data);
            data.bitsize = 1 + std::rand() % 31;
        }
        else if (data.type == "unsigned char")
        {
            std::function<unsigned char()> randint = []() { return std::rand() % 256; };
            generate_random_bounds<unsigned char>(randint, data);
            data.bitsize = 1 + std::rand() % 7;
        }
        else if (data.type == "signed char")
        {
            std::function<signed char()> randint = []() { return -256 + std::rand() % 256; };
            generate_random_bounds<signed char>(randint, data);
            data.bitsize = 1 + std::rand() % 7;
        }
        else if (data.type == "float")
        {
            static constexpr float float_min = -1e9;
            static constexpr float float_max = 1e9;
            std::function<float()> randfloat = []() {
                return float_min + static_cast<float>(rand()) /( static_cast<float>(RAND_MAX/(float_max-float_min)));
            };
            generate_random_bounds<float>(randfloat, data);
            data.bitsize = 1 + std::rand() % 31;
        }
        else if (data.type == "double")
        {
            static constexpr double double_min = -1e9;
            static constexpr double double_max = 1e9;
            std::function<double()> randdouble = []() {
                return double_min + static_cast<double>(rand()) /( static_cast<double>(RAND_MAX/(double_max-double_min)));
            };
            generate_random_bounds<double>(randdouble, data);
            data.bitsize = 1 + std::rand() % 63;
        }
        else if (data.type == "bool") data.bitsize = 1;
        else if (data.type == "gps_time_t") data.bitsize = 68;

        // Assign a flow.
        std::vector<int> flow_ids; for(int i = 1; i <= num_flows; i++) flow_ids.push_back(i);
        auto rng = std::default_random_engine{}; std::shuffle(flow_ids.begin(),flow_ids.end(), rng);
        for(int flow_id : flow_ids)
        {
            size_t flow_size = Serializer<signed char>::log2i(num_flows);
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

        #define create_field(strtype, boundtype, fieldtype, create_field_fn, stdfn) \
            if(f.type == strtype) \
            { \
                boundtype min = stdfn(f.min); \
                boundtype max = stdfn(f.max); \
                registry.create_field_fn<fieldtype>(f.name, min, max, f.bitsize); \
            }
        #define create_field_boundless(strtype, fieldtype, create_field_fn) \
            if(f.type == strtype) \
            { \
                registry.create_field_fn<fieldtype>(f.name); \
            }
        #define create_readable_field(strtype, boundtype, fieldtype, stdfn) \
            create_field(strtype, boundtype, fieldtype, create_readable_field, stdfn)
        #define create_readable_lin_vector_field(strtype, boundtype, stdfn) \
            create_field(strtype, boundtype, boundtype, create_readable_lin_vector_field, stdfn)
        #define create_readable_field_boundless(strtype, fieldtype) \
            create_field_boundless(strtype, fieldtype, create_readable_field)

        #define create(fieldtype) \
            create_##fieldtype##_field("unsigned int", unsigned int, unsigned int, std::stol); \
            create_##fieldtype##_field("unsigned char", unsigned char, unsigned char, std::stol); \
            create_##fieldtype##_field("signed int", signed int, signed int, std::stol); \
            create_##fieldtype##_field("signed char", signed char, signed char, std::stol); \
            create_##fieldtype##_field("float", float, float, std::stof); \
            create_##fieldtype##_field("double", double, double, std::stod); \
            create_##fieldtype##_lin_vector_field("lin float vector", double, std::stof); \
            create_##fieldtype##_lin_vector_field("lin double vector", double, std::stod); \
            create_##fieldtype##_field_boundless("lin float quaternion", lin::Vector4f); \
            create_##fieldtype##_field_boundless("lin double quaternion", lin::Vector4d); \
            create_##fieldtype##_field_boundless("bool", bool); \
            create_##fieldtype##_field_boundless("gps_time_t", gps_time_t);

        create(readable);
        
        #undef create
        #undef create_readable_field_boundless
        #undef create_readable_field
        #undef create_field_boundless
        #undef create_field
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
