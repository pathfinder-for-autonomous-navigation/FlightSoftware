#include "Event.hpp"

Event::Event(const std::string& name,
          std::vector<ReadableStateFieldBase*>& _data_fields,
          const char* (*_print_fn)(const unsigned int, std::vector<ReadableStateFieldBase*>&),
          const unsigned int& _ccno) :
          StateField<bool>(name, true, false),
          data_fields(_data_fields),
          print_fn(_print_fn),
          ccno(_ccno)
{
    unsigned int field_data_size_bits = 0;
    for(const ReadableStateFieldBase* field : data_fields) {
        field_data_size_bits += field->bitsize();
    }
    field_data.reset(new bit_array(32 + field_data_size_bits));
    for(size_t i = 0; i < field_data->size(); i++) {
        (*field_data)[i] = 0;
    }
}

void Event::serialize() {
    unsigned int field_data_ptr = 0;

    std::bitset<32> ccno_serialized(ccno);
    for(int i = 0; i < 32; i++) {
        (*field_data)[i] = ccno_serialized[i];
    }
    field_data_ptr += 32;

    for(ReadableStateFieldBase* field : data_fields) {
        const bit_array& field_bits = field->get_bit_array();
        field->serialize();
        for(size_t i = 0; i < field->bitsize(); i++, field_data_ptr++) {
            (*field_data)[field_data_ptr] = field_bits[i];
        }
    }
}

size_t Event::bitsize() const {
    return field_data->size();
}

const bit_array& Event::get_bit_array() const {
    return *field_data;
}

void Event::signal() {
    serialize();
}

const char* Event::print() const {
    return print_fn(ccno, data_fields);
}

void Event::deserialize() {}
bool Event::deserialize(const char *val) { return true; }
void Event::set_bit_array(const bit_array& arr) {}
