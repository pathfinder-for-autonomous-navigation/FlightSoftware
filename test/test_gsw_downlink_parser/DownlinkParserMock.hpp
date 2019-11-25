#ifndef DOWNLINK_PARSER_MOCK_HPP_
#define DOWNLINK_PARSER_MOCK_HPP_

#include "../../src/GroundCode/DownlinkParser.hpp"

class DownlinkParserMock : public DownlinkParser {
  public:
    DownlinkParserMock(StateFieldRegistry& r,
        const std::vector<DownlinkProducer::FlowData>& flow_data) :
            DownlinkParser(r, flow_data) {}

    DownlinkProducer* get_downlink_producer() {
        return this->fcp.get_downlink_producer();
    }

    std::string process_downlink_packet(char* packet, size_t len) {
        // std::vector<char> p(len);
        // for(size_t i = 0; i < len; i++) p[i] = packet[i];
        // this->process_downlink_packet(p);
        return "";
    }
};

#endif
