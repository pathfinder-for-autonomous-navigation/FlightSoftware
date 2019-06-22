#ifndef STATE_FIELD_REGISTRY_HPP_
#define STATE_FIELD_REGISTRY_HPP_

#include <map>
#include <vector>
#include <Debuggable.hpp>
#include "ControlTask.hpp"
#include <ChRt.h>

/**
 * @brief Dummy class so that we can create pointers of type DataField that point to
 * objects of type StateField<T>. See "StateField.hpp"
 */
class DataField {};

/**
 * @brief Registry of state fields and which threads have read/write access to the
 * fields. StateField objects use this registry to verify valid access to their values.
 * Essentially, this class is a lightweight wrapper around multimap.
 */
class StateFieldRegistry : public Debuggable {
  private:
    std::multimap<Task*, std::vector<DataField*>> _fields_allowed_to_read;
    std::multimap<Task*, std::vector<DataField*>> _fields_allowed_to_write;
  public:
    using Debuggable::Debuggable;

    /**
     * @brief Allows the specified Control Task to read the specified state field.
     * 
     * @param r Task
     * @param field State field
     */
    void add_reader(Task& r, DataField& field);

    /**
     * @brief Allows the specified Control Task to write to the specified state field.
     * 
     * @param r Task
     * @param field Data field
     */
    void add_writer(Task& w, DataField& field);

    /**
     * @brief Checks registry for read access.
     * 
     * @param r 
     * @param field 
     * @return true If Control Task has read access to state field.
     * @return false If Control Task does not have read access to state field.
     */
    bool can_read(Task& r, DataField& field);

    /**
     * @brief Checks registry for write access.
     * 
     * @param w
     * @param field 
     * @return true If Control Task has write access to state field.
     * @return false If Control Task does not have write access to state field.
     */
    bool can_write(Task& r, DataField& field);
};

#endif