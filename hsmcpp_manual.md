# HSMCPP Developer Manual

This manual explains how to derive a Hierarchical State Machine (HSM) using the `hsmcpp` library in this workspace, especially for embedded firmware on the Arduino GIGA R1 / PlatformIO project.

The library is installed under:

- `.pio/libdeps/giga_r1_m7/hsmcpp/include/hsmcpp/`
- `.pio/libdeps/giga_r1_m7/hsmcpp/include/hsmcpp.hpp`

It is designed for event-driven state logic, where the system is always in one or more active states and transitions happen in response to events.

---

## 1. What HSMCPP is

`hsmcpp` implements a hierarchical state machine in C++.

The main concepts are:

- State ID: a unique integer representing a state
- Event ID: a unique integer representing an event
- Transition: rule that says: “when event X occurs in state A, move to state B”
- Hierarchy: parent states can contain child states
- Entry point: default child state entered when the parent becomes active
- History state: remembers the last active child state
- Final state: terminal state used in structured state transition patterns
- Timers: auto-generated events after time intervals
- Actions: state entry/exit triggers, timers, or transitions

The core class is:

- `hsmcpp::HierarchicalStateMachine`

The dispatcher interface is:

- `hsmcpp::IHsmEventDispatcher`

The Arduino-specific implementation used in this project is:

- `hsmcpp::HsmEventDispatcherArduino`

---

## 2. Library structure in this project

The library can be viewed as:

- `hsmcpp.hpp` = umbrella include file
- `hsmcpp/hsm.hpp` = main HSM API
- `hsmcpp/HsmTypes.hpp` = event/state timer type definitions and callback signatures
- `hsmcpp/IHsmEventDispatcher.hpp` = dispatcher interface contract
- `hsmcpp/HsmEventDispatcherArduino.hpp` = Arduino event queue implementation

The most important APIs used in practice are:

```cpp
HierarchicalStateMachine fsm(initialState);

fsm.registerState(...);
fsm.registerSubstate(...);
fsm.registerTransition(...);
fsm.registerSubstateEntryPoint(...);
fsm.registerTimer(...);
fsm.startTimer(...);
fsm.transition(...);
fsm.transitionSync(...);
fsm.isStateActive(...);
```

---

## 3. Core design concept

The HSM pattern is especially useful for firmware logic such as:

- power controller state machine
- motor enable/disable logic
- charging mode vs running mode
- watchdog and fault handling
- startup sequence control
- mode transitions in an AMR

A typical system model looks like this:

```cpp
enum class StateID {
    OFF,
    ACTIVE,
    IDLE,
    MOVING,
    FAULT
};

enum class EventID {
    TURN_ON,
    TURN_OFF,
    MOVE,
    STOP,
    FAULT_DETECTED,
    RESET
};
```

This is the model from which you define the state graph.

In HSM, the system always has an active state, and events decide the next state.

---

## 4. State machine lifecycle

A normal usage flow is:

1. Create enum IDs for states and events
2. Create a dispatcher
3. Create the state machine with the initial state
4. Register states
5. Register parent-child hierarchy with `registerSubstate()`
6. Register transitions with `registerTransition()`
7. Initialize the FSM using `initialize(dispatcher)`
8. Call `dispatcher->dispatchEvents()` in `loop()`
9. Trigger transitions with `fsm.transition(event)`

This is the exact pattern used in the workspace example under:

- `examples/hsm_state_machine/src/main.cpp`

---

## 5. Minimal Arduino example

The project’s example is the canonical pattern to follow.

