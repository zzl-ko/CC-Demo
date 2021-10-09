#ifndef GOTO_STATE_MACHINE_H
#define GOTO_STATE_MACHINE_H
/************************************** INCLUDE *************************************************************/
#include "high_level_common.h"
#include "docking_state_machine.h"

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

//Goto as MSM istelf
class Goto : public HighLevelState
{
public:
    Goto() : HighLevelState(HighLevelFeaturesEnum::H_Goto) {}

    /*------------------------------ User Interface ------------------------------*/
    void EntryFunc(std::string location_name);
    void ExitFunc(EndConditionEnum end_condition);

    void handleCompleteProcedure();
    void handleAbortProcedure(EndConditionEnum abort_condition);

    /*------------------------------ FSM Interface ------------------------------*/
    typedef mpl::vector1<f_Goto> flag_list; //On Flags
    template <class Event,class FSM>
    void on_entry(Event const&, FSM&) {
        std::cout <<D_COUT_CUS_BOLDYELLOW<<"entering Goto MSM"<<D_COUT_CUS_RESET <<std::endl;
        end_condition_ = EndConditionEnum::End_ABORT_GENERAL;
        //EntryFunc(evt.sub_command_, evt.last_used_);
    }
    template <class Event,class FSM>
    void on_exit(Event const&,FSM& ) {
        std::cout <<D_COUT_CUS_BOLDYELLOW<<"leaving Goto MSM"<<D_COUT_CUS_RESET <<std::endl;
        ExitFunc(end_condition_);
    }

    /*------------------------------ Actions ------------------------------*/
    struct a_CompleteAction {
        template <class Event, class Fsm, class SourceState, class TargetState>
        void operator()(Event const&, Fsm& fsm, SourceState& src, TargetState&)
        {
            std::cout <<D_COUT_CUS_YELLOW<<"goto a_CompleteAction"<<D_COUT_CUS_RESET <<std::endl;
            fsm.end_condition_ = EndConditionEnum::End_COMPLETE;
        }
    };
    struct a_AbortAction {
        template <class Event, class Fsm, class SourceState, class TargetState>
        void operator()(Event const& evt, Fsm& fsm, SourceState& src, TargetState&)
        {
            std::cout <<D_COUT_CUS_YELLOW<<"goto a_AbortAction, evt.end_condition_: "<<(int)evt.end_condition_<<D_COUT_CUS_RESET <<std::endl;
            fsm.end_condition_ = evt.end_condition_;
        }
    };

