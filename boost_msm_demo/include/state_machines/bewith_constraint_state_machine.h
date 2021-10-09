#ifndef ConstraintFollow_STATE_MACHINE_H
#define ConstraintFollow_STATE_MACHINE_H
/************************************** INCLUDE *************************************************************/
#include "high_level_common.h"

/************************************** NAMESPACE ***********************************************************/
namespace robot_manager {

namespace msm = boost::msm;
namespace mpl = boost::mpl;
using namespace msm::front;

namespace rm_state_machine {
/************************************** CONST DEFINE ********************************************************/
/************************************** MACRO DEFINE ********************************************************/
/************************************** EMUN DEFINE *********************************************************/
/************************************** TYPE DEFINE *********************************************************/

//ConstraintFollow MSM istelf
class ConstraintFollow : public HighLevelState
{
public:
    ConstraintFollow() : HighLevelState(HighLevelFeaturesEnum::H_ConstraintFollow) {}

    /*------------------------------ User Interface ------------------------------*/
    void EntryFunc();
    void ExitFunc(EndConditionEnum end_condition);

    void handleCompleteProcedure();
    void handleAbortProcedure(EndConditionEnum end_condition);

    /*------------------------------ FSM Interface ------------------------------*/
    typedef mpl::vector1<f_ConstraintFollow> flag_list; //On Flags
    template <class Event,class FSM>
    void on_entry(Event const& evt,FSM&) {
        std::cout <<D_COUT_CUS_BOLDYELLOW<<"entering ConstraintFollow MSM"<<D_COUT_CUS_RESET <<std::endl;
        end_condition_ = EndConditionEnum::End_ABORT_GENERAL;
    }
    template <class Event,class FSM>
    void on_exit(Event const&,FSM& ) {
        std::cout <<D_COUT_CUS_BOLDYELLOW<<"leaving ConstraintFollow MSM"<<D_COUT_CUS_RESET <<std::endl;
        ExitFunc(end_condition_);
    }

    /*------------------------------ Actions ------------------------------*/
    struct a_TransitionProcedure {
        template <class Event, class Fsm, class SourceState, class TargetState>
        void operator()(Event const& evt, Fsm& fsm, SourceState&, TargetState&)
        {
            std::cout <<D_COUT_CUS_YELLOW<<"ConstraintFollow a_TransitionProcedure"<<D_COUT_CUS_RESET <<std::endl;
            fsm.end_condition_ = evt.end_condition_;
        }
    };

    struct a_CompleteProcedure {
        template <class Event, class Fsm, class SourceState, class TargetState>
        void operator()(Event const&, Fsm& fsm, SourceState& src, TargetState&)
        {
            std::cout <<D_COUT_CUS_YELLOW<<"ConstraintFollow a_CompleteProcedure"<<D_COUT_CUS_RESET <<std::endl;
            src.end_condition_ = EndConditionEnum::End_COMPLETE;
            fsm.process_event(e_Stop());
        }
    };
    struct a_AbortProcedure {
        template <class Event, class Fsm, class SourceState, class TargetState>
        void operator()(Event const&, Fsm& fsm, SourceState& src, TargetState&)
        {
            std::cout <<D_COUT_CUS_YELLOW<<"ConstraintFollow a_AbortProcedure"<<D_COUT_CUS_RESET <<std::endl;
            src.end_condition_ = EndConditionEnum::End_ABORT_GENERAL;
            fsm.process_event(e_Stop());
        }
    };
    struct a_EntryProcedure {
        template <class Event, class Fsm, class SourceState, class TargetState>
        void operator()(Event const& evt, Fsm& fsm, SourceState& src, TargetState& trgt)
        {
            std::cout <<D_COUT_CUS_YELLOW<<"ConstraintFollow a_EntryProcedure"<<D_COUT_CUS_RESET <<std::endl;
            trgt.EntryFunc();
        }
    };

