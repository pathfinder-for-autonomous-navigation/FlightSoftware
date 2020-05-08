import random
import subprocess

#Generate packets and flows 

random.seed()
field_id = 0

# Helper functions for creating packets
def convert_to_bytes(packet):
    packet += "0"*(8 - len(packet) % 8)
    packet_str = int(packet,2).to_bytes(len(packet) // 8, byteorder='big')
    hex_str = "'\\x" + "', '\\x".join('{:02x}'.format(x) for x in packet_str) + "'"
    return hex_str
def packet_header(cc_count):
	return "1" + cc_count
def flow(flow_id, flow_items):
	return flow_id + "".join(flow_items)

'''
Serialize an individual value
'''
def serialize(data_type, value, min_val, max_val, bitsize):
    json_str = '''
    {{
        \"type\": \"{}\",
        \"value\": {},
        \"min\": {},
        \"max\": {},
        \"bitsize\": {}
    }}
    '''.format(data_type, value, min_val, max_val, bitsize)
    args = ("/Users/andreeafoarce/Desktop/PAN/FlightSoftware/.pio/build/tools_packet_generator/program", json_str) #all strings
    gen = subprocess.Popen(args, stdout=subprocess.PIPE)
    gen.wait()
    output = gen.stdout.read()
    return str(output).replace('\\n', '').replace('\'', '').replace('b', '')


'''
Initialize fields for tests
'''
def initializer(val_str, val, str_type):
    global field_id
    state_field_id = val_str + '_' + str(field_id)
    initializer = '''
    auto foo_{id}_fp = tf.registry.create_readable_field<{type}>("{id}");
    foo_{id}_fp->set({val});
    '''.format(val=val, id=state_field_id, type=str_type)
    field_id += 1
    return (initializer, state_field_id)

'''
Randomly generates a bool flow 
Returns its serialized bitstring and decimal value
'''
def bool_ser():
    i = random.getrandbits(1)
    s = 'true' if (i==1) else 'false'
    bitstr = serialize("bool", s, 0, 1, 1)
    return (bitstr, s, initializer(s, s, "bool"))

def unsigned_int_ser(min=0,max=4294967295,bitsize=32):
    i = random.getrandbits(bitsize) 
    bitstr = serialize("unsigned int", str(i), 0, 4294967295, 32)
    return (bitstr, i, initializer(str(i), str(i), "unsigned int"))

def unsigned_char_ser():
    i = random.getrandbits(8)
    bitstr = serialize("unsigned char", str(i), 0, 255, 8)
    return (bitstr, i, initializer(str(i), str(i), "unsigned char"))

def lin_Vector4d_ser(min=-100,max=100,bitsize=64): #TODO find good min/max vals, randomize
    v4 = []
    for _ in range(0, 4):
        v4.append(random.uniform(min, max)) 
    val_str = str(v4).replace('[', '\"').replace(']', '\"')
    bit_str = serialize("lin::Vector4d", val_str, min, max, bitsize)
    id_str = str(v4).replace(', ', '_').replace('-', 'n').replace('[', '').replace(']', '').replace('.', '_')
    return (bit_str, id_str, initializer(id_str, '{'+ val_str.replace('\"', '') + '}', "lin::Vector4d"))

data_serializers = [bool_ser, unsigned_int_ser, unsigned_char_ser, lin_Vector4d_ser]
#data_serializers = [bool_ser]


'''
Print decimal values of fields for adding to test cases
'''
def pretty_print_flow_data(packet, cycle_count_d, flows_d, state_field_initializers):
    packet_h = convert_to_bytes(packet)
    length = str(packet_h.count("\\x"))

    s = '''
    {{
    TestFixture tf;
    {}
    std::vector<DownlinkProducer::FlowData> flow_data = {{
    '''.format(''.join([tup[0] for row in state_field_initializers for tup in row]))

    for flow in range(0, len(flows_d)):
        s += '\t{\n'
        s += '\t\t' + str(flow+1) + ',\n'
        s += '\t\t' + 'true' + ',\n'
        s += '\t\t' + '{\n'
        for f in range(0, len(flows_d[flow])):
            field = state_field_initializers[flow][f][1]
            s += '\t\t' + '\t\"foo_' + str(field) + '\",\n'
        s += '\t\t' + '}\n'
        s += '\t},\n'
    s = s[:-2] + '\n'
    s += '''
    }};
    tf.init(flow_data);
    tf.cycle_count_fp->set({cycle_count});

    TEST_ASSERT_EQUAL({length}, tf.snapshot_size_bytes_fp->get());
    tf.downlink_producer->execute();
    const char expected_outputs[{length}] = {{ {hex_packet} }};
    TEST_ASSERT_EQUAL_MEMORY(expected_outputs, tf.snapshot_ptr_fp->get(), {length});

    }}
    '''.format(cycle_count=cycle_count_d, length=length, hex_packet=packet_h)
    print(s)

def print_output(packet, cycle_count_d, flows_d, state_field_initializers):
    print('\nDecimal valued flow data for test case')
    print("-----------------------------------")
    pretty_print_flow_data(packet, cycle_count_d, flows_d, state_field_initializers)
    print("-----------------------------------\n\n")

    
    print('\nBinary Packet')
    print("-----------------------------------")
    print(packet)
    print("-----------------------------------\n\n")

'''
Randomly make a test case with optional number of flows and 
range for number of fields per flow. 
'''
def test_rand_flows(num_flows=1, range_num_flow_fields=(1, 1)):
    flows_d = []
    state_field_initializers = []
    cycle_count_b, cycle_count_d, _ = unsigned_int_ser()
    flow_id_bits = num_flows.bit_length()
    packet = packet_header(cycle_count_b)

    for flow_num in range(1, num_flows+1):
        num_flow_fields = random.randint(range_num_flow_fields[0], range_num_flow_fields[1])
        flow_id = '{:0{bits}b}'.format(flow_num, bits=flow_id_bits)[::-1]
        flow_data = ""
        flow_data_d = []
        flow_field_inits = []

        for _ in range(0, num_flow_fields):
            state_field_value_b, state_field_value_d, state_field_init = random.choice(data_serializers)()
            flow_field_inits.append(state_field_init)
            flow_data += state_field_value_b
            flow_data_d.append(state_field_value_d)
        state_field_initializers.append(flow_field_inits)
        packet += flow(flow_id, [flow_data])
        flows_d.append(flow_data_d)
    print_output(packet, cycle_count_d, flows_d, (state_field_initializers))


'''
def test_manual_int_flows(cycle=20, flows=[[]]): #TODO update 
    packet = packet_header('{:0{bits}b}'.format(cycle, bits=32)[::-1])
    flow_id_bits = len(flows).bit_length()
    for flow_id in range(1, len(flows)+1):
        flow_data = ""
        for field in range(0, len(flows[flow_id-1])):
            field_b = '{:0{bits}b}'.format(flows[flow_id-1][field], bits=32)[::-1]
            flow_data += field_b
        packet += flow('{:0{bits}b}'.format(flow_id, bits=flow_id_bits)[::-1], [flow_data])
    print_output(packet, cycle, flows)
'''


#Example test generations

test_rand_flows(3, (1, 2))

'''
test_manual_int_flows(flows = [[400, 400, 400 ],
                                [400, 400, 400, 400, 400, 400, 400], 
                                [400, 400, 400, 400, 400, 400, 400]])
'''
