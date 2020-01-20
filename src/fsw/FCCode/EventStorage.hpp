#ifndef EVENT_STORAGE_HPP_
#define EVENT_STORAGE_HPP_

#include "Event.hpp"
#include <StateFieldRegistry.hpp>

class EventStorage {
  public:
    EventStorage(const std::string& name,
                 const unsigned int storage_size,
                 std::vector<ReadableStateFieldBase*>& _data_fields,
                 const char* (*_print_fn)(const unsigned int, std::vector<ReadableStateFieldBase*>&),
                 const unsigned int& _ccno);

    // Add all stored sub-events to the state field registry.
    void add_event_to_registry(StateFieldRegistry& registry);

    // The event storage uses the event it currently points to
    // to provide the same interface as a single event.
    size_t bitsize() const;
    const bit_array& get_bit_array() const;
    const char* print() const;
    void signal();

  private:
    std::vector<Event> sub_events;
    size_t event_ptr = 0;
};

#endif
