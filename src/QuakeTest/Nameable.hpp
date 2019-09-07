#ifndef NAMEABLE_HPP_
#define NAMEABLE_HPP_

#include <string>

class Nameable {
   protected:
    const std::string _name;

   public:
    Nameable(const std::string &name);
    const std::string &name() const;
};

#endif