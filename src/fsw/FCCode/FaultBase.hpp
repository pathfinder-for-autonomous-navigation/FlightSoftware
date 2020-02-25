#ifndef FAULT_BASE_HPP_
#define FAULT_BASE_HPP_

#include "common/Nameable.hpp"

/**
 * @brief Dummy class so that we can create pointers of type FaultBase that point to objects of
 * type Fault. See "Fault.hpp"
 */
class FaultBase : public Nameable {

};

#endif