```cpp
#include <Arduino.h>
#include <memory>

#include "hsmcpp/hsm.hpp"
#include "hsmcpp/HsmEventDispatcherArduino.hpp"

using namespace hsmcpp;

enum class StateID { OFF, ACTIVE, IDLE, MOVING };
enum class EventID { TURN_ON, TURN_OFF, MOVE, STOP };

std::shared_ptr<HsmEventDispatcherArduino> dispatcher;
std::shared_ptr<HierarchicalStateMachine> fsm;

void setup() {
    Serial.begin(115200);
    dispatcher = HsmEventDispatcherArduino::create();
    fsm = std::make_shared<HierarchicalStateMachine>(static_cast<StateID_t>(StateID::OFF));

    fsm->registerState(static_cast<StateID_t>(StateID::OFF),
                       nullptr,
                       [](const VariantVector_t&) {
                           Serial.println("[STATE] OFF");
                           return true;
                       },
                       nullptr);

    fsm->registerState(static_cast<StateID_t>(StateID::ACTIVE),
                       nullptr,
                       [](const VariantVector_t&) {
                           Serial.println("[STATE] ACTIVE");
                           return true;
                       },
                       nullptr);

    fsm->registerState(static_cast<StateID_t>(StateID::IDLE),
                       nullptr,
                       [](const VariantVector_t&) {
                           Serial.println("[SUB-STATE] IDLE");
                           return true;
                       },
                       nullptr);

    fsm->registerState(static_cast<StateID_t>(StateID::MOVING),
                       nullptr,
                       [](const VariantVector_t&) {
                           Serial.println("[SUB-STATE] MOVING");
                           return true;
                       },
                       nullptr);

    fsm->registerSubstate(static_cast<StateID_t>(StateID::ACTIVE), static_cast<StateID_t>(StateID::IDLE));
    fsm->registerSubstate(static_cast<StateID_t>(StateID::ACTIVE), static_cast<StateID_t>(StateID::MOVING));

    fsm->registerTransition(static_cast<StateID_t>(StateID::OFF),
                            static_cast<StateID_t>(StateID::IDLE),
                            static_cast<EventID_t>(EventID::TURN_ON));

    fsm->registerTransition(static_cast<StateID_t>(StateID::IDLE),
                            static_cast<StateID_t>(StateID::MOVING),
                            static_cast<EventID_t>(EventID::MOVE));

    fsm->registerTransition(static_cast<StateID_t>(StateID::MOVING),
                            static_cast<StateID_t>(StateID::IDLE),
                            static_cast<EventID_t>(EventID::STOP));

    fsm->registerTransition(static_cast<StateID_t>(StateID::ACTIVE),
                            static_cast<StateID_t>(StateID::OFF),
                            static_cast<EventID_t>(EventID::TURN_OFF));

    fsm->initialize(dispatcher);
}

void loop() {
    dispatcher->dispatchEvents();

    if (Serial.available() > 0) {
        char cmd = Serial.read();

        switch (cmd) {
            case '1':
                fsm->transition(static_cast<EventID_t>(EventID::TURN_ON));
                break;
            case '2':
                fsm->transition(static_cast<EventID_t>(EventID::MOVE));
                break;
            case '3':
                fsm->transition(static_cast<EventID_t>(EventID::STOP));
                break;
            case '0':
                fsm->transition(static_cast<EventID_t>(EventID::TURN_OFF));
                break;
            default:
                break;
        }
    }
}
```

This pattern is the foundation for every derived HSM you build in this project.

---

## 6. Derived HSM design for MCU power control

For a power controller or AMR controller, you normally model the machine as a hierarchy, not as one flat list of states.

A good structure is:

```cpp
enum class PowerState {
    OFF,
    BOOT,
    STANDBY,
    READY,
    RUNNING,
    FAULT,
    SHUTDOWN
};

enum class PowerEvent {
    PWR_ON,
    BOOT_DONE,
    GO_READY,
    START_MOTION,
    STOP_MOTION,
    FAULT_DETECTED,
    FAULT_CLEAR,
    SHUTDOWN_REQ,
    PWR_OFF
};
```

Then create hierarchy with parent states:

- `READY` contains `IDLE`, `MOTION`, `FAULT_RECOVERY`
- `RUNNING` contains `DRIVE`, `CHARGING`, `MANUAL_MODE`

