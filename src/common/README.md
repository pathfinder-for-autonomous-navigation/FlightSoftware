## Flight Software Core Architecture

The flight software is broken into three key objects:
- `ControlTask`: Base unit of work done within the satellite.
- `StateField`: represents a satellite variable that is manipulated by control tasks.
- `StateFieldRegistry`: string-indexed database of all state fields. It is up to each Control Task to register its variables into this registry.

There can be several different kinds of Control Tasks:
- A `TimedTask` will be run periodically between a start and an end time that is specified in its constructor. If no end time is specified, the task is run periodically indefinitely. A timed task uses the ChibiOS API to achieve timing, and is run asynchronously with respect to the main control loop.
- A `StateMachine` is a special kind of task that changes its operation based on a state variable. It has `StateHandler` and `TransitionHandler` subtasks.

Control Tasks may need to use hardware to accomplish their functionality. All hardware is accessed via drivers that are subclasses of the `Device` class.

The following kinds of state fields are defined:
- A `InternalStateField` is an implementation detail. This kind of state field may be used by a single or multiple control tasks, but its value is not important to ground operators, so it is never sent to or set by the ground.
- A `ReadableStateField` may have its value read by the ground.
- A `WritableStateField` may have its value read by and written to by the ground.
  - `SMStateField` is a special kind of writable state field used by state machines to represent their state.

Related to read/writable state fields is a concept of a `Serializer`, which is a member object used by these state fields to compress their contained data into formats suitable for transmission over telemetry.

Telemetry can either be sent/received over the radio or via USB command line to a simulation computer. Telemetry over the former source will be achieved via a `QuakeControl` (implementation TBD). Telemetry over the latter is achieved via the `DebugControl` task, which uses a `debug_console` object to send telemetry or receive commands via the Teensy's USB port. The latter source is extremely useful for testing.
