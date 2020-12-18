#include "DownlinkTestFixture.hpp"

DownlinkTestFixture::DownlinkTestFixture(const TelemetryInfoGenerator::TelemetryInfo& data) : test_data(data)
{
    // Create required field(s)
    cycle_count_fp = registry.create_readable_field<unsigned int>("pan.cycle_no");
    create_state_fields();

    downlink_producer = std::make_unique<DownlinkProducer>(registry, 0);
    downlink_producer->init_flows(data.flow_data);
    snapshot_ptr_fp = registry.find_internal_field_t<char*>("downlink.ptr");
    snapshot_size_bytes_fp = registry.find_internal_field_t<size_t>(
                                "downlink.snap_size");

    downlink_parser = std::make_unique<DownlinkParser>(registry, data.flow_data);
}

void DownlinkTestFixture::parse(const DownlinkTestFixture::test_input_t& input,
    DownlinkTestFixture::test_output_t& output)
{
    apply_input(input);
    downlink_producer->execute();

    char* snapshot = snapshot_ptr_fp->get();
    std::string snapshot_str(snapshot);
    std::vector<char> packet(snapshot_str.begin(), snapshot_str.end());
    DownlinkParser::DownlinkData processing_output = downlink_parser->process_downlink_packet(packet);
}

void DownlinkTestFixture::save_test_data(
      const test_input_t& input,
      const test_output_t& output,
      const test_result_t& errors,
      const std::string& path) const
{
    
}

template<typename T>
std::string get_value(T min, T max)
{    
    if (std::is_integral<T>::value)
        return std::to_string(min + ( std::rand() % ( max - min + 1 ) ));
    else if (std::is_floating_point<T>::value)
        return std::to_string(min + std::rand() * (max - min) / RAND_MAX);
    else if (std::is_same<T, bool>::value)
        return std::rand() % 2 == 0 ? "true" : "false";
}

void DownlinkTestFixture::generate_test_input(DownlinkTestFixture::test_input_t& input) const
{
    for(auto const& field : test_data.field_data)
    {
        const TelemetryInfoGenerator::FieldData& f = field.second;

        std::string val;
        if (f.type == "unsigned int") val = get_value<unsigned int>(std::stoul(f.min), std::stoul(f.max));
        if (f.type == "signed int") val = get_value<signed int>(std::stoi(f.min), std::stoi(f.max));
        if (f.type == "unsigned char") val = get_value<unsigned char>(std::stoul(f.min), std::stoul(f.max));
        if (f.type == "signed char") val = get_value<signed char>(std::stoi(f.min), std::stoi(f.max));
        else if (f.type == "float") val = get_value<signed char>(std::stof(f.min), std::stof(f.max));
        else if (f.type == "double") val = get_value<signed char>(std::stod(f.min), std::stod(f.max));
        else if (f.type == "gps_time_t") val = "";
        else if (f.type == "bool") val = get_value<bool>(false, false);

        // TODO implement vector and quaternion autogenerators
        else if (f.type == "lin float vector") continue;
        else if (f.type == "lin double vector") continue;
        else if (f.type == "lin float quaternion") continue;
        else if (f.type == "lin double quaternion") continue;

        input.insert({f.name, val});
    }
}

template<typename T>
void generate_random_bounds(std::function<T()>& randomfn, TelemetryInfoGenerator::FieldData& data)
{
    T min = randomfn();
    T max;
    do { max = randomfn(); } while(max <= min);
    data.min = std::to_string(min);
    data.max = std::to_string(max);
}