Example:

```cpp
fsm->registerState(static_cast<StateID_t>(PowerState::OFF), ...);
fsm->registerState(static_cast<StateID_t>(PowerState::READY), ...);
fsm->registerState(static_cast<StateID_t>(PowerState::RUNNING), ...);
fsm->registerState(static_cast<StateID_t>(PowerState::IDLE), ...);
fsm->registerState(static_cast<StateID_t>(PowerState::MOTION), ...);

fsm->registerSubstate(static_cast<StateID_t>(PowerState::READY), static_cast<StateID_t>(PowerState::IDLE));
fsm->registerSubstate(static_cast<StateID_t>(PowerState::READY), static_cast<StateID_t>(PowerState::MOTION));
```

This allows the system to stay organized when multiple operating modes exist.

---

## 7. Transition registration and semantics

The main transition function is:

```cpp
void registerTransition(const StateID_t fromState,
                       const StateID_t toState,
                       const EventID_t onEvent,
                       HsmTransitionCallback_t transitionCallback = nullptr,
                       HsmTransitionConditionCallback_t conditionCallback = nullptr,
                       const bool expectedConditionValue = true);
```

Interpretation:

- from `fromState`
- on event `onEvent`
- if condition passes
- then transition to `toState`

Optional callbacks:

- `transitionCallback`: called when transition happens
- `conditionCallback`: checks if transition is allowed
- `expectedConditionValue`: expected result of condition callback

Example:

```cpp
fsm->registerTransition(
    static_cast<StateID_t>(PowerState::IDLE),
    static_cast<StateID_t>(PowerState::RUNNING),
    static_cast<EventID_t>(PowerEvent::START_MOTION),
    [](const VariantVector_t&) {
        Serial.println("Motion started");
    },
    [](const VariantVector_t&) {
        return true;
    },
    true);
```

This is the key method for building a state graph.

---

## 8. Registering states and callbacks

The state registration API is:

```cpp
void registerState(const StateID_t state,
                   HsmStateChangedCallback_t onStateChanged = nullptr,
                   HsmStateEnterCallback_t onEntering = nullptr,
                   HsmStateExitCallback_t onExiting = nullptr);
```

Callback meanings:

- `onStateChanged`: called when the state becomes active
- `onEntering`: called while entering the state
- `onExiting`: called before leaving the state

Important rule:

- If an enter/exit callback returns `false`, the transition is canceled.

This is a safety feature and is very useful in embedded control logic.

Example:

```cpp
fsm->registerState(
    static_cast<StateID_t>(PowerState::FAULT),
    nullptr,
    [](const VariantVector_t&) {
        Serial.println("Fault entered");
        return true;
    },
    []() {
        Serial.println("Leaving fault state");
        return true;
    });
```

Use this to:

- enable/disable power rails
- set watchdog states
- check sensor availability
- block unsafe transitions

---

## 9. Hierarchy patterns

The library supports nested states and parent-child relationships.

### 9.1 Parent and substate

```cpp
fsm->registerSubstate(parentState, childState);
```

This says: child state belongs under parent state.

Example:

```cpp
fsm->registerSubstate(static_cast<StateID_t>(PowerState::READY),
                      static_cast<StateID_t>(PowerState::IDLE));

fsm->registerSubstate(static_cast<StateID_t>(PowerState::READY),
                      static_cast<StateID_t>(PowerState::MOTION));
```

### 9.2 Entry point

Entry points allow a parent state to select a default child state when activated.

```cpp
fsm->registerSubstateEntryPoint(parentState,
                               childState,
                               onEvent,
                               conditionCallback,
                               expectedConditionValue);
```

Use this when a parent state should start in a specific child state depending on the event.

### 9.3 History state

Useful when you want the state machine to remember the previous substate and return there later.

```cpp
fsm->registerHistory(parentState,
                    historyState,
                    HistoryType::SHALLOW,
                    defaultTarget,
                    transitionCallback);
```

