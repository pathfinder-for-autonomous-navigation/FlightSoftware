#include "FakeQuake.hpp"

using namespace Devices;

bool FakeQuake::is_functional() {
    return true;
}
void FakeQuake::reset() { }
void FakeQuake::disable() { }

std::string fake_quake_name = "Fake Quake";
std::string& FakeQuake::name() const { 
    return fake_quake_name;
}

int FakeQuake::sbdwb(char const *c, int len) { 
    return 0; // TODO
}

int FakeQuake::run_sbdix() {
    return 0; // TODO
}

int FakeQuake::end_sbdix() {
    return 0; // TODO
}

int FakeQuake::sbdrb() {
    return 0; // TODO
}