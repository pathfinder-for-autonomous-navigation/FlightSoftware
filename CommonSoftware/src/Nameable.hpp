#ifndef NAMEABLE_HPP_
#define NAMEABLE_HPP_

#include <string>

class Nameable {
   public:
    virtual const std::string &name() const = 0;
};

#endif