Use for:

- resume last mode
- restore prior operating substate after a temporary interruption
- recover from transient fault or service interruption

---

## 10. Final states and terminal behavior

A final state can be registered with:

```cpp
fsm->registerFinalState(state, event, callback...);
```

A final state is used when a state machine is designed to terminate a workflow or reach a completed state. It is often used in more advanced HSM patterns or generated state graphs.

This is not always needed for a firmware controller, but it is valuable when the state machine represents a lifecycle or sequence with a clear end condition.

---

## 11. Timers and actions

The library supports timed events.

### 11.1 Register a timer

```cpp
fsm->registerTimer(timerID, eventID);
```

When the timer expires, the associated event is sent to the state machine.

### 11.2 Start and stop timer

```cpp
fsm->startTimer(timerID, intervalMs, isSingleShot);
fsm->restartTimer(timerID);
fsm->stopTimer(timerID);
fsm->isTimerRunning(timerID);
```

Example:

```cpp
fsm->registerTimer(1, static_cast<EventID_t>(PowerEvent::BOOT_DONE));

fsm->startTimer(1, 2000, true);
```

This is effective for:

- startup delay
- sensor settling delay
- watchdog timeout
- state timeout logic
- fault retry timers

### 11.3 State actions

The library supports automatic state-triggered actions:

```cpp
enum class StateAction {
    START_TIMER,
    STOP_TIMER,
    RESTART_TIMER,
    TRANSITION
};
```

Example usage:

```cpp
fsm->registerStateAction(static_cast<StateID_t>(PowerState::BOOT),
                        StateActionTrigger::ON_STATE_ENTRY,
                        StateAction::START_TIMER,
                        static_cast<TimerID_t>(1),
                        1000,
                        true);
```

This allows state entry to automatically start a timer or trigger another event.

---

## 12. Triggering transitions

The transition API is central to HSM usage.

### Asynchronous transition

```cpp
fsm->transition(event, args...);
```

This sends the event to the dispatcher queue for later processing.

### Synchronous transition

```cpp
bool ok = fsm->transitionSync(event, timeoutMs, args...);
```

This waits for completion and is useful when you want deterministic execution in tests or blocking logic.

### Extended transition options

```cpp
bool ok = fsm->transitionEx(event, clearQueue, sync, timeoutMs, args...);
```

Useful for:

- clearing pending events before new input
- explicitly waiting for event completion
- advanced control flow

### Interrupt-safe version

```cpp
bool ok = fsm->transitionInterruptSafe(event);
```

This is intended for interrupt-level use, with the caveat that it relies on dispatcher support and may fail if the queue is full.

---

## 13. Checking state and transition possibility

Useful APIs:

```cpp
const std::list<StateID_t>& activeStates = fsm->getActiveStates();
bool isActive = fsm->isStateActive(stateID);
bool possible = fsm->isTransitionPossible(event, args...);
```

This is helpful when debugging or when building guards around logic.

---

## 14. Dispatcher contract

The actual HSM is decoupled from the platform by the dispatcher interface.

The required contract is defined by `IHsmEventDispatcher` and includes:

- `start()`
- `stop()`
- `registerEventHandler()`
- `unregisterEventHandler()`
- `emitEvent()`
- `enqueueEvent()`
- `enqueueAction()`
- `registerTimerHandler()`
- `unregisterTimerHandler()`
- `startTimer()`
- `restartTimer()`
- `stopTimer()`
- `isTimerRunning()`

For Arduino, `HsmEventDispatcherArduino` implements this and integrates with loop-based execution.

Important rule:

> The dispatcher's lifetime must outlive the HSM instance, or the HSM must be released before the dispatcher is destroyed.

---

## 15. Recommended HSM pattern for this project

For this firmware project, the most maintainable approach is:

