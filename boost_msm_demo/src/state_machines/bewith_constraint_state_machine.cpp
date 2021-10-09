/************************************** INCLUDE *************************************************************/
#include "state_machines/bewith_constraint_state_machine.h"
#include "robot_manager.h"

/************************************** NAMESPACE ***********************************************************/
namespace robot_manager {
namespace rm_state_machine {

/************************************** CONST DEFINE ********************************************************/
/************************************** MACRO DEFINE ********************************************************/
/************************************** EMUN DEFINE *********************************************************/
/************************************** TYPE DEFINE *********************************************************/

/************************************** GLOBAL VARIABLE DECLARE *********************************************/
/************************************** LOCAL VARIABLE DECLARE **********************************************/
/************************************** EXTERN FUNCTION PROTOTYPE *******************************************/
/************************************** LOCAL FUNCTION PROTOTYPE ********************************************/

/************************************************************************************************************/
/*                                      PUBLIC FUNCTION IMPLEMENT                                           */
/************************************************************************************************************/


/************************************************************************************************************/
/*                                      PRIVATE FUNCTION IMPLEMENT                                          */
/************************************************************************************************************/
void ConstraintFollow::EntryFunc()
{
    rm->constraint_follow_obj_.Start();
}

void ConstraintFollow::ExitFunc(EndConditionEnum end_condition)
{
    std::cout <<D_COUT_CUS_YELLOW<<"ConstraintFollow::ExitFunc - end_condition_type: "<<end_condition<<D_COUT_CUS_RESET <<std::endl;
    switch (end_condition) {
    case EndConditionEnum::End_COMPLETE:
        handleCompleteProcedure();
        break;
    case EndConditionEnum::End_ABORT_GENERAL:
    default:
        handleAbortProcedure(end_condition);
        break;
    }
}

void ConstraintFollow::handleCompleteProcedure()
{
    std::cout <<D_COUT_CUS_YELLOW<<"ConstraintFollow::handleCompleteProcedure"<<D_COUT_CUS_RESET <<std::endl;
    rm->constraint_follow_obj_.Complete();
}

void ConstraintFollow::handleAbortProcedure(EndConditionEnum end_condition)
{
    std::cout <<D_COUT_CUS_YELLOW<<"ConstraintFollow::handleAbortProcedure"<<D_COUT_CUS_RESET <<std::endl;
    rm->constraint_follow_obj_.Abort(end_condition);
}

} //End of namespace rm_state_machine
} //End of namespace robot_manager
