#ifndef DOWNLINK_PARSER_MOCK_HPP_
#define DOWNLINK_PARSER_MOCK_HPP_

#include <gsw/DownlinkParser.hpp>

#include <json.hpp>
using json = nlohmann::json;

/**
 * @brief Mocks the downlink parser so that it's easier to
 * unit-test.
 */
class DownlinkParserMock : public DownlinkParser {
  public:
    DownlinkParserMock(StateFieldRegistry& r,
        const std::vector<DownlinkProducer::FlowData>& flow_data) :
            DownlinkParser(r, flow_data) {}

    DownlinkProducer* get_downlink_producer() {
        return this->fcp.get_downlink_producer();
    }

    json process_downlink(const char* packet, const size_t len) {
        int len_cpy = len;
        while(len_cpy > 0) {
            std::vector<char> p(packet,
                packet + std::min(static_cast<unsigned int>(70),
                                  static_cast<unsigned int>(len))
            );
            process_downlink_packet(p);
            len_cpy -= 70;
        }
        const std::string result = process_downlink_packet(std::vector<char>({'\x80'}));
        return json::parse(result);
    }
};

#endif
