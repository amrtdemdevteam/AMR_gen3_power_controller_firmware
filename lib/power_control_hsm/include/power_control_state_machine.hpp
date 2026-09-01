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

        enum class State : hsmcpp::StateID_t {
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


        enum class Event : hsmcpp::EventID_t {
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
            EMER_LOW,
            EMER_HIGH,
            MODE_MANUAL,
            MODE_AUTO,
            CHARGED_IDLE_TIMEOUT
        };

        enum  Timers {
            INIT_TIMER = 100, // just timer ID
            WAIT_IPC_TIMER = 101
        };

        /**
         * @brief Initializes the power control state machine.
         * @return true if initialization was successful, false otherwise.
         */
        bool begin();

        /**
         * @brief Runs the power control state machine.
         */
        void run();

        /**
         * @brief Posts an event to the state machine then it will transition accordingly.
         * @param event The event to post.
         */
        void postEvent(Event event);

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
        void setStateCallbacks(State                  state,
                               StateChangedCallback_t onChanged,
                               StateEnterCallback_t   onEnter=nullptr,
                               StateExitCallback_t    onExit=nullptr);
        
        /**
         * @brief Sets a callback to be called when a transition fails.
         *        Must be called before begin().
         * @param callback Callback function to be called on transition failure.
         */
        void setTransitionFailedCallback(std::function<void(std::string)> callback);

        /**
         * @brief Converts a State enum value to its string representation.
         * @param state The State enum value to convert.
         * @return A string representation of the State enum value.
         */
        static std::string ToString(const State state) {
            switch (state) {
                case State::SHUTDOWN: return "SHUTDOWN";
                case State::INIT: return "INIT";
                case State::SERVICE_LAYER: return "SERVICE_LAYER";
                case State::APPLICATION_LAYER: return "APPLICATION_LAYER";
                case State::SLEEP: return "SLEEP";
                case State::CHARGED: return "CHARGED";
                case State::SERVICE_MODE: return "SERVICE_MODE";
                case State::SERVICE_EMER: return "SERVICE_EMER";
                case State::CHARGING: return "CHARGING";
                case State::GO_TO_CHARGER: return "GO_TO_CHARGER";
                case State::WAIT_CHARGER: return "WAIT_CHARGER";
                case State::CHARGING_IN_PROGRESS: return "CHARGING_IN_PROGRESS";
                case State::OPERATION: return "OPERATION";
                case State::APP_EMER: return "APP_EMER";
                case State::MANUAL: return "MANUAL";
                case State::STANDBY: return "STANDBY";
                case State::AUTO: return "AUTO";
                default: return "UNKNOWN_STATE";
            }
        }

    private:

        std::shared_ptr<hsmcpp::HsmEventDispatcherArduino> dispatcher_;
        std::shared_ptr<hsmcpp::HierarchicalStateMachine> hsm_;

        // Callback map populated via setStateCallbacks() before begin()
        std::map<State, StateCallbacks> stateCallbackMap_;

        // This function will be called when a transition fails. It can be set by the user of the state machine.
        std::function<void(std::string)> transitionFailedCallback_;

        void registerStates();
        void registerStateActions();
        void registerSubstates();
        void registerTransitions();
        void registerFailureHandler();
        void registerTimer();
        void transitionFailedHandler(const std::list<hsmcpp::StateID_t>& activeStates, const hsmcpp::EventID_t event, const hsmcpp::VariantVector_t& eventArgs);
        
        
        static hsmcpp::StateID_t toStateID(const State state) {
            return static_cast<hsmcpp::StateID_t>(state);
        }

        static hsmcpp::EventID_t toEventID(const Event event) {
            return static_cast<hsmcpp::EventID_t>(event);
        }

        static hsmcpp::TimerID_t toTimerID(const Timers timer) {
            return static_cast<hsmcpp::TimerID_t>(timer);
        }



};
#endif // POWER_CONTROL_STATE_MACHINE_HPP