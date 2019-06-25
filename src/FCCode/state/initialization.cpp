#include "../debug.hpp"
#include "StateField.hpp"
#include "StateFieldRegistry.hpp"
#include "Serializer.hpp"
#include "StateMachine.hpp"
#include "StateMachineStateField.hpp"
#include "ControlTask.hpp"

StateFieldRegistry registry(dbg);
static const size_t prop_num_states = 2;
static SMStateSerializer<prop_num_states> prop_state_serializer;
static SMStateField<prop_num_states> propulsion_state_field("prop.sm_state", dbg, registry, prop_state_serializer);

class PropulsionSM : StateMachine<prop_num_states> {
  public:
    static constexpr float max_tank_pressure = 10.0f;
    static constexpr size_t tank_compressed_sz = 10;
    static constexpr Serializer<float, float, tank_compressed_sz> tank_pressure_serializer(0, max_tank_pressure);
    Serializer<temperature_t, temperature_t, SerializerBase::temp_sz> temperature_serializer;
    Serializer<unsigned int, unsigned int, static_cast<size_t>(1)> tank_valve_serializer(0, 1);
};

void initialize() {
    // Declare constants and serializers
    static constexpr float max_tank_pressure = 10;
    static constexpr size_t tank_compressed_sz = 10;
    Serializer<float, float, tank_compressed_sz> tank_pressure_serializer(0, max_tank_pressure);

    Serializer<temperature_t, temperature_t, SerializerBase::temp_sz> temperature_serializer;

    Serializer<unsigned int, unsigned int, static_cast<size_t>(1)> tank_valve_serializer(0, 1);

    // Declare fields
    ReadableStateField<float, float, tank_compressed_sz>
        tank_pressure("prop.tank_pressure", dbg, registry, tank_pressure_serializer, []()->float{ return 2.0f; });
    
    ReadableStateField<temperature_t, temperature_t, SerializerBase::temp_sz>
        inner_tank_temperature("prop.inner_tank.temperature", dbg, registry, temperature_serializer, []() -> temperature_t { return static_cast<temperature_t>(10); });
    ReadableStateField<temperature_t, temperature_t, SerializerBase::temp_sz>
        outer_tank_temperature("prop.outer_tank.temperature", dbg, registry, temperature_serializer, []() -> temperature_t { return static_cast<temperature_t>(10); });

    WritableStateField<unsigned int, unsigned int, static_cast<size_t>(1)>
        intertank_valve("prop.intertank_valve", dbg, registry, tank_valve_serializer, []() -> unsigned int { return 1; });

    // Declare tasks

    // Register fields and tasks with state registry
}