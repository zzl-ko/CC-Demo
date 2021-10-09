#ifndef BEWITH_STATE_MACHINE_H
#define BEWITH_STATE_MACHINE_H
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

//Bewith as MSM istelf
class Bewith : public HighLevelState
{
public:
    Bewith() : HighLevelState(HighLevelFeaturesEnum::H_Bewith) {}

    /*------------------------------ User Interface ------------------------------*/
    void EntryFunc();
    void ExitFunc(EndConditionEnum end_condition);

    void handleCompleteProcedure();
    void handleAbortProcedure(EndConditionEnum end_condition);

    /*------------------------------ FSM Interface ------------------------------*/
    typedef mpl::vector1<f_Bewith> flag_list; //On Flags
    template <class Event,class FSM>
    void on_entry(Event const& ,FSM&) {
        std::cout <<D_COUT_CUS_BOLDYELLOW<<"entering Bewith MSM"<<D_COUT_CUS_RESET <<std::endl;
        end_condition_ = EndConditionEnum::End_ABORT_GENERAL;
        EntryFunc();
    }
    template <class Event,class FSM>
    void on_exit(Event const&,FSM& ) {
        std::cout <<D_COUT_CUS_BOLDYELLOW<<"leaving Bewith MSM"<<D_COUT_CUS_RESET <<std::endl;
        ExitFunc(end_condition_);
    }

    /*------------------------------ Actions ------------------------------*/
    struct a_CompleteProcedure {
        template <class Event, class Fsm, class SourceState, class TargetState>
        void operator()(Event const&, Fsm& fsm, SourceState& src, TargetState&)
        {
            std::cout <<D_COUT_CUS_YELLOW<<"bewith a_CompleteProcedure"<<D_COUT_CUS_RESET <<std::endl;
            src.end_condition_ = EndConditionEnum::End_COMPLETE;
            fsm.process_event(e_Stop());
        }
    };
    struct a_AbortProcedure {
        template <class Event, class Fsm, class SourceState, class TargetState>
        void operator()(Event const&, Fsm& fsm, SourceState& src, TargetState&)
        {
            std::cout <<D_COUT_CUS_YELLOW<<"beiwth a_AbortProcedure"<<D_COUT_CUS_RESET <<std::endl;
            src.end_condition_ = EndConditionEnum::End_ABORT_GENERAL;
            fsm.process_event(e_Stop());
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
    struct g_Stay {
        template <class Event, class Fsm, class SourceState, class TargetState>
        bool operator()(Event const& evt, Fsm&, SourceState&, TargetState&)
        {
            // bool desired_feature_snap_to_mic = evt.DesiredFeature() == HighLevelFeaturesEnum::H_SnapToMic;
            // bool desired_feature_snap_to_user = evt.DesiredFeature() == HighLevelFeaturesEnum::H_SnapToUser;
            bool stay_due_to_desired_feature = false;//desired_feature_snap_to_mic || desired_feature_snap_to_user;
            std::cout <<D_COUT_CUS_YELLOW<<"Bewith g_Stay - should stay, desired feature: "<<evt.DesiredFeature()<<D_COUT_CUS_RESET <<std::endl << std::endl;
            return stay_due_to_desired_feature;
        }
    };

    /*------------------------------ States ------------------------------*/
    //Pseudo Exit
    struct s_PseudoExit:exit_pseudo_state <e_Stop> {
        template <class Event, class FSM>
        void on_entry(Event const&, FSM&) {std::cout <<D_COUT_CUS_BOLDYELLOW<<"entering Bewith s_PseudoExit State"<<D_COUT_CUS_RESET <<std::endl;}
        template <class Event, class FSM>
        void on_exit(Event const&,FSM& ) {std::cout <<D_COUT_CUS_BOLDYELLOW<<"leaving Bewith s_PseudoExit State"<<D_COUT_CUS_RESET <<std::endl;}
    };
    //Regular Bewith
    class s_RegularBewith : public msm::front::state<>
    {
    public:
        template <class Event,class FSM>
        void on_entry(Event const& ,FSM&) {std::cout <<D_COUT_CUS_BOLDYELLOW<<"entering s_RegularBewith State"<<D_COUT_CUS_RESET <<std::endl;}
        template <class Event,class FSM>
        void on_exit(Event const&,FSM& ) {std::cout <<D_COUT_CUS_BOLDYELLOW<<"leaving s_RegularBewith State"<<D_COUT_CUS_RESET <<std::endl;}
    };
    // the initial state. Must be defined
    typedef s_RegularBewith initial_state;

    /*------------------------------ Internal Transition Table ------------------------------*/
    struct internal_transition_table : mpl::vector<
        //                    Event                     Action                    Guard
        //       +------------+-------------------------+-------------------------+---------------+
        Internal <            e_UserCommandRequest,     none,                     g_Stay          >,
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
                              e_LeavingDockingEnd,      s_RegularBewith,          none,                none            >,
        //  +-+---------------+-------------------------+-------------------------+--------------------+---------------+
        Row < s_RegularBewith,e_Stop,                   s_PseudoExit,             none,                none            >,
        Row < s_RegularBewith,e_GetAwayFromDocking,     s_GetAwayFromDocking,     none,                none            >
        //  +-+---------------+-------------------------+-------------------------+--------------------+---------------+
    > {};
};
typedef msm::back::state_machine<Bewith> s_Bewith;

} //End of namespace rm_state_machine
} //End of namespace robot_manager
#endif //BEWITH_STATE_MACHINE_H
