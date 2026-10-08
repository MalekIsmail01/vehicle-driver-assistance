# Vehicle Driver Assistance (Prototype)

A small, portable C99 project for experimenting with vehicle-assistance logic:

- Cruise control with an adjustable speed setpoint
- Brake-pedal cancellation and manual accelerator override
- A distance-based emergency-braking request
- Rain sensing with dry, light-rain, and heavy-rain wiper levels
- Input validation and host-side unit tests

This repository contains the control-logic layer only. It does not include
board-specific drivers, AUTOSAR RTE bindings, or CAN configuration. Adapt the
input/output structures in `include/vehicle_assist.h` to connect the logic to
your ECU, sensors, switches, actuators, and CAN messages.

> **Safety:** This is an educational prototype, not production automotive
> software. Its simplified stopping-distance calculation does not account for
> road friction, grade, actuator delays, sensor uncertainty, or functional
> safety requirements. Do not use it to control a real vehicle.

## Build and test

With CMake:

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Or compile the host-side tests directly with a C99 compiler:

```sh
cc -std=c99 -Wall -Wextra -Werror -pedantic -Iinclude \
  src/vehicle_assist.c tests/test_vehicle_assist.c -o vehicle_assist_tests
./vehicle_assist_tests
```

## Behavior and units

- Speed inputs and target speed use km/h.
- Lead-vehicle distance uses meters; zero means no valid lead-distance reading.
- Rain wetness is a normalized percentage from 0 (dry) to 100 (very wet).
- Target-speed buttons change the setpoint by 5 km/h per update, bounded to
  10–130 km/h. Debounce physical buttons in the hardware adapter.
- The emergency-braking request uses `v² / (2a)` with a fixed normal
  deceleration of 10 m/s² and is asserted when a positive lead distance is
  shorter than that stopping distance.
- Brake input or emergency braking disables cruise control. Braking takes
  priority over accelerator input. While cruise is enabled, the controller
  requests acceleration below the setpoint and braking above it.
- Rain-level transitions use hysteresis to reduce wiper-level chatter.

Call `vehicle_assist_init` once, then call `vehicle_assist_update` periodically.
An update returns `false` for invalid pointers or out-of-range inputs; on
failure, the output object is left unchanged.
