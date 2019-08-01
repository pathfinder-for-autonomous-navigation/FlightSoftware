#include "InitializationRequired.hpp"

InitializationRequired::InitializationRequired() : _is_initialized(false) {}
bool InitializationRequired::init() {
    _is_initialized = true;
    return true;
}
bool InitializationRequired::is_initialized() const { return _is_initialized; }