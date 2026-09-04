# FSM State Machine Example

This example demonstrates a simple three-state finite state machine using the `arduino-fsm` library.

## Build

From the project root:

```bash
platformio run -d examples/hsm_state_machine
```

## Upload

```bash
platformio run -d examples/hsm_state_machine --target upload
```

The example cycles through:

- Red -> Green -> Blue -> Red
- Triggered by a timeout every 5 seconds
