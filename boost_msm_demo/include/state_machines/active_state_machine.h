#ifndef ACTIVE_STATE_MACHINE_H
#define ACTIVE_STATE_MACHINE_H
/************************************** INCLUDE *************************************************************/
#include "bewith_state_machine.h"
#include "bewith_constraint_state_machine.h"
#include "docking_state_machine.h"
#include "goto_state_machine.h"

/************************************** NAMESPACE ***********************************************************/
namespace msm = boost::msm;
namespace mpl = boost::mpl;
using namespace msm::front;

using namespace robot_manager::rm_state_machine;

namespace rm_state_machine {
/************************************** CONST DEFINE ********************************************************/
/************************************** MACRO DEFINE ********************************************************/
/************************************** EMUN DEFINE *********************************************************/
/************************************** TYPE DEFINE *********************************************************/
//---Active State Mechine---//
class Active : public msm::front::state_machine_def<Active>, public CommonToStateMachine
{
public:
    /*------------------------------ User Interface ------------------------------*/
    template <class FSM>
    HighLevelFeaturesEnum getCurrentHighLevelFeatureID(FSM& fsm)
    {
        HighLevelFeaturesEnum current_high_level_feature_id;
        const int* active_states = fsm.current_state();
        int int_active_state = active_states[0];
        std::cout <<D_COUT_CUS_BOLDGREEN<<"getHighLevelFeatureID - active state: "<<int_active_state<<D_COUT_CUS_RESET <<std::endl;
        current_high_level_feature_id = (HighLevelFeaturesEnum)int_active_state;
        std::cout <<D_COUT_CUS_BOLDGREEN<<"getHighLevelFeatureID - current_high_level_feature_id: "<<current_high_level_feature_id<<D_COUT_CUS_RESET <<std::endl;
        return current_high_level_feature_id;
    }

    /*------------------------------ FSM Interface ------------------------------*/
    typedef mpl::vector1<f_Active> flag_list; //On Flags
    template <class Event, class FSM>
    void on_entry(Event const& evt, FSM&) {std::cout <<D_COUT_CUS_BOLDGREEN<<"entering Active MSM"<<D_COUT_CUS_RESET <<std::endl;}
    template <class Event, class FSM>
    void on_exit(Event const&, FSM&) {std::cout <<D_COUT_CUS_BOLDGREEN<<"leaving Active MSM"<<D_COUT_CUS_RESET <<std::endl;}
    // Replaces the default no-transition response.
    template <class FSM, class Event>
    void no_transition(Event const& e, FSM&, int state)
    {
        std::cout <<D_COUT_CUS_BOLDGREEN<<"no transition from state: "<<state<<" on event: "<<typeid(e).name()<<D_COUT_CUS_RESET <<std::endl;
    }

    /*------------------------------ Actions ------------------------------*/
    // transition to other state
    struct a_TransitionProcedure {
        template <class Event, class Fsm, class SourceState, class TargetState>
        void operator()(Event const& evt, Fsm& fsm, SourceState& src, TargetState&)
        {
            std::cout <<D_COUT_CUS_GREEN<<"Active a_TransitionProcedure"<<D_COUT_CUS_RESET <<std::endl;
            src.end_condition_ = evt.end_condition_;
        }
    };

    // transition to complete (Idle)
    struct a_CompleteProcedure {
        template <class Event, class Fsm, class SourceState, class TargetState>
        void operator()(Event const&, Fsm& fsm, SourceState& src, TargetState&)
        {
            std::cout <<D_COUT_CUS_GREEN<<"Active a_CompleteProcedure"<<D_COUT_CUS_RESET <<std::endl;
            src.handleCompleteProcedure();
        }
    };

    // transition to abort (Idle)
    struct a_AbortProcedure {
        template <class Event, class Fsm, class SourceState, class TargetState>
        void operator()(Event const&, Fsm& fsm, SourceState& src, TargetState&)
        {
            std::cout <<D_COUT_CUS_GREEN<<"Active a_AbortProcedure"<<D_COUT_CUS_RESET <<std::endl;
            src.handleAbortProcedure();
        }
    };

    /*------------------------------ Guards ------------------------------*/
    struct g_Goto {
        template <class Event, class Fsm, class SourceState, class TargetState>
        bool operator()(Event const& evt, Fsm&, SourceState&, TargetState&)
        {
            return evt.DesiredFeature() == HighLevelFeaturesEnum::H_Goto;
        }
    };
    struct g_Bewith {
        template <class Event, class Fsm, class SourceState, class TargetState>
        bool operator()(Event const& evt, Fsm&, SourceState&, TargetState&)
        {
            return evt.DesiredFeature() == HighLevelFeaturesEnum::H_Bewith;
        }
    };
    struct g_ConstraintFollow {
        template <class Event, class Fsm, class SourceState, class TargetState>
        bool operator()(Event const& evt, Fsm&, SourceState&, TargetState&)
        {
            return evt.DesiredFeature() == HighLevelFeaturesEnum::H_ConstraintFollow;
        }
    };
    struct g_2ndActive {
        template <class Event, class Fsm, class SourceState, class TargetState>
        bool operator()(Event const& evt, Fsm&, SourceState&, TargetState&)
        {
            // bool dreidel_request = evt.DesiredFeature() == HighLevelFeaturesEnum::H_Dreidel;
            // bool tilt_nod_request = evt.DesiredFeature() == HighLevelFeaturesEnum::H_TiltNod;
            // bool repose_request = evt.DesiredFeature() == HighLevelFeaturesEnum::H_RePose;
            bool constraint_follow_request = evt.DesiredFeature() == HighLevelFeaturesEnum::H_ConstraintFollow;
            bool indeed_2nd_active_request = false || constraint_follow_request;// || dreidel_request || tilt_nod_request || repose_request;
            return indeed_2nd_active_request;
        }
    };

