#ifndef INITIALIZATION_REQUIRED_HPP_
#define INITIALIZATION_REQUIRED_HPP_

/**
 * @brief Used by classes as a marker to indicate that objects of the class
 * must be initialized via the init() function prior to use.
 */
class InitializationRequired {
   private:
    bool _is_initialized = false;

   public:
    /**
     * @brief Constructor
     */
    InitializationRequired() {}

    /**
     * @brief Initializer. Overridden in children classes.
     *
     * @return True
     */
    virtual bool init() {
        _is_initialized = true;
        return true;
    }

    /**
     * @brief Checks if object is initialized.
     *
     * @return True if initialized, false otherwise.
     */
    bool is_initialized() const { return _is_initialized; }
};

#endif