#ifndef MAIN_STATE_MACHINE_H
#define MAIN_STATE_MACHINE_H
/************************************** INCLUDE *************************************************************/
#include "high_level_common.h"
#include "active_state_machine.h"

/************************************** NAMESPACE ***********************************************************/
namespace msm = boost::msm;
namespace mpl = boost::mpl;
using namespace msm::front;

namespace rm_state_machine {
/************************************** CONST DEFINE ********************************************************/
/************************************** MACRO DEFINE ********************************************************/

/************************************** EMUN DEFINE *********************************************************/
/************************************** TYPE DEFINE *********************************************************/
class MainStateMachine_ : public msm::front::state_machine_def<MainStateMachine_>
{
public:
    /*------------------------------ FSM Interface ------------------------------*/
    template <class Event,class FSM>
    void on_entry(Event const&, FSM&)
    {
        std::cout <<D_COUT_CUS_BOLDRED<<"entering Main MSM"<<D_COUT_CUS_RESET << std::endl;
    }
    template <class Event,class FSM>
    void on_exit(Event const&, FSM&)
    {
        std::cout <<D_COUT_CUS_BOLDRED<<"leaving Main MSM"<<D_COUT_CUS_RESET << std::endl;
    }
    // Replaces the default no-transition response.
    template <class FSM,class Event>
    void no_transition(Event const& e, FSM&, int state)
    {
        std::cout <<D_COUT_CUS_BOLDRED<<"no transition from state: "<<state<<" on event: "<<typeid(e).name()<<D_COUT_CUS_RESET << std::endl;
    }

    /*------------------------------ Actions ------------------------------*/
    // transition actions
    struct StartBeingActive {
        template <class Event, class Fsm, class SourceState, class TargetState>
        void operator()(Event const&, Fsm&, SourceState&, TargetState&)
        {
            std::cout <<D_COUT_CUS_RED<<"start_being_active"<<D_COUT_CUS_RESET << std::endl;
        }
    };
    struct StartFollowCommand {
        template <class Event, class Fsm, class SourceState, class TargetState>
        void operator()(Event const& evt, Fsm&, SourceState&, TargetState&)
        {
            std::cout <<D_COUT_CUS_RED<<"start_follow_command"<<D_COUT_CUS_RESET << std::endl;
        }
    };
    struct TurnOff {
        template <class Event, class Fsm, class SourceState, class TargetState>
        void operator()(Event const& evt, Fsm&, SourceState&, TargetState&)
        {
            std::cout <<D_COUT_CUS_RED<<"turning off"<<D_COUT_CUS_RESET << std::endl;
        }
    };

    /*------------------------------ Guards ------------------------------*/
    // guard conditions

    /*------------------------------ States ------------------------------*/
    // The list of FSM states
    class s_Idle : public HighLevelState
    {
    public:
        s_Idle() : HighLevelState(HighLevelFeaturesEnum::H_Idle)
        {std::cout <<D_COUT_CUS_BOLDRED<<"s_Idle Constructor"<<D_COUT_CUS_RESET << std::endl;}
        ~s_Idle() {std::cout <<D_COUT_CUS_BOLDRED<<"s_Idle Destructor"<<D_COUT_CUS_RESET << std::endl;}

        // User interface
        void sendAbort() {};

        // every (optional) entry/exit methods get the event passed.
        typedef mpl::vector1<f_Idle> flag_list; //On Flags

        template <class Event,class FSM>
        void on_entry(Event const&, FSM&) {std::cout <<D_COUT_CUS_BOLDRED<<"entering s_Idle State"<<D_COUT_CUS_RESET << std::endl;}
        template <class Event,class FSM>
        void on_exit(Event const&, FSM&) {std::cout <<D_COUT_CUS_BOLDRED<<"leaving s_Idle State"<<D_COUT_CUS_RESET << std::endl;}

        // Actions
        struct InternalAction {
            template <class Event, class Fsm, class SourceState, class TargetState>
            void operator()(Event const&, Fsm&, SourceState&, TargetState&)
            {
                std::cout <<D_COUT_CUS_RED<<"InternalAction - stop command under idle"<<D_COUT_CUS_RESET << std::endl;
            }
        };
        struct sanityAbortAction {
            template <class Event, class Fsm, class SourceState, class TargetState>
            void operator()(Event const&, Fsm&, SourceState& src, TargetState&)
            {
                std::cout <<D_COUT_CUS_RED<<"sanityAbortAction - stop command under idle"<<D_COUT_CUS_RESET << std::endl;
                src.sendAbort();
            }
        };

        // Transition table for Idle
        struct internal_transition_table : mpl::vector<
            //                Event                     Action                    Guard
            //       +--------+-------------------------+-------------------------+---------------+
            Internal <        e_Stop,                   InternalAction,           none            >,
            Internal <        e_HighLevelFeatureEND,    sanityAbortAction,        none            >
            //       +--------+-------------------------+-------------------------+---------------+
        > {};
    };

    // the initial state of the MainStateMachine. Must be defined
    typedef s_Idle initial_state;

    /*------------------------------ Internal Transition Actions ------------------------------*/
    /*------------------------------ Internal Transition Table ------------------------------*/

    // used to show a transition conflict. 
    // This guard will simply deactivate one transition and thus solve the conflict

    /*------------------------------ Transition Table ------------------------------*/
    // transition table for MainStateMachine
    struct transition_table : mpl::vector<
        //    Start           Event                     Next                      Action               Guard
        //  +-+---------------+-------------------------+-------------------------+--------------------+---------------+
        Row < s_Idle,         e_UserCommandRequest,     s_Active::entry_pt<Active::s_PseudoEntry>,
                                                                                  StartFollowCommand,  none            >,
        Row < s_Idle,         e_StartDocking,           s_Active::entry_pt<Active::s_PseudoEntry>,
                                                                                  StartFollowCommand,  none            >,
        //  +-+---------------+-------------------------+-------------------------+--------------------+---------------+
        Row < s_Active,       e_Stop,                   s_Idle,                   TurnOff,             none            >,
        Row < s_Active,       e_UserCommandRequest,     s_Active::entry_pt<Active::s_PseudoEntry>,
                                                                                  StartFollowCommand,  none            >,
        Row < s_Active::exit_pt<Active::s_PseudoExit>,
                              e_Stop,                   s_Idle,                   TurnOff,             none            >
        //  +-+---------------+-------------------------+-------------------------+--------------------+---------------+
    > {};
};
// Pick a back-end
typedef msm::back::state_machine<MainStateMachine_> MainStateMachine;

/************************************** VARIABLE DECALRE ****************************************************/
/************************************** FUNCTION PROTOTYPE **************************************************/

} //End of namespace rm_state_machine
#endif // MAIN_STATE_MACHINE_H
