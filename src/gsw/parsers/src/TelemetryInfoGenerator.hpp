#ifndef FIELD_INFO_HPP_
#define FIELD_INFO_HPP_

#include <json.hpp>
#include <string>
#include <common/StateFieldRegistry.hpp>
#include <fsw/FCCode/MainControlLoop.hpp>

/**
 * @brief Generates a JSON file describing the minimum,
 * maximum, and bitsize values of each readable and
 * writable field in the state field registry.
 */
class TelemetryInfoGenerator {
  public:
    /**
     * @brief Construct a new telemetry info generator
     */
    TelemetryInfoGenerator(const std::vector<DownlinkProducer::FlowData>& _flow_data);

    struct FieldData
    {
      std::string name;
      std::string type;
      unsigned char flow_id = 0;
      bool writable = false;
      std::string min;
      std::string max;
      unsigned int bitsize = 0;
    };

    struct TelemetryInfo
    {
      std::map<std::string, unsigned int> eeprom_saved_fields;
      std::map<std::string, FieldData> field_data;
      std::vector<DownlinkProducer::FlowData> flow_data;
    };

    TelemetryInfo generate_telemetry_info();
    nlohmann::json generate_telemetry_info_json();

  private:
    StateFieldRegistry r;
    MainControlLoop fcp;
    const std::vector<DownlinkProducer::FlowData>& flow_data;
};

void to_json(nlohmann::json& j, const TelemetryInfoGenerator::FieldData& d);
void from_json(const nlohmann::json& j, TelemetryInfoGenerator::FieldData& d);
void to_json(nlohmann::json& j, const DownlinkProducer::FlowData& d);
void from_json(const nlohmann::json& j, DownlinkProducer::FlowData& d);

/**
 * @brief Generates telemetry data in JSON format.
 * 
 * @return json Formatted as follows:
 * 
 * {
 *    "fields" : {
 *      "fieldname1" : {
 *        "flow_id" : 1,
 *        "writable" : true,
 *        "min" : 0,
 *        "max" : 100,
 *        "bitsize" : 10
 *      }
 *    }
 *    "flows" : [{
 *      "id" : 1,
 *      "priority" : 0,
 *      "active" : true,
 *      "fields" : ["fieldname1"]
 *    }]
 * }
 * 
 */
void to_json(nlohmann::json& j, const TelemetryInfoGenerator::TelemetryInfo& d);
void from_json(const nlohmann::json& j, TelemetryInfoGenerator::TelemetryInfo& d);

#endif
