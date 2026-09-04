#ifndef POWER_CONTROL_STATE_MACHINE_HPP
#define POWER_CONTROL_STATE_MACHINE_HPP


#include <memory>
#include <map>
#include <hsmcpp.hpp>
#include <hsmcpp/HsmEventDispatcherArduino.hpp>

class PowerControlStateMachine {
    public:
        // Callback type aliases matching hsmcpp API
        using StateChangedCallback_t = hsmcpp::HsmStateChangedCallback_t; // callback function signature: void(const hsmcpp::VariantVector_t&)
        using StateEnterCallback_t   = hsmcpp::HsmStateEnterCallback_t; // callback function signature: bool(const hsmcpp::VariantVector_t&)
        using StateExitCallback_t    = hsmcpp::HsmStateExitCallback_t; // callback function signature: bool(void)
        using TransitionFailedCallback_t = hsmcpp::HsmTransitionFailedCallback_t; // callback function signature: void(const std::list<hsmcpp::StateID_t>&, const hsmcpp::EventID_t, const hsmcpp::VariantVector_t&)

        // Groups entry/exit/changed callbacks for a single state
        struct StateCallbacks {
            StateChangedCallback_t onChanged;
            StateEnterCallback_t   onEnter;
            StateExitCallback_t    onExit;
        };

        class State {
            public:
                enum Value : hsmcpp::StateID_t {
                    SHUTDOWN = 1,
                    INIT,
                    SERVICE_LAYER,
                    APPLICATION_LAYER,
                    SLEEP,
                    CHARGED,
                    SERVICE_MODE,
                    SERVICE_EMER,
                    CHARGING,
                    GO_TO_CHARGER,
                    WAIT_CHARGER,
                    CHARGING_IN_PROGRESS,
                    OPERATION,
                    APP_EMER,
                    MANUAL,
                    STANDBY,
                    AUTO
                };

                static std::string ToString(const Value state) {
                    switch (state) {
                        case SHUTDOWN: return "SHUTDOWN";
                        case INIT: return "INIT";
                        case SERVICE_LAYER: return "SERVICE_LAYER";
                        case APPLICATION_LAYER: return "APPLICATION_LAYER";
                        case SLEEP: return "SLEEP";
                        case CHARGED: return "CHARGED";
                        case SERVICE_MODE: return "SERVICE_MODE";
                        case SERVICE_EMER: return "SERVICE_EMER";
                        case CHARGING: return "CHARGING";
                        case GO_TO_CHARGER: return "GO_TO_CHARGER";
                        case WAIT_CHARGER: return "WAIT_CHARGER";
                        case CHARGING_IN_PROGRESS: return "CHARGING_IN_PROGRESS";
                        case OPERATION: return "OPERATION";
                        case APP_EMER: return "APP_EMER";
                        case MANUAL: return "MANUAL";
                        case STANDBY: return "STANDBY";
                        case AUTO: return "AUTO";
                        default: return "UNKNOWN_STATE";
                    }
                }

                static std::string ToString(const hsmcpp::StateID_t stateID) {
                    return ToString(static_cast<Value>(stateID));
                }
        };


        class Event {
            public:
                enum Value : hsmcpp::EventID_t {
                    POWER_ON = 1,
                    POWER_OFF,
                    INIT_DONE,
                    FMS_WAKE_UP,
                    FMS_NO_TASK,
                    FMS_TASK_ASSIGNED,
                    IPC_OK,
                    IPC_FAIL,
                    BATTERY_LOW,
                    BATTERY_HIGH,
                    CHARGER_PLUGGED,
                    CHARGER_REMOVED,
                    ARRIVED_AT_CHARGER,
                    EMER_ON,
                    EMER_OFF,
                    MODE_MANUAL,
                    MODE_AUTO,
                    CHARGED_IDLE_TIMEOUT
                };