void DownlinkTestFixture::generate_telemetry_info(TelemetryInfoGenerator::TelemetryInfo& info)
{
    static constexpr unsigned int field_limit = 200;
    static constexpr unsigned int flow_limit = 20;
    unsigned int num_fields = std::rand() % field_limit + 1;
    unsigned int num_flows = std::rand() % flow_limit + 1;

    for(int i = 0; i < num_flows; i++)
        info.flow_data.push_back({static_cast<unsigned char>(i+1), true, {}});

    std::vector<std::string> datatypes = 
        {"unsigned int", "signed int", "unsigned char", "signed char", "float", "double", "gps_time_t", "bool"};
    // TODO add vector and quaternion types

    for(int i = 0; i < num_fields; i++)
    {
        TelemetryInfoGenerator::FieldData data;

        data.name = "field" + std::to_string(i);
        data.type = datatypes[std::rand() & datatypes.size()];

        if (data.type == "unsigned int")
        {
            std::function<unsigned int()> randint = []() { return std::rand(); };
            generate_random_bounds<unsigned int>(randint, data);
            data.bitsize = std::rand() % 32;
        }
        else if (data.type == "signed int")
        {
            std::function<signed int()> randint = []() {
                int x = std::rand(); int y = std::rand();
                return x * ((y % 2 == 0) ? (-1) : 1);
            };
            generate_random_bounds<signed int>(randint, data);
            data.bitsize = std::rand() % 32;
        }
        else if (data.type == "unsigned char")
        {
            std::function<unsigned char()> randint = []() { return std::rand() % 256; };
            generate_random_bounds<unsigned char>(randint, data);
            data.bitsize = std::rand() % 8;
        }
        else if (data.type == "signed char")
        {
            std::function<signed char()> randint = []() { return -256 + std::rand() % 256; };
            generate_random_bounds<signed char>(randint, data);
            data.bitsize = std::rand() % 8;
        }
        else if (data.type == "float")
        {
            static constexpr float float_min = -1e9;
            static constexpr float float_max = 1e9;
            std::function<float()> randfloat = []() {
                return float_min + static_cast<float>(rand()) /( static_cast<float>(RAND_MAX/(float_max-float_min)));
            };
            generate_random_bounds<float>(randfloat, data);
            data.bitsize = std::rand() % 32;
        }
        else if (data.type == "double")
        {
            static constexpr double double_min = -1e9;
            static constexpr double double_max = 1e9;
            std::function<double()> randdouble = []() {
                return double_min + static_cast<double>(rand()) /( static_cast<double>(RAND_MAX/(double_max-double_min)));
            };
            generate_random_bounds<double>(randdouble, data);
            data.bitsize = std::rand() % 64;
        }
        else if (data.type == "bool") data.bitsize = 1;
        else if (data.type == "gps_time_t") data.bitsize = 68;

        data.flow_id = std::rand() % num_flows + 1;

        info.flow_data[data.flow_id - 1].field_list.push_back(data.name);
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
            if(f.type == #strtype) \
            { \
                registry.create_field_fn<fieldtype>(f.name); \
            }
        #define create_readable_field(strtype, boundtype, fieldtype, stdfn) \
            create_field(strtype, boundtype, fieldtype, create_readable_field, stdfn)
        #define create_writable_field(strtype, boundtype, fieldtype, stdfn) \
            create_field(strtype, boundtype, fieldtype, create_writable_field, stdfn)
        #define create_readable_lin_vector_field(strtype, boundtype, stdfn) \
            create_field(strtype, boundtype, boundtype, create_readable_lin_vector_field, stdfn)
        #define create_writable_lin_vector_field(strtype, boundtype, stdfn) \
            create_field(strtype, boundtype, boundtype, create_writable_lin_vector_field, stdfn)
        #define create_readable_field_boundless(strtype, fieldtype) \
            create_field_boundless(strtype, fieldtype, create_readable_field)
        #define create_writable_field_boundless(strtype, fieldtype) \
            create_field_boundless(strtype, fieldtype, create_writable_field)

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

        if (f.writable)
        {
            create(writable);
        }
        else
        {
            create(readable);
        }
        
        #undef create
        #undef create_writable_field_boundless
        #undef create_readable_field_boundless
        #undef create_writable_field
        #undef create_readable_field
        #undef create_field_boundless
        #undef create_field
    }
}

void DownlinkTestFixture::compare(const DownlinkTestFixture::test_input_t& input,
    const DownlinkTestFixture::test_output_t& output,
    DownlinkTestFixture::test_result_t& result) const
{
    for(auto const& input_field : input)
    {
        std::string name = input_field.first;
        std::string input_valstr = input_field.second;
        const TelemetryInfoGenerator::FieldData& data = test_data.field_data.at(name);

        if (output.find(name) == output.end())
        {
            result.errors.insert({name, {input_valstr,"",-1}});
            continue;
        }

        std::string output_valstr = output.at(name);
        
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
            unsigned int num_intervals = (0b1 << data.bitsize) - 1;
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

    for(auto const& output_field : output)
    {
        std::string name = output_field.first;
        std::string output_valstr = output_field.second;
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
