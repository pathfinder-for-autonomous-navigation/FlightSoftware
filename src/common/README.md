# Common Software

Contains base-level components for Flight Software, like serializers, state fields, the state field registry, events, faults, and the debug console.

## Useful utilities
- `serializer_test`: Compile using `pio run -e serializer_test`. This is a barebones utility for printing the bit result of serializing a value.

## Testing
Run `pio test -e native_common_ci`.