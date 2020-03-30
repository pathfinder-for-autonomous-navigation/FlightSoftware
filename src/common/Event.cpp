#include "Event.hpp"

ReadableStateField<unsigned int> *Event::ccno = nullptr;

Event::Event(const std::string& name,
          std::vector<ReadableStateFieldBase*>& _data_fields,
          const char* (*_print_fn)(const unsigned int, std::vector<ReadableStateFieldBase*>&)) :
          StateField<bool>(name, true, false),
          _name(name),
          data_fields(_data_fields),
          print_fn(_print_fn)
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

Event::Event(Event &&other) : StateField<bool>(other.name(), true, false),
                              data_fields(other.data_fields),
                              field_data(std::move(other.field_data)) {}

void Event::serialize() {
    unsigned int field_data_ptr = 0;

    std::bitset<32> ccno_serialized(ccno->get());
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
    return print_fn(ccno->get(), data_fields);
}

void Event::deserialize() 
{
    unsigned int field_data_ptr = 0;
    std::bitset<32> ccno_serialized;
    for (int i = 0; i < 32; i++)
    {
        ccno_serialized[i] = (*field_data)[i];
    }
    field_data_ptr += 32;
    const unsigned int event_ccno = (int)(ccno_serialized.to_ulong());
    ccno->set(event_ccno);

    for (ReadableStateFieldBase *field : data_fields)
    {
        bit_array &field_bits = const_cast<bit_array &>(field->get_bit_array());
        for (unsigned int i = 0; i < field->bitsize(); i++, field_data_ptr++)
        {
            field_bits[i] = (*field_data)[field_data_ptr];
        }
        field->deserialize();
    }
}

void Event::set_bit_array(const bit_array &arr)
{
    assert(arr.size() == field_data->size());
    for (unsigned int i = 0; i < arr.size(); i++)
    {
        (*field_data)[i] = arr[i];
    }
}

bool Event::deserialize(const char *val) { return true; }


unsigned int Event::eeprom_save_period() const { return 0; }
unsigned int Event::get_eeprom_repr() const { return 0; }
void Event::set_from_eeprom(unsigned int val) { }
