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
};

#endif
