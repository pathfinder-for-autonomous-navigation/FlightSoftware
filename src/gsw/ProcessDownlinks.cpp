#include <boost/python.hpp>
#include "DownlinkParser.hpp"

BOOST_PYTHON_MODULE(process_downlinks)
{
    using namespace boost::python;
    class_<DownlinkParser>("DownlinkParser", init<StateFieldRegistry, const std::vector<DownlinkProducer::FlowData>>())
        .def("process_downlink_file", &DownlinkParser::process_downlink_file)
        .def("process_downlink_packet", &DownlinkParser::process_downlink_packet)
        .def_readwrite("most_recent_frame", &DownlinkParser::most_recent_frame)
    ;
}