def convert_to_bytes(packet):
	packet += "0"*(8 - len(packet) % 8)
	packet_str = int(packet,2).to_bytes(len(packet) // 8, byteorder='big')
	hex_str = "'\\x" + "', '\\x".join('{:02x}'.format(x) for x in packet_str) + "'"
	return hex_str

# Helper functions for creating packets
def packet_header(cc_count):
	return "1" + cc_count
def flow(flow_id, flow_items):
	return flow_id[::-1] + "".join(flow_items)

int_0 = "0"*32
print("-----------------------------------")
print("Expected value for ground software functional testing")
packet = (
        packet_header(int_0) +
        ########### pan.state, pan.sat_designation
        flow("01", ["0"*4,     "00"               ]) +
        ########### docksys.docked, docksys.dock_config, docksys.is_turning 
        flow("10", ["0",            "0",                 "0"])
    )
print(packet)
packet = convert_to_bytes(packet)
print(packet)
print("Length: " + str(packet.count("\\x")))
print("-----------------------------------")
