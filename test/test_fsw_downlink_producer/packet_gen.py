# Scratch space for generating manual packets for downlink testing

int_400 = "00000000000000000000000110010000" # Used for state field value
int_20  = "00000000000000000000000000010100" # Used for cycle count

def convert_to_bytes(packet):
	packet += "0"*(8 - len(packet) % 8)

	packet_str = int(packet,2).to_bytes(len(packet) // 8, byteorder='big')
	hex_str = "0x" + ', 0x'.join('{:02x}'.format(x) for x in packet_str)
	return hex_str

packet = "1" + int_20
print(convert_to_bytes(packet))