    /*------------------------------ Guards ------------------------------*/
    struct g_CompleteState {
        template <class Event, class Fsm, class SourceState, class TargetState>
        bool operator()(Event const& evt, Fsm&, SourceState&, TargetState&)
        {
            return evt.end_condition_ == EndConditionEnum::End_COMPLETE;
        }
    };
    struct g_AbortState {
        template <class Event, class Fsm, class SourceState, class TargetState>
        bool operator()(Event const& evt, Fsm&, SourceState&, TargetState&)
        {
            return evt.end_condition_ >= EndConditionEnum::End_ABORT_GENERAL;
        }
    };

    /*------------------------------ States ------------------------------*/
    //Pseudo Exit
    struct s_PseudoExit : exit_pseudo_state <e_Stop> {
        template <class Event,class FSM>
        void on_entry(Event const& ,FSM&) {std::cout <<D_COUT_CUS_BOLDYELLOW<<"entering ConstraintFollow s_PseudoExit State"<<D_COUT_CUS_RESET <<std::endl;}
        template <class Event,class FSM>
        void on_exit(Event const&,FSM& ) {std::cout <<D_COUT_CUS_BOLDYELLOW<<"leaving ConstraintFollow s_PseudoExit State"<<D_COUT_CUS_RESET <<std::endl;}
    };

    //Pseudo Exit
    struct s_PseudoExit2 : exit_pseudo_state <e_SelfTriggeredCommand> {
        template <class Event,class FSM>
        void on_entry(Event const& ,FSM&) {std::cout <<D_COUT_CUS_BOLDYELLOW<<"entering ConstraintFollow s_PseudoExit2 State"<<D_COUT_CUS_RESET <<std::endl;}
        template <class Event,class FSM>
        void on_exit(Event const&,FSM& ) {std::cout <<D_COUT_CUS_BOLDYELLOW<<"leaving ConstraintFollow s_PseudoExit2 State"<<D_COUT_CUS_RESET <<std::endl;}
    };

    // //Regular
    class s_MainConstraintFollow : public msm::front::state<>
    {
    public:
        template <class Event,class FSM>
        void on_entry(Event const& ,FSM&) {std::cout <<D_COUT_CUS_BOLDYELLOW<<"entering s_MainConstraintFollow State"<<D_COUT_CUS_RESET <<std::endl;}
        template <class Event,class FSM>
        void on_exit(Event const&,FSM& ) {std::cout <<D_COUT_CUS_BOLDYELLOW<<"leaving s_MainConstraintFollow State"<<D_COUT_CUS_RESET <<std::endl;}
    };

    // the initial state. Must be defined
    typedef s_MainConstraintFollow initial_state;

    
    /*------------------------------ Internal Transition Table ------------------------------*/
    struct internal_transition_table : mpl::vector<
        //                    Event                     Action                    Guard
        //       +------------+-------------------------+-------------------------+---------------+
        Internal <            e_HighLevelFeatureEND,    a_CompleteProcedure,      g_CompleteState >,
        Internal <            e_HighLevelFeatureEND,    a_AbortProcedure,         g_AbortState    >
        //       +------------+-------------------------+-------------------------+---------------+
    > {};

    /*------------------------------ Transition Table ------------------------------*/
    struct transition_table : mpl::vector<
        //    Start           Event                     Next                      Action               Guard
        //  +-+---------------+-------------------------+-------------------------+--------------------+---------------+
        Row < s_GetAwayFromDocking,  
                              e_LeavingDockingEnd,      s_MainConstraintFollow,   none,                none            >,
        //  +-+---------------+-------------------------+-------------------------+--------------------+---------------+
        Row < s_MainConstraintFollow,
                              e_Stop,                   s_PseudoExit,             none,                none            >,
        Row < s_MainConstraintFollow,
                              e_SelfTriggeredCommand,   s_PseudoExit2,            a_TransitionProcedure,none           >,
        Row < s_MainConstraintFollow,
                              e_GetAwayFromDocking,     s_GetAwayFromDocking,     none,                none            >
        //  +-+---------------+-------------------------+-------------------------+--------------------+---------------+
    > {};
};
typedef msm::back::state_machine<ConstraintFollow> s_ConstraintFollow;


} //End of namespace rm_state_machine
} //End of namespace robot_manager
#endif //ConstraintFollow_STATE_MACHINE_H