    struct a_CompleteProcedure {
        template <class Event, class Fsm, class SourceState, class TargetState>
        void operator()(Event const&, Fsm& fsm, SourceState& src, TargetState&)
        {
            std::cout <<D_COUT_CUS_YELLOW<<"goto a_CompleteProcedure"<<D_COUT_CUS_RESET <<std::endl;
            src.end_condition_ = EndConditionEnum::End_COMPLETE;
            // src.handleCompleteProcedure();
            fsm.process_event(e_Stop());
        }
    };
    struct a_AbortProcedure {
        template <class Event, class Fsm, class SourceState, class TargetState>
        void operator()(Event const& evt, Fsm& fsm, SourceState& src, TargetState&)
        {
            std::cout <<D_COUT_CUS_YELLOW<<"goto a_AbortProcedure, evt.end_condition_: "<<(int)evt.end_condition_<<D_COUT_CUS_RESET <<std::endl;
            src.end_condition_ = evt.end_condition_;
            // fsm.handleAbortProcedure();
            fsm.process_event(e_Stop());
        }
    };
    struct a_TransitionProcedure {
        template <class Event, class Fsm, class SourceState, class TargetState>
        void operator()(Event const& evt, Fsm& fsm, SourceState& src, TargetState&)
        {
            std::cout <<D_COUT_CUS_YELLOW<<"goto a_TransitionProcedure"<<D_COUT_CUS_RESET <<std::endl;
            src.end_condition_ = EndConditionEnum::End_ABORT_GENERAL;
            // fsm.handleAbortProcedure();
        }
    };
    struct a_EntryProcedure {
        template <class Event, class Fsm, class SourceState, class TargetState>
        void operator()(Event const& evt, Fsm& fsm, SourceState& src, TargetState&)
        {
            std::cout <<D_COUT_CUS_YELLOW<<"goto a_EntryProcedure"<<D_COUT_CUS_RESET <<std::endl;
            fsm.EntryFunc(evt.sub_command_);
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
    struct s_PseudoExit:exit_pseudo_state <e_Stop> {
        template <class Event,class FSM>
        void on_entry(Event const& ,FSM&) {std::cout <<D_COUT_CUS_BOLDYELLOW<<"entering goto s_PseudoExit State"<<D_COUT_CUS_RESET <<std::endl;}
        template <class Event,class FSM>
        void on_exit(Event const&,FSM& ) {std::cout <<D_COUT_CUS_BOLDYELLOW<<"leaving goto s_PseudoExit State"<<D_COUT_CUS_RESET <<std::endl;}
    };

    //Pseudo Entry
    struct s_PseudoEntry:entry_pseudo_state<> {
        template <class Event,class FSM>
        void on_entry(Event const& ,FSM&) {std::cout <<D_COUT_CUS_BOLDYELLOW<<"entering goto s_PseudoEntry State"<<D_COUT_CUS_RESET <<std::endl;}
        template <class Event,class FSM>
        void on_exit(Event const&,FSM& ) {std::cout <<D_COUT_CUS_BOLDYELLOW<<"leaving goto s_PseudoEntry State"<<D_COUT_CUS_RESET <<std::endl;}
    };

    class s_SimpleGoto : public msm::front::state<>
    {
    public:
        template <class Event,class FSM>
        void on_entry(Event const& ,FSM&) {std::cout <<D_COUT_CUS_BOLDYELLOW<<"entering s_SimpleGoto State"<<D_COUT_CUS_RESET <<std::endl;}
        template <class Event,class FSM>
        void on_exit(Event const&,FSM& ) {std::cout <<D_COUT_CUS_BOLDYELLOW<<"leaving s_SimpleGoto State"<<D_COUT_CUS_RESET <<std::endl;}
    };

    // the initial state. Must be defined
    typedef s_SimpleGoto initial_state;

    /*------------------------------ Internal Transition Table ------------------------------*/
    struct internal_transition_table : mpl::vector<
        //                    Event                     Action                    Guard
        //       +------------+-------------------------+-------------------------+---------------+
        Internal <            e_GotoEnd,                a_CompleteProcedure,      g_CompleteState >,
        Internal <            e_GotoEnd,                a_AbortProcedure,         g_AbortState    >,
        Internal <            e_HighLevelFeatureEND,    a_AbortProcedure,         none            >
        //       +------------+-------------------------+-------------------------+---------------+
    > {};

    /*------------------------------ Transition Table ------------------------------*/
    struct transition_table : mpl::vector<
        //    Start           Event                     Next                      Action               Guard
        //  +-+---------------+-------------------------+-------------------------+--------------------+---------------+
        Row < s_PseudoEntry,  e_UserCommandRequest,     s_SimpleGoto,             a_EntryProcedure,    none            >,
        //  +-+---------------+-------------------------+-------------------------+--------------------+---------------+
        Row < s_GetAwayFromDocking,    
                              e_LeavingDockingEnd,      s_SimpleGoto,             none,                none            >,
        //  +-+---------------+-------------------------+-------------------------+--------------------+---------------+
        Row < s_SimpleGoto,   e_Stop,                   s_PseudoExit,             none,                none            >,
        Row < s_SimpleGoto,   e_GetAwayFromDocking,     s_GetAwayFromDocking,     none,                none            >,
        Row < s_SimpleGoto,   e_StartDocking,           s_Docking,                none,                none            >,
        //  +-+---------------+-------------------------+-------------------------+--------------------+---------------+
        Row < s_Docking,      e_Stop,                   s_PseudoExit,             a_CompleteAction,    g_CompleteState >,
        Row < s_Docking,      e_Stop,                   s_PseudoExit,             a_AbortAction,       g_AbortState    >
        //  +-+---------------+-------------------------+-------------------------+--------------------+---------------+
    > {};
};
typedef msm::back::state_machine<Goto> s_Goto;


} //End of namespace rm_state_machine
} //End of namespace robot_manager
#endif //GOTO_STATE_MACHINE_H
