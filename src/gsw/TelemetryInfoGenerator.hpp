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
    nlohmann::json generate_telemetry_info();
  private:
    StateFieldRegistry r;
    MainControlLoop fcp;
    const std::vector<DownlinkProducer::FlowData>& flow_data;
};

#endif
