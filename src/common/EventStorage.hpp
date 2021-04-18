#ifndef EVENT_STORAGE_HPP_
#define EVENT_STORAGE_HPP_

#include "Event.hpp"
#include <common/StateFieldRegistry.hpp>

class EventStorage : public EventBase
{
public:
  /**
     * @brief Construct a new Event Storage object.
     * 
     * @param name Name of event. The sub events will be named "name.i" where i ranges from 1 to
     *             storage_size.
     * @param storage_size Number of sub-events to create.
     * @param _data_fields Data fields related to the event.
     * @param _print_fn Function for printing data about the event.
     * @param int Reference to the control cycle count.
     */
  EventStorage(const std::string &name,
               const unsigned int storage_size,
               std::vector<ReadableStateFieldBase *> &_data_fields,
               const char *(*_print_fn)(const unsigned int, std::vector<ReadableStateFieldBase *> &));

  /**
     * @brief Add all stored sub-events to the state field registry.
     * 
     * @param registry
     */
  void add_events_to_registry(StateFieldRegistry &registry);

  // The event storage uses the event it currently points to
  // to provide the same interface as a single event.
  size_t bitsize() const override;
  const bit_array &get_bit_array() const override;

  #if defined(GSW) || defined(UNIT_TEST)
  void deserialize() override;
  const char *print() const override;
 #endif

  void signal() override;

private:
  /**
     * @brief Stores the sub-events that comprise the event storage.
     */
  std::vector<Event> sub_events;

#ifdef UNIT_TEST
public:
#endif
  /**
     * @brief The sub-event towards which the event storage is currently pointing.
     * 
     */
  size_t event_ptr = 0;
};

#endif
