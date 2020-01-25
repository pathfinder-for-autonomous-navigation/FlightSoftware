#ifndef EVENT_HPP_
#define EVENT_HPP_

#include <common/StateField.hpp>

/**
 * @brief This is an extension of state field that provides the ability
 * for control tasks to generate events.
 */
class Event : public ReadableStateFieldBase, public StateField<bool> {
  public:
    Event(const std::string& name,
          std::vector<ReadableStateFieldBase*>& _data_fields,
          const char* (*_print_fn)(const unsigned int, std::vector<ReadableStateFieldBase*>&),
          const unsigned int& _ccno);

    // Functions that will be overridden from SerializableStateFieldBase
  protected:
    void serialize() override;
  public:
    size_t bitsize() const override;
    const bit_array& get_bit_array() const override;
    const char* print() const override;

    /**
     * @brief Signal that the event occurred, and pull data from the data fields.
     * 
     * This function just calls serialize().
     */
    void signal();

  private:
    std::vector<ReadableStateFieldBase*>& data_fields;
    std::unique_ptr<bit_array> field_data;
    const char* (*print_fn)(const unsigned int, std::vector<ReadableStateFieldBase*>&);

    const unsigned int& ccno; // Control cycle count

    // The value of events should never be set via deserialize or set_bit_array(), so these
    // functions will have stub implementations.
    void deserialize() override;
    bool deserialize(const char *val) override;
    void set_bit_array(const bit_array& arr) override;

    // Disable state field functions.
    using StateField<bool>::set;
    using StateField<bool>::get;
};

#endif