1. Define a meaningful state enum for the controller domain
2. Use a root top-level state to represent the machine lifecycle
3. Group nested states under a logical parent state
4. Use parent states for major modes such as `POWER`, `READY`, `RUNNING`, `FAULT`
5. Use events for external commands and internal conditions
6. Use callback guards to reject invalid transitions
7. Use timer events for delays and watchdog control
8. Keep all transitions explicit and easy to audit

Example domain model:

```cpp
enum class ControllerState {
    OFF,
    BOOT,
    STANDBY,
    READY,
    DRIVE,
    CHARGING,
    FAULT,
    SHUTDOWN
};

enum class ControllerEvent {
    SYSTEM_ON,
    BOOT_COMPLETE,
    ENABLE_DRIVE,
    DISABLE_DRIVE,
    CHARGING_START,
    CHARGING_STOP,
    FAULT_DETECTED,
    FAULT_RESET,
    SHUTDOWN_REQUEST,
    SYSTEM_OFF
};
```

This is a strong basis for a robust motion-controller state machine.

---

## 16. Practical usage for AMR / motor control

In an AMR power controller, a derived HSM usually models:

- system power-up initialization
- idle wait state
- motion active state
- charging or standby state
- emergency stop / fault state
- watchdog-triggered reset/recovery

Example real-world transition graph:

```text
OFF --PWR_ON--> BOOT
BOOT --BOOT_COMPLETE--> READY
READY --ENABLE_DRIVE--> DRIVE
DRIVE --FAULT_DETECTED--> FAULT
FAULT --FAULT_RESET--> READY
READY --SHUTDOWN_REQUEST--> SHUTDOWN
SHUTDOWN --PWR_OFF--> OFF
```

The child states are often more detailed:

```text
READY
  ├─ IDLE
  ├─ HOLDING
  └─ WAITING_FOR_CMD

DRIVE
  ├─ FORWARD
  ├─ REVERSE
  ├─ TURNING
  └─ STOPPED
```

This makes the system easier to test and reason about.

---

## 17. Common pitfalls

### 17.1 Missing state registration

If a state is used in a transition but never registered with `registerState()`, the machine cannot process it correctly.

### 17.2 Invalid hierarchy

A substate cannot be assigned to multiple parents, and circular parent-child loops are forbidden.

### 17.3 Improper callback return values

For enter/exit callbacks, returning `false` cancels the transition. That is intentional, but it must be handled correctly.

### 17.4 Dispatcher lifetime

Do not destroy the dispatcher while the HSM is still in use.

### 17.5 Using transition APIs from reentrant callbacks

The library warns that synchronous calls from inside a state callback can deadlock, especially when waiting indefinitely.

### 17.6 Overcomplicated flat state graphs

A hierarchy is easier to maintain than a huge flat state list.

---

## 18. Debugging and validation strategy

Use the library’s debugging capability when enabled.

The docs note that the debug log can be generated with:

```cpp
fsm->enableHsmDebugging();
```

or

```cpp
fsm->enableHsmDebugging("/path/to/dump.hsmlog");
```

This helps visualize transitions and state flow, which is tremendously useful when deriving a complex controller logic model.

For embedded debugging, also use:

- Serial logging on every state transition
- explicit event names for state changes
- heartbeat prints in `loop()`
- test transitions under simulated inputs

---

## 19. Recommended coding template

The following structure is a practical template for a derived HSM in this project:

```cpp
using namespace hsmcpp;

enum class AppState {
    OFF,
    INIT,
    READY,
    RUNNING,
    FAULT,
    SHUTDOWN
};

enum class AppEvent {
    POWER_ON,
    INIT_OK,
    START_RUN,
    STOP_RUN,
    FAULT_DETECTED,
    FAULT_RESET,
    SHUTDOWN_REQ,
    POWER_OFF
};

std::shared_ptr<HsmEventDispatcherArduino> dispatcher;
std::shared_ptr<HierarchicalStateMachine> fsm;

void initHsm() {
    dispatcher = HsmEventDispatcherArduino::create();
    fsm = std::make_shared<HierarchicalStateMachine>(static_cast<StateID_t>(AppState::OFF));

    fsm->registerState(static_cast<StateID_t>(AppState::OFF), nullptr,
                       [](const VariantVector_t&) {
                           Serial.println("OFF");
                           return true;
                       }, nullptr);

    fsm->registerState(static_cast<StateID_t>(AppState::INIT), nullptr,
                       [](const VariantVector_t&) {
                           Serial.println("INIT");
                           return true;
                       }, nullptr);

    fsm->registerState(static_cast<StateID_t>(AppState::READY), nullptr,
                       [](const VariantVector_t&) {
                           Serial.println("READY");
                           return true;
                       }, nullptr);

    fsm->registerState(static_cast<StateID_t>(AppState::RUNNING), nullptr,
                       [](const VariantVector_t&) {
                           Serial.println("RUNNING");
                           return true;
                       }, nullptr);

    fsm->registerState(static_cast<StateID_t>(AppState::FAULT), nullptr,
                       [](const VariantVector_t&) {
                           Serial.println("FAULT");
                           return true;
                       }, nullptr);

    fsm->registerTransition(static_cast<StateID_t>(AppState::OFF),
                            static_cast<StateID_t>(AppState::INIT),
                            static_cast<EventID_t>(AppEvent::POWER_ON));

    fsm->registerTransition(static_cast<StateID_t>(AppState::INIT),
                            static_cast<StateID_t>(AppState::READY),
                            static_cast<EventID_t>(AppEvent::INIT_OK));

    fsm->registerTransition(static_cast<StateID_t>(AppState::READY),
                            static_cast<StateID_t>(AppState::RUNNING),
                            static_cast<EventID_t>(AppEvent::START_RUN));

    fsm->registerTransition(static_cast<StateID_t>(AppState::RUNNING),
                            static_cast<StateID_t>(AppState::READY),
                            static_cast<EventID_t>(AppEvent::STOP_RUN));

    fsm->registerTransition(static_cast<StateID_t>(AppState::RUNNING),
                            static_cast<StateID_t>(AppState::FAULT),
                            static_cast<EventID_t>(AppEvent::FAULT_DETECTED));

    fsm->registerTransition(static_cast<StateID_t>(AppState::FAULT),
                            static_cast<StateID_t>(AppState::READY),
                            static_cast<EventID_t>(AppEvent::FAULT_RESET));

    fsm->initialize(dispatcher);
}

void loop() {
    dispatcher->dispatchEvents();
}
```

This is the cleanest starting pattern for deriving a custom HSM in a control firmware repository.

---

## 20. Summary

`hsmcpp` is best used when your embedded system needs structured event-driven behavior with explicit state transitions, nested logic, and timeouts.

For this project, the recommended approach is:

- define explicit `StateID` and `EventID` enums
- create an Arduino dispatcher
- register states and substate hierarchy
- add transitions and phase guards
- use timers for delays and fault timeouts
- call `dispatchEvents()` in `loop()`
- trigger transitions with `fsm->transition(...)`

This gives you a maintainable, testable, and scalable controller design for AMR power and motion logic.

---

## 21. Minimal quick reference

```cpp
std::shared_ptr<HsmEventDispatcherArduino> dispatcher = HsmEventDispatcherArduino::create();
std::shared_ptr<HierarchicalStateMachine> fsm = std::make_shared<HierarchicalStateMachine>(initialState);

fsm->registerState(stateA, onStateChanged, onEntering, onExiting);
fsm->registerSubstate(parentState, childState);
fsm->registerTransition(fromState, toState, event);
fsm->registerSelfTransition(state, event, TransitionType::EXTERNAL_TRANSITION);
fsm->registerTimer(timerID, event);
fsm->startTimer(timerID, intervalMs, singleShot);

fsm->initialize(dispatcher);
dispatcher->dispatchEvents();
fsm->transition(event, args...);
```

This is the practical mental model for building a derived HSM with this library.
