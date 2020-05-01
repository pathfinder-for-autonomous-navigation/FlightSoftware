import random

#Generate packets and flows 

random.seed()

# Helper functions for creating packets
def convert_to_bytes(packet):
	packet += "0"*(8 - len(packet) % 8)
	packet_str = int(packet,2).to_bytes(len(packet) // 8, byteorder='big')
	hex_str = "'\\x" + "', '\\x".join('{:02x}'.format(x) for x in packet_str) + "'"
	return hex_str
def packet_header(cc_count):
	return "1" + cc_count
def flow(flow_id, flow_items):
	return flow_id[::-1] + "".join(flow_items)


'''
Gets a random integer and returns a tuple with the bitstring in 
big endian and the decimal value of the integer. 
'''
def rand_int_string(bits):
    i = random.getrandbits(bits)
    bitstr_i = '{:0{bits}b}'.format(i, bits=bits)[::-1]
    return (bitstr_i[::-1], i)


'''
Print decimal values of fields for adding to test cases
'''
def pretty_print_flow_data(cycle_count_d, flows_d):
    s = 'Cycle count: ' + str(cycle_count_d) 
    s += '\n{\n'
    for flow in range(0, len(flows_d)):
        s += '\t{\n'
        s += '\t\t' + str(flow+1) + ',\n'
        s += '\t\t' + 'true' + ',\n'
        s += '\t\t' + '{\n'
        for field in flows_d[flow]:
            s += '\t\t' + '\t\"foo_' + str(field) + '\",\n'
        s += '\t\t' + '}\n'
        s += '\t},\n'
    s = s[:-2] + '\n}'
    print(s)


'''
Randomly make a test case with optional number of flows and 
range for number of fields per flow. 
'''
def test_x_rand_flows(num_flows=1, range_num_flow_fields=(1, 1)):
    flows_d = []
    cycle_count_b, cycle_count_d = rand_int_string(32)
    flow_id_bits = num_flows.bit_length()
    packet = packet_header(cycle_count_b)

    for flow_num in range(0, num_flows):
        num_flow_fields = random.randint(range_num_flow_fields[0], range_num_flow_fields[1]+1)
        flow_id = '{:0{bits}b}'.format(flow_num, bits=flow_id_bits)[::-1]
        flow_data = ""
        flow_data_d = []

        for _ in range(0, num_flow_fields):
            state_field_value_b, state_field_value_d = rand_int_string(32)
            flow_data += state_field_value_b
            flow_data_d.append(state_field_value_d)
        
        packet += flow(flow_id, flow_data)
        flows_d.append(flow_data_d)

    print('\nDecimal valued flow data for test case')
    print("-----------------------------------")
    pretty_print_flow_data(cycle_count_d, flows_d)
    print("-----------------------------------\n\n")

    
    print('\nBinary Packet')
    print("-----------------------------------")
    print(packet)
    print("-----------------------------------\n\n")


    packet_h = convert_to_bytes(packet)
    print('\nHexadecimal Packet')
    print("-----------------------------------")
    print(packet_h)
    print("-----------------------------------\n\n")
    
    print("Length: " + str(packet_h.count("\\x")))
    print("-----------------------------------")


test_x_rand_flows(3)

#TODO make manual case
