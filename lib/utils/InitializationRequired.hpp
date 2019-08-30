#ifndef INITIALIZATION_REQUIRED_HPP_
#define INITIALIZATION_REQUIRED_HPP_

class InitializationRequired {
   protected:
    bool _is_initialized = false;

   public:
    InitializationRequired();
    virtual bool init() {
        _is_initialized = true;
        return true;
    }
    bool is_initialized() const { return _is_initialized; }
};

#endif