    /*------------------------------ States ------------------------------*/
    //Pseudo Entry
    struct s_PseudoEntry:entry_pseudo_state<> {};
    //Pseudo Exit
    struct s_PseudoExit:exit_pseudo_state<e_Stop> {};

    class ActiveIdle : public msm::front::state<>
    {
    public:
        template <class Event, class FSM>
        void on_entry(Event const&, FSM&) {std::cout <<D_COUT_CUS_BOLDGREEN<<"entering ActiveIdle State"<<D_COUT_CUS_RESET <<std::endl;}
        template <class Event, class FSM>
        void on_exit(Event const&, FSM&) {std::cout <<D_COUT_CUS_BOLDGREEN<<"leaving ActiveIdle State"<<D_COUT_CUS_RESET <<std::endl;}
    };

    class HighLevelActive : public msm::front::state_machine_def<HighLevelActive>, public CommonToStateMachine
    {
    public:
        template <class Event, class FSM>
        void on_entry(Event const&, FSM&) {std::cout <<D_COUT_CUS_BOLDGREEN<<"entering HighLevelActive MSM"<<D_COUT_CUS_RESET <<std::endl;}
        template <class Event, class FSM>
        void on_exit(Event const&, FSM&) {std::cout <<D_COUT_CUS_BOLDGREEN<<"leaving HighLevelActive MSM"<<D_COUT_CUS_RESET <<std::endl;}
        // Replaces the default no-transition response.
        template <class FSM, class Event>
        void no_transition(Event const& e, FSM&, int state)
        {
            std::cout <<D_COUT_CUS_YELLOW<<"HighLevelActive no transition from state: "<<state<<" on event: "<<typeid(e).name()<<D_COUT_CUS_RESET <<std::endl;
        }

        // States
        //Pseudo Entry
        struct s_PsdEnt:entry_pseudo_state<> {};
        //Pseudo Exit
        struct s_PsdExt:exit_pseudo_state<e_Stop> {};

        class s_ActiveInit : public msm::front::state<>
        {
        public:
            template <class Event, class FSM>
            void on_entry(Event const&, FSM&) {std::cout <<D_COUT_CUS_BOLDGREEN<<"entering ActiveInitSt State"<<D_COUT_CUS_RESET <<std::endl;}
            template <class Event, class FSM>
            void on_exit(Event const&, FSM&) {std::cout <<D_COUT_CUS_BOLDGREEN<<"leaving ActiveInitSt State"<<D_COUT_CUS_RESET <<std::endl;}
        };

        // the initial state. Must be defined
        typedef s_ActiveInit initial_state;

        // Transition table for HighLevelActive
        struct transition_table : mpl::vector<
            //    Start           Event                     Next                      Action               Guard
            //  +-+---------------+-------------------------+-------------------------+--------------------+---------------+
            Row < s_ActiveInit,   e_Stop,                   s_PsdExt,                 none,                none            >,
            //  +-+---------------+-------------------------+-------------------------+--------------------+---------------+
            Row < s_PsdEnt,       e_Stop,                   s_PsdExt,                 none,                none            >,
            Row < s_PsdEnt,       e_UserCommandRequest,     s_Goto::entry_pt<Goto::s_PseudoEntry>,
                                                                                      none,                g_Goto          >,
            Row < s_PsdEnt,       e_UserCommandRequest,     s_Bewith,                 none,                g_Bewith        >
            //  +-+---------------+-------------------------+-------------------------+--------------------+---------------+
        > {};
    };
    //back-end
    typedef msm::back::state_machine<HighLevelActive> s_HighLevelActive;

    class HighLevelActive2 : public msm::front::state_machine_def<HighLevelActive2>, public CommonToStateMachine
    {
    public:
        template <class Event,class FSM>
        void on_entry(Event const& ,FSM&) {
            std::cout <<D_COUT_CUS_BOLDGREEN<<"entering s_HighLevelActive2 MSM"<<D_COUT_CUS_RESET <<std::endl;}
        template <class Event,class FSM>
        void on_exit(Event const&,FSM& ) {std::cout <<D_COUT_CUS_BOLDGREEN<<"leaving s_HighLevelActive2 MSM"<<D_COUT_CUS_RESET <<std::endl;}
        // Replaces the default no-transition response.
        template <class FSM,class Event>
        void no_transition(Event const& e, FSM&, int state)
        {
            std::cout <<D_COUT_CUS_YELLOW<<"HighLevelActive2 no transition from state: "<<state<<" on event: "<<typeid(e).name()<<D_COUT_CUS_RESET <<std::endl;
        }

