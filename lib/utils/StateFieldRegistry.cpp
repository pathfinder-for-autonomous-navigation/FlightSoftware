#include "StateFieldRegistry.hpp"

DataField::DataField(const std::string& name) : Nameable(name) {}

void StateFieldRegistry::add_reader(Task& reader, DataField& field) {
    if (_fields_allowed_to_read.count(&reader) == 0) {
        // TODO add debug console
        _fields_allowed_to_read.emplace(&reader, *(new std::vector<DataField*>) );
    }
    // TODO
    // _fields_allowed_to_read.at(&reader).push_back(&field);
}

void StateFieldRegistry::add_writer(Task& writer, DataField& field) {
    if (_fields_allowed_to_write.count(&writer) == 0) {
        // TODO add debug console
        _fields_allowed_to_write.emplace(&writer, *(new std::vector<DataField*>) );
    }
    // TODO
    // _fields_allowed_to_write.at(&writer).push_back(&field);
}

bool StateFieldRegistry::can_read(Task& reader, DataField& field) {
    if (!access_lock.initialized) rwMtxObjectInit(&access_lock);
    
    if (chThdGetSelfX() != nullptr) rwMtxRLock(&access_lock);
    {
        // TODO
    }
    if (chThdGetSelfX() != nullptr) rwMtxRUnlock(&access_lock);

    return false;
}

bool StateFieldRegistry::can_write(Task& reader, DataField& field) {
    if (!access_lock.initialized) rwMtxObjectInit(&access_lock);
    
    if (chThdGetSelfX() != nullptr) rwMtxRLock(&access_lock);
    {
        // TODO        
    }
    if (chThdGetSelfX() != nullptr) rwMtxRUnlock(&access_lock);
    return false;
}