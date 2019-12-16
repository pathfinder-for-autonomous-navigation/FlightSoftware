# Scratch space for generating manual packets for downlink testing

# Integer bitstrings for 400 and 20, in BIG endian
int_400 = "00000000000000000000000110010000"[::-1] # Used for state field value
int_20  = "00000000000000000000000000010100"[::-1] # Used for cycle count

def convert_to_bytes(packet):
	packet += "0"*(8 - len(packet) % 8)
	packet_str = int(packet,2).to_bytes(len(packet) // 8, byteorder='big')
	hex_str = "'\\x" + "', '\\x".join('{:02x}'.format(x) for x in packet_str) + "'"
	return hex_str

print("-----------------------------------")
print("Expected value for test_task_initialization")
packet = "1" + int_20 + "1"
print(packet)
packet = convert_to_bytes(packet)
print(packet)
print("Length: " + str(packet.count("\\x")))
print("-----------------------------------")

print("-----------------------------------")
print("Expected value for test_one_flow #1")
packet = "1" + int_20 + "1" + int_400
print(packet)
packet = convert_to_bytes(packet)
print(packet)
print("Length: " + str(packet.count("\\x")))
print("-----------------------------------")

print("Expected value for test_one_flow #2")
packet = "1" + int_20 + "1" + int_400 * 5
print(packet)
packet = convert_to_bytes(packet)
print(packet)
print("Length: " + str(packet.count("\\x")))
print("-----------------------------------")


print("Expected value for test_multiple_flows #1")
packet = "1" + int_20 + "01"[::-1] + int_400 + "10"[::-1] + int_400
print(packet)
packet = convert_to_bytes(packet)
print(packet)
print("Length: " + str(packet.count("\\x")))
print("-----------------------------------")

print("Expected value for test_multiple_flows #2")
packet = "1" + int_20 + "01"[::-1] + int_400*3 + "10"[::-1] + int_400*7 + "11"[::-1] + int_400*7
packet = packet[0:560] + "0" + packet[560:-1] # Add packet delimiter
print(packet)
packet = convert_to_bytes(packet)
print(packet)
print("Length: " + str(packet.count("\\x")))
print("-----------------------------------")

print("Expected value for test_some_flows_inactive")
packet = "1" + int_20 + "01"[::-1] + int_400 + "10"[::-1] + int_400 # Add packet delimiter
print(packet)
packet = convert_to_bytes(packet)
print(packet)
print("Length: " + str(packet.count("\\x")))
print("-----------------------------------")

int_800 = "00000000000000000000001100100000"[::-1]
print("-----------------------------------")
print("Expected value for test_downlink_change")
packet = "1" + int_20 + "1" + int_800
print(packet)
packet = convert_to_bytes(packet)
print(packet)
print("Length: " + str(packet.count("\\x")))
print("-----------------------------------")

int_0 = "0"*32
print("-----------------------------------")
print("Expected value for ground software functional testing")
packet = "1" + int_0 + ("01"[::-1] + "0"*4 + "0" + "00") + ("10"[::-1] + "0" + "0" + "0")
print(packet)
packet = convert_to_bytes(packet)
print(packet)
print("Length: " + str(packet.count("0x")))
print("-----------------------------------")
