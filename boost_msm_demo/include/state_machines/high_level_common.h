#ifndef HIGHLEVEL_STATE_MACHINE_H
#define HIGHLEVEL_STATE_MACHINE_H
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
//--High Level Feature--//
struct HighLevelState: public msm::front::state_machine_def<HighLevelState>, public CommonToStateMachine
{
    HighLevelState(HighLevelFeaturesEnum desired_feature_id) : high_level_feature_id_(desired_feature_id) 
    {
        std::cout <<D_COUT_CUS_BOLDBLUE<<"HighLevelState Constructor ID: "<<high_level_feature_id_<<D_COUT_CUS_RESET << std::endl;
    }
    ~HighLevelState() {
        std::cout <<D_COUT_CUS_BOLDBLUE<<"HighLevelState Destructor ID: "<<high_level_feature_id_<<D_COUT_CUS_RESET << std::endl;
    }

    /*------------------------------ Variables ------------------------------*/
    EndConditionEnum end_condition_ = EndConditionEnum::End_ABORT_GENERAL;

    /*------------------------------ User Interface ------------------------------*/
    HighLevelFeaturesEnum getHighLevelFeatureID() {return high_level_feature_id_;}

    /*------------------------------ FSM Interface ------------------------------*/
    // Replaces the default no-transition response.
    template <class FSM,class Event>
    void no_transition(Event const& e, FSM&, int state)
    {
        std::cout <<D_COUT_CUS_BOLDBLUE<<"HighLevelState - no transition from state: "<<state<<" on event: "<<typeid(e).name()<<D_COUT_CUS_RESET<< std::endl;
    }

    /*------------------------------ States ------------------------------*/
    // LeaveDocking MSM
    struct GetAwayFromDocking: public msm::front::state_machine_def<GetAwayFromDocking>, public CommonToStateMachine
    {
        // Variables
        EndConditionEnum end_condition_ = EndConditionEnum::End_ABORT_GENERAL;

        /*------------------------------ FSM Interface ------------------------------*/
        typedef mpl::vector1<f_GetAwayFromDocking> flag_list; //On Flags
        template <class Event,class FSM>
        void on_entry(Event const& evt, FSM&) {
            std::cout <<D_COUT_CUS_BOLDBLUE<<"entering GetAwayFromDocking MSM"<<D_COUT_CUS_RESET;
            end_condition_ = EndConditionEnum::End_ABORT_GENERAL;
            EntryFunc();
        }
        template <class Event,class FSM>
        void on_exit(Event const&,FSM& ) {
            std::cout <<D_COUT_CUS_BOLDBLUE<<"leaving GetAwayFromDocking MSM"<<D_COUT_CUS_RESET;
            //ExitFunc(end_condition_);
        }

        /*------------------------------ Actions ------------------------------*/
        struct a_CompleteProcedure {
            template <class Event, class Fsm, class SourceState, class TargetState>
            void operator()(Event const&, Fsm& fsm, SourceState& src, TargetState&)
            {
                std::cout <<D_COUT_CUS_BLUE<<"GetAwayFromDocking a_CompleteProcedure"<<D_COUT_CUS_RESET;
                src.end_condition_ = EndConditionEnum::End_COMPLETE;
            }
        };
        struct a_AbortProcedure {
            template <class Event, class Fsm, class SourceState, class TargetState>
            void operator()(Event const&, Fsm& fsm, SourceState& src, TargetState&)
            {
                std::cout <<D_COUT_CUS_BLUE<<"GetAwayFromDocking a_AbortProcedure"<<D_COUT_CUS_RESET;
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

        /*------------------------------ States ------------------------------*/
        //Pseudo Exit
        struct s_PseudoExitStop:exit_pseudo_state <e_Stop> {};
        struct s_PseudoExitComplete:exit_pseudo_state <e_LeavingDockingEnd> {};

        class s_LeavingDocking : public msm::front::state<>
        {
        public:
            template <class Event,class FSM>
            void on_entry(Event const&, FSM&) {std::cout <<D_COUT_CUS_BOLDBLUE<<"entering s_LeavingDocking State"<<D_COUT_CUS_RESET;}
            template <class Event,class FSM>
            void on_exit(Event const&, FSM&) {std::cout <<D_COUT_CUS_BOLDBLUE<<"leaving s_LeavingDocking State"<<D_COUT_CUS_RESET;}
        };

        // the initial state. Must be defined
        typedef s_LeavingDocking initial_state;

        /*------------------------------ Internal Transition Table ------------------------------*/
        struct internal_transition_table : mpl::vector<
            //                    Event                     Action                    Guard
            //       +------------+-------------------------+-------------------------+---------------+
            Internal <            e_GetAwayFromDocking,     none,                     none            >,
            Internal <            e_HighLevelFeatureEND,    a_AbortProcedure,         none            >
            //       +------------+-------------------------+-------------------------+---------------+
        > {};

        /*------------------------------ Transition Table ------------------------------*/
        struct transition_table : mpl::vector<
            //    Start           Event                     Next                      Action               Guard
            //  +-+---------------+-------------------------+-------------------------+--------------------+---------------+
            Row < s_LeavingDocking, e_Stop,                 s_PseudoExitStop,         none,                none            >,
            Row < s_LeavingDocking, e_LeavingDockingEnd,    s_PseudoExitComplete,     none,                none            >
            //  +-+---------------+-------------------------+-------------------------+--------------------+---------------+
        > {};

        private:
    };
    typedef msm::back::state_machine<GetAwayFromDocking> s_GetAwayFromDocking;

private:
    HighLevelFeaturesEnum high_level_feature_id_;
};

/************************************** VARIABLE DECALRE ****************************************************/
/************************************** FUNCTION PROTOTYPE **************************************************/

} //End of namespace rm_state_machine
} //End of namespace robot_manager
#endif //HIGHLEVEL_STATE_MACHINE_H
