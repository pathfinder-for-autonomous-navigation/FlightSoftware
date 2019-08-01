#ifndef INITIALIZATION_REQUIRED_HPP_
#define INITIALIZATION_REQUIRED_HPP_

class InitializationRequired {
protected:
  bool _is_initialized;

public:
  InitializationRequired();
  bool init();
  bool is_initialized() const;
};

#endif