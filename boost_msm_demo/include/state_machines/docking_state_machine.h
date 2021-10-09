#ifndef DOCKING_STATE_MACHINE_H
#define DOCKING_STATE_MACHINE_H
/************************************** INCLUDE *************************************************************/
#include "msm_common.h"

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

//--Docking MSM--//
struct Docking: public msm::front::state_machine_def<Docking>, public CommonToStateMachine
{
    /*------------------------------ Variables ------------------------------*/
    EndConditionEnum end_condition_ = EndConditionEnum::End_ABORT_GENERAL;

    /*------------------------------ User Interface ------------------------------*/
    void EntryFunc();
    void ExitFunc(EndConditionEnum end_condition);
    
    void handleCompleteProcedure();
    void handleAbortProcedure(EndConditionEnum abort_condition);

    /*------------------------------ FSM Interface ------------------------------*/
    typedef mpl::vector1<f_Docking> flag_list; //On Flags
    template <class Event, class FSM>
    void on_entry(Event const& evt, FSM&) {
        std::cout <<D_COUT_CUS_BOLDYELLOW<<"entering Docking MSM"<<D_COUT_CUS_RESET <<std::endl;
        end_condition_ = EndConditionEnum::End_ABORT_GENERAL;
        EntryFunc();
    }
    template <class Event, class FSM>
    void on_exit(Event const&, FSM&) {
        std::cout <<D_COUT_CUS_BOLDYELLOW<<"leaving Docking MSM"<<D_COUT_CUS_RESET <<std::endl;
        ExitFunc(end_condition_);
    }

    /*------------------------------ Actions ------------------------------*/
    struct a_CompleteProcedure {
        template <class Event, class Fsm, class SourceState, class TargetState>
        void operator()(Event const&, Fsm& fsm, SourceState& src, TargetState&)
        {
            std::cout <<D_COUT_CUS_YELLOW<<"docking a_CompleteProcedure"<<D_COUT_CUS_RESET <<std::endl;
            src.end_condition_ = EndConditionEnum::End_COMPLETE;
            fsm.process_event(e_Stop(EndConditionEnum::End_COMPLETE));
        }
    };
    struct a_AbortProcedure {
        template <class Event, class Fsm, class SourceState, class TargetState>
        void operator()(Event const& evt, Fsm& fsm, SourceState& src, TargetState&)
        {
            std::cout <<D_COUT_CUS_YELLOW<<"docking a_AbortProcedure: "<<(int)evt.end_condition_<<D_COUT_CUS_RESET <<std::endl;
            src.end_condition_ = evt.end_condition_;
            fsm.process_event(e_Stop(evt.end_condition_));
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
    // Pseudo Exit
    struct s_PseudoExit:exit_pseudo_state <e_Stop> {
        template <class Event, class FSM>
        void on_entry(Event const&, FSM&) {std::cout <<D_COUT_CUS_BOLDYELLOW<<"entering Docking s_PseudoExit State"<<D_COUT_CUS_RESET <<std::endl;}
        template <class Event, class FSM>
        void on_exit(Event const&,FSM& ) {std::cout <<D_COUT_CUS_BOLDYELLOW<<"leaving Docking s_PseudoExit State"<<D_COUT_CUS_RESET <<std::endl;}
    };

    class s_SimpleDocking : public msm::front::state<>
    {
    public:
        template <class Event, class FSM>
        void on_entry(Event const&, FSM&) {std::cout <<D_COUT_CUS_BOLDYELLOW<<"entering s_SimpleDocking State"<<D_COUT_CUS_RESET <<std::endl;}
        template <class Event, class FSM>
        void on_exit(Event const&,FSM& ) {std::cout <<D_COUT_CUS_BOLDYELLOW<<"leaving s_SimpleDocking State"<<D_COUT_CUS_RESET <<std::endl;}
    };

    // the initial state. Must be defined
    typedef s_SimpleDocking initial_state;

    /*------------------------------ Internal Transition Table ------------------------------*/
    struct internal_transition_table : mpl::vector<
        //                    Event                     Action                    Guard
        //       +------------+-------------------------+-------------------------+---------------+
        Internal <            e_DockingEnd,             a_CompleteProcedure,      g_CompleteState >,
        Internal <            e_DockingEnd,             a_AbortProcedure,         g_AbortState    >,
        Internal <            e_HighLevelFeatureEND,    a_AbortProcedure,         none            >
        //       +------------+-------------------------+-------------------------+---------------+
    > {};

    /*------------------------------ Transition Table ------------------------------*/
    struct transition_table : mpl::vector<
        //    Start           Event                     Next                      Action               Guard
        //  +-+---------------+-------------------------+-------------------------+--------------------+---------------+
        Row < s_SimpleDocking,e_Stop,                   s_PseudoExit,             none,                none            >
        //  +-+---------------+-------------------------+-------------------------+--------------------+---------------+
    > {};

private:

};
typedef msm::back::state_machine<Docking> s_Docking;

/************************************** VARIABLE DECALRE ****************************************************/
/************************************** FUNCTION PROTOTYPE **************************************************/

} //End of namespace rm_state_machine
} //End of namespace robot_manager
#endif //DOCKING_STATE_MACHINE_H
