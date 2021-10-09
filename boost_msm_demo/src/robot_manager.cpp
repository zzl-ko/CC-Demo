/************************************** INCLUDE *************************************************************/
#include <thread>

#include "robot_manager.h"

/************************************** NAMESPACE ***********************************************************/
using namespace std::chrono_literals;

namespace robot_manager {
/************************************** CONST DEFINE ********************************************************/
/************************************** MACRO DEFINE ********************************************************/
/************************************** EMUN DEFINE *********************************************************/
/************************************** TYPE DEFINE *********************************************************/

/************************************** GLOBAL VARIABLE DECLARE *********************************************/
robot_manager::RobotManager * CommonToStateMachine::rm;
MainStateMachine              RobotManager::rm_msm_;

/************************************** LOCAL VARIABLE DECLARE **********************************************/
/************************************** EXTERN FUNCTION PROTOTYPE *******************************************/
/************************************** LOCAL FUNCTION PROTOTYPE ********************************************/

/************************************************************************************************************/
/*                                      PUBLIC FUNCTION IMPLEMENT                                           */
/************************************************************************************************************/
RobotManager::RobotManager()
{
    CommonToStateMachine::set_rm_object(this);
    std::thread(RobotManager::checkState).detach();
    rm_msm_.start();

    gotoTest();
    bewithTest();
    constraintFollowTest();
    dockingTest();
}
RobotManager::~RobotManager()
{
    rm_msm_.stop();
}

void RobotManager::checkState()
{
    int curr_state = -1;
    int last_state = -1;
    while (true) {
        if (rm_msm_.is_flag_active<f_Idle>()) {
            curr_state = 0;
        } else if (rm_msm_.is_flag_active<f_Goto>()) {
            curr_state = 2;
        } else if (rm_msm_.is_flag_active<f_Bewith>()) {
            curr_state = 3;
        } else if (rm_msm_.is_flag_active<f_ConstraintFollow>()) {
            curr_state = 4;
        }

        if (last_state != curr_state) {
            std::cout <<D_COUT_CUS_BOLDWHITE<<"@@@@ Curr state: "<<curr_state<<D_COUT_CUS_RESET <<std::endl;
        }
        last_state = curr_state;
    }
}

void RobotManager::gotoTest()
{
    std::cout <<D_COUT_CUS_BOLDWHITE<<"*** Start RobotManager::gotoTest ***"<<D_COUT_CUS_RESET <<std::endl;
    T_UserCommandMsg msg = {
        std::string(MOVE_CMD),
        std::string("bedroom"),
        std::string("status: start"),
        0, 0.0, 0.0, 0.0
    };

    rm_msm_.process_event(e_UserCommandRequest(msg));
    std::this_thread::sleep_for(2s);

    rm_msm_.process_event(e_GotoEnd(EndConditionEnum::End_ABORT_GENERAL));
    std::this_thread::sleep_for(1s);

    rm_msm_.process_event(e_UserCommandRequest(HighLevelFeaturesEnum::H_Goto, std::string("kitchen")));
    std::this_thread::sleep_for(2s);

    rm_msm_.process_event(e_HighLevelFeatureEND(EndConditionEnum::End_ABORT_USER));

    std::cout <<D_COUT_CUS_BOLDWHITE<<"*** EndOf RobotManager::gotoTest ***"<<D_COUT_CUS_RESET <<std::endl;
}
void RobotManager::bewithTest()
{
    std::cout <<D_COUT_CUS_BOLDWHITE<<"*** Start RobotManager::bewithTest ***"<<D_COUT_CUS_RESET <<std::endl;

	rm_msm_.process_event(e_UserCommandRequest(HighLevelFeaturesEnum::H_Bewith));
    std::this_thread::sleep_for(2s);

    std::cout <<D_COUT_CUS_BOLDWHITE<<"*** EndOf RobotManager::bewithTest ***"<<D_COUT_CUS_RESET <<std::endl;
}
void RobotManager::constraintFollowTest()
{
    std::cout <<D_COUT_CUS_BOLDWHITE<<"*** Start RobotManager::constraintFollowTest ***"<<D_COUT_CUS_RESET <<std::endl;
    std::cout <<D_COUT_CUS_BOLDWHITE<<"*** EndOf RobotManager::constraintFollowTest ***"<<D_COUT_CUS_RESET <<std::endl;
}
void RobotManager::dockingTest()
{
    std::cout <<D_COUT_CUS_BOLDWHITE<<"*** EndOf RobotManager::dockingTest ***"<<D_COUT_CUS_RESET <<std::endl;

    rm_msm_.process_event(e_StartDocking());
    std::this_thread::sleep_for(2s);
    rm_msm_.process_event(e_DockingEnd(EndConditionEnum::End_COMPLETE));

    std::cout <<D_COUT_CUS_BOLDWHITE<<"*** EndOf RobotManager::dockingTest ***"<<D_COUT_CUS_RESET <<std::endl;
}

/************************************************************************************************************/
/*                                      PRIVATE FUNCTION IMPLEMENT                                          */
/************************************************************************************************************/

} //EndOf of namespace robot_manager
