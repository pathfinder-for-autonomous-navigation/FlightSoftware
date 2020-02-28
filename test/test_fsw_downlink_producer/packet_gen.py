# Scratch space for generating manual packets for downlink testing

# Integer bitstrings for 400 and 20, in BIG endian
int_400 = "00000000000000000000000110010000"[::-1] # Used for state field value
int_20  = "00000000000000000000000000010100"[::-1] # Used for cycle count

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

print("-----------------------------------")
print("Expected value for test_task_initialization")
packet = packet_header(int_20) + flow("1", [])
print(packet)
packet = convert_to_bytes(packet)
print(packet)
print("Length: " + str(packet.count("\\x")))
print("-----------------------------------")

print("-----------------------------------")
print("Expected value for test_one_flow #1")
packet = packet_header(int_20) + flow("1", [int_400])
print(packet)
packet = convert_to_bytes(packet)
print(packet)
print("Length: " + str(packet.count("\\x")))
print("-----------------------------------")

print("Expected value for test_one_flow #2")
packet = packet_header(int_20) + flow("1", [int_400] * 5)
print(packet)
packet = convert_to_bytes(packet)
print(packet)
print("Length: " + str(packet.count("\\x")))
print("-----------------------------------")


print("Expected value for test_multiple_flows #1")
packet = packet_header(int_20) + flow("01", [int_400]) + flow("10", [int_400])
print(packet)
packet = convert_to_bytes(packet)
print(packet)
print("Length: " + str(packet.count("\\x")))
print("-----------------------------------")

print("Expected value for test_multiple_flows #2")
packet = packet_header(int_20) + flow("01", [int_400]*3) + flow("10", [int_400]*7) + flow("11", [int_400]*7)
packet = packet[0:560] + "0" + packet[560:-1] # Add packet delimiter
print(packet)
packet = convert_to_bytes(packet)
print(packet)
print("Length: " + str(packet.count("\\x")))
print("-----------------------------------")

print("Expected value for test_some_flows_inactive")
packet = packet_header(int_20) + flow("01", [int_400]) + flow("10", [int_400])
print(packet)
packet = convert_to_bytes(packet)
print(packet)
print("Length: " + str(packet.count("\\x")))
print("-----------------------------------")

int_800 = "00000000000000000000001100100000"[::-1]
print("-----------------------------------")
print("Expected value for test_downlink_change")
packet = packet_header(int_20) + flow("1", [int_800])
print(packet)
packet = convert_to_bytes(packet)
print(packet)
print("Length: " + str(packet.count("\\x")))
print("-----------------------------------")
