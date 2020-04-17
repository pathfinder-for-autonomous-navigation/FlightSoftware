#ifndef EVENT_HPP_
#define EVENT_HPP_

#include <common/StateField.hpp>

/**
 * @brief This event base class exposes methods for reading and
 * writing to an event.
 */
class EventBase
{
public:
  /**
     * @brief The signal method should be called to indicate
     * that the event is occurred. Definitions of this function
     * should pull data from the event's related state fields and store
     * them into the event's bitset. Calls serialize.
     */
  virtual void signal() = 0;


  /**
     * @brief Get bitsize of contained bitset.
     */
  virtual size_t bitsize() const = 0;

  /**
     * @brief Get the contained bitset.
     * 
     * @return const bit_array& 
     */
  virtual const bit_array &get_bit_array() const = 0;

#if defined(GSW) || defined(UNIT_TEST)
   /**
       * @brief Store the event's bitset into the event's fields' data
       * so that it can be retrieved by ground software for parsing.
       */
   virtual void deserialize() = 0;

  /**
     * @brief Print event data to a string.
     * 
     * @return const char* The string.
     */
  virtual const char *print() const = 0;

#endif
};

/**
 * @brief This is an extension of state field that provides the ability
 * for control tasks to generate events.
 */
class Event : public ReadableStateFieldBase, public StateField<bool>, public EventBase {
  public:
    /**
     * @brief Construct a new Event object
     * 
     * @param name Name of event.
     * @param _data_fields Data fields related to the event.
     * @param _print_fn Function for printing data about the event.
     */
    Event(const std::string& name,
          std::vector<ReadableStateFieldBase*>& _data_fields,
          const char* (*_print_fn)(const unsigned int, std::vector<ReadableStateFieldBase*>&));

   /**
     * @brief Move constructor, required for EventStorage.
     * 
     * @param other 
     */
    Event(Event &&other);

    const std::string &name() const override { return _name; }
    const std::vector<ReadableStateFieldBase*> &_data_fields() { return data_fields; }

    // Functions that will be overridden from SerializableStateFieldBase
  protected:
    const std::string _name;
    void serialize() override;
  public:
      // Functions from the EventBase interface.
      void signal() override;
      size_t bitsize() const override;
      const bit_array &get_bit_array() const override;
      void deserialize() override;
      void set_bit_array(const bit_array &arr) override;
      bool deserialize(const char *val) override;
      const char *print() const override;

      // Functions from the SerializableStateField interface. These are
      // going to receive stub implementations since events should not
      // be EEPROM-saveable.
      unsigned int eeprom_save_period() const override;
      unsigned int get_eeprom_repr() const override;
      void set_from_eeprom(unsigned int val) override;

   static ReadableStateField<unsigned int> *ccno;

    virtual ~Event() {}

  private:
    std::vector<ReadableStateFieldBase*>& data_fields;
    std::unique_ptr<bit_array> field_data;
    const char* (*print_fn)(const unsigned int, std::vector<ReadableStateFieldBase*>&);

    // Disable state field functions.
    using StateField<bool>::set;
    using StateField<bool>::get;


};

#endif
