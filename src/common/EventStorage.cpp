#include "EventStorage.hpp"
#include <string>

EventStorage::EventStorage(const std::string &name,
                           const unsigned int storage_size,
                           std::vector<ReadableStateFieldBase *> &_data_fields,
                           const char *(*_print_fn)(const unsigned int, std::vector<ReadableStateFieldBase *> &))
{
    assert(storage_size <= 99 && storage_size >= 0); // So that the suffixed event count doesn't have more than 2 digits
    sub_events.reserve(storage_size);
    for (unsigned char i = 1; i <= storage_size; i++)
    {
        char x[4];
        memset(x, 0, 4);
        x[0] = '.';
        sprintf(x, ".%d", i % 100);
        sub_events.emplace_back(name + std::string(x),
                                _data_fields, _print_fn);
    }
}

void EventStorage::add_events_to_registry(StateFieldRegistry &registry)
{
    for (Event &e : sub_events)
    {
        registry.add_event(&e);
    }
}

size_t EventStorage::bitsize() const
{
    return sub_events[event_ptr].bitsize();
}

const bit_array &EventStorage::get_bit_array() const
{
    return sub_events[event_ptr].get_bit_array();
}

#if defined(GSW) || defined(UNIT_TEST)

const char *EventStorage::print() const 
{
    return sub_events[event_ptr].print();
}

void EventStorage::deserialize() 
{
    sub_events[event_ptr].deserialize();
}

#endif

void EventStorage::signal()
{
    sub_events[event_ptr].signal();
    event_ptr++;
    if (event_ptr == sub_events.size())
        event_ptr = 0;
}
