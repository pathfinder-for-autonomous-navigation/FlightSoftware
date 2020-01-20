#include "EventStorage.hpp"
#include <string>

EventStorage::EventStorage(const std::string& name,
                 const unsigned int storage_size,
                 std::vector<ReadableStateFieldBase*>& _data_fields,
                 const char* (*_print_fn)(const unsigned int, std::vector<ReadableStateFieldBase*>&),
                 const unsigned int& _ccno)
{
    assert(storage_size < 100); // So that the suffixed event count doesn't have more than 2 digits
    sub_events.reserve(storage_size);
    for(size_t i = 1; i <= storage_size; i++) {
        char x[4];
        x[0] = '.';
        itoa(i, x + 1, 10);
        x[3] = 0;
        sub_events.emplace_back(name + std::string(x),
            _data_fields, _print_fn, _ccno);
    }
}

void EventStorage::add_event_to_registry(StateFieldRegistry& registry) {
    for(Event& e : sub_events) {
        registry.add_readable_field(static_cast<ReadableStateFieldBase*>(&e));
    }
}

size_t EventStorage::bitsize() const {
    return sub_events[event_ptr].bitsize();
}

const bit_array& EventStorage::get_bit_array() const {
    return sub_events[event_ptr].get_bit_array();
}

const char* EventStorage::print() const {
    return sub_events[event_ptr].print();
}

void EventStorage::signal() {
    sub_events[event_ptr].signal();
    event_ptr++;
    if (event_ptr == sub_events.size()) event_ptr = 0;
}