        // States
        //Pseudo Entry
        struct s_PsdEnt:entry_pseudo_state<> {};
        // Pseudo Exit
        struct s_PsdExt:exit_pseudo_state <e_Stop> {};

        class s_ActiveInitSt2 : public msm::front::state<>
        {
        public:
            template <class Event,class FSM>
            void on_entry(Event const& ,FSM&) {
                std::cout <<D_COUT_CUS_BOLDGREEN<<"entering ActiveInitState2 State"<<D_COUT_CUS_RESET <<std::endl;}
            template <class Event,class FSM>
            void on_exit(Event const&,FSM& ) {std::cout <<D_COUT_CUS_BOLDGREEN<<"leaving ActiveInitState2 State"<<D_COUT_CUS_RESET <<std::endl;}
        };

        // the initial state. Must be defined
        typedef s_ActiveInitSt2 initial_state;

        // Transition table for HighLevelActive2
        struct transition_table : mpl::vector<
            //    Start           Event                     Next                      Action               Guard
            //  +-+---------------+-------------------------+-------------------------+--------------------+---------------+
            Row < s_ActiveInitSt2,e_Stop,                   s_PsdExt,                 none,                none            >,
            //  +-+---------------+-------------------------+-------------------------+--------------------+---------------+
            Row < s_PsdEnt,       e_Stop,                   s_PsdExt,                 none,                none            >,
            Row < s_PsdEnt,       e_UserCommandRequest,     s_ConstraintFollow,       s_ConstraintFollow::a_EntryProcedure,
                                                                                                           g_ConstraintFollow >
            //  +-+---------------+-------------------------+-------------------------+--------------------+---------------+
        > {};
    };
    //back-end
    typedef msm::back::state_machine<HighLevelActive2> s_HighLevelActive2;

    // the initial state of the Active. Must be defined
    typedef mpl::vector<s_HighLevelActive> initial_state;

    /*------------------------------ Internal Transition Table ------------------------------*/
    // Internal transition table for Active
    struct internal_transition_table : mpl::vector<
        //                    Event                     Action                    Guard
        //       +------------+-------------------------+-------------------------+---------------+
        Internal <            e_Activate,               none,                     none            >
        //       +------------+-------------------------+-------------------------+---------------+
    > {};

    /*------------------------------ Transition Table ------------------------------*/
    struct transition_table : mpl::vector<
        //    Start           Event                     Next                      Action               Guard
        //  +-+---------------+-------------------------+-------------------------+--------------------+---------------+
        Row < s_PseudoEntry,  e_UserCommandRequest,     s_HighLevelActive::entry_pt<HighLevelActive::s_PsdEnt>,
                                                                                  none,                none            >,
        Row < s_PseudoEntry,  e_UserCommandRequest,     s_HighLevelActive2::entry_pt<HighLevelActive2::s_PsdEnt>,
                                                                                  none,                g_2ndActive     >,
        Row < s_PseudoEntry,  e_StartDocking,           s_Docking,                none,                none            >,
        //  +-+---------------+-------------------------+-------------------------+--------------------+---------------+
        Row < s_Docking,      e_Stop,                   s_PseudoExit,             none,                none            >,
        //  +-+---------------+-------------------------+-------------------------+--------------------+---------------+
        Row < s_HighLevelActive,
                              e_Stop,                   s_PseudoExit,             none,                none            >,
        Row < s_HighLevelActive,
                              e_UserCommandRequest,     s_HighLevelActive::entry_pt<HighLevelActive::s_PsdEnt>,
                                                                                  none,                none            >,
        Row < s_HighLevelActive,
                              e_UserCommandRequest,     s_HighLevelActive2::entry_pt<HighLevelActive2::s_PsdEnt>,
                                                                                  none,                g_2ndActive     >,
        //  +-+---------------+-------------------------+-------------------------+--------------------+---------------+
        Row < s_HighLevelActive2,
                              e_Stop,                   s_PseudoExit,             none,                none            >,
        Row < s_HighLevelActive2,
                              e_UserCommandRequest,     s_HighLevelActive2::entry_pt<HighLevelActive2::s_PsdEnt>,
                                                                                  none,                none            >,
        Row < s_HighLevelActive2,
                              e_UserCommandRequest,     s_HighLevelActive::entry_pt<HighLevelActive::s_PsdEnt>,
                                                                                  none,                none            >
        //  +-+---------------+-------------------------+-------------------------+--------------------+---------------+
    > {};
};
// back-end
typedef msm::back::state_machine<Active> s_Active;

/************************************** VARIABLE DECALRE ****************************************************/
/************************************** FUNCTION PROTOTYPE **************************************************/

} //End of namespace rm_state_machine
#endif //ACTIVE_STATE_MACHINE_H