                static std::string ToString(const Value event) {
                    switch (event) {
                        case POWER_ON: return "POWER_ON";
                        case POWER_OFF: return "POWER_OFF";
                        case INIT_DONE: return "INIT_DONE";
                        case FMS_WAKE_UP: return "FMS_WAKE_UP";
                        case FMS_NO_TASK: return "FMS_NO_TASK";
                        case FMS_TASK_ASSIGNED: return "FMS_TASK_ASSIGNED";
                        case IPC_OK: return "IPC_OK";
                        case IPC_FAIL: return "IPC_FAIL";
                        case BATTERY_LOW: return "BATTERY_LOW";
                        case BATTERY_HIGH: return "BATTERY_HIGH";
                        case CHARGER_PLUGGED: return "CHARGER_PLUGGED";
                        case CHARGER_REMOVED: return "CHARGER_REMOVED";
                        case ARRIVED_AT_CHARGER: return "ARRIVED_AT_CHARGER";
                        case EMER_ON: return "EMER_ON";
                        case EMER_OFF: return "EMER_OFF";
                        case MODE_MANUAL: return "MODE_MANUAL";
                        case MODE_AUTO: return "MODE_AUTO";
                        case CHARGED_IDLE_TIMEOUT: return "CHARGED_IDLE_TIMEOUT";
                        default: return "UNKNOWN_EVENT";
                    }
                }

                static std::string ToString(const hsmcpp::EventID_t eventID) {
                    return ToString(static_cast<Value>(eventID));
                }
        };


        /**
         * @brief Initializes the power control state machine.
         * @param initialState The initial state to start the state machine in.
         * @return true if initialization was successful, false otherwise.
         */
        bool begin(State::Value initialState=State::SHUTDOWN);

        /**
         * @brief Runs the power control state machine.
         */
        void run();

        /**
         * @brief Posts an event to the state machine then it will transition accordingly.
         * @param event The event to post.
         */
        void postEvent(Event::Value event);

        /**
         * @brief Sets entry/exit/changed callbacks for a state.
         *        Must be called before begin().
         * @param state      Target state.
         * @param onChanged  Called when HSM has already changed its current state (may be nullptr).
         *                   callback function signature: void(const hsmcpp::VariantVector_t&)
         * @param onEnter    Called right before changing to this state.
         *                   Transition is canceled if callback returns false (may be nullptr).
         *                   callback function signature: bool(const hsmcpp::VariantVector_t&)
         * @param onExit     Called for this state before starting to transition away.
         *                   Transition is canceled if callback returns false (may be nullptr).
         *                   callback function signature: bool(void)
         */
        void setStateCallbacks(State::Value           state,
                               StateChangedCallback_t onChanged,
                               StateEnterCallback_t   onEnter=nullptr,
                               StateExitCallback_t    onExit=nullptr);
        
        /**
         * @brief Sets a callback to be called when a transition fails.
         *        Must be called before begin().
         * @param callback Callback function to be called on transition failure.
         */
        void setTransitionFailedCallback(std::function<void(std::string)> callback);

    private:

        std::shared_ptr<hsmcpp::HsmEventDispatcherArduino> dispatcher_;
        std::shared_ptr<hsmcpp::HierarchicalStateMachine> hsm_;

        // Callback map populated via setStateCallbacks() before begin()
        std::map<State::Value, StateCallbacks> stateCallbackMap_;

        // This function will be called when a transition fails. It can be set by the user of the state machine.
        std::function<void(std::string)> transitionFailedCallback_;

        void registerStates();
        void registerStateActions();
        void registerSubstates();
        void registerTransitions();
        void registerFailureHandler();
        void registerTimer();
        void transitionFailedHandler(const std::list<hsmcpp::StateID_t>& activeStates, const hsmcpp::EventID_t event, const hsmcpp::VariantVector_t& eventArgs);
        
        
        static hsmcpp::StateID_t toStateID(const State::Value state) {
            return static_cast<hsmcpp::StateID_t>(state);
        }

        static hsmcpp::EventID_t toEventID(const Event::Value event) {
            return static_cast<hsmcpp::EventID_t>(event);
        }



};
#endif // POWER_CONTROL_STATE_MACHINE_HPP