/************************************** INCLUDE *************************************************************/
#include "state_machines/goto_state_machine.h"
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
void Goto::EntryFunc(std::string location_name)
{
    rm->goto_obj_.Start(location_name);
}

void Goto::ExitFunc(EndConditionEnum end_condition)
{
    std::cout <<D_COUT_CUS_YELLOW<<"Goto::ExitFunc - end_condition_type: "<<end_condition<<D_COUT_CUS_RESET <<std::endl;
    if (end_condition == EndConditionEnum::End_COMPLETE) {
        handleCompleteProcedure();
    }
    else { //abort for any reason
        handleAbortProcedure(end_condition);
    }
}

void Goto::handleCompleteProcedure()
{
    std::cout <<D_COUT_CUS_YELLOW<<"Goto::handleCompleteProcedure"<<D_COUT_CUS_RESET <<std::endl;
    rm->goto_obj_.Complete();
}

void Goto::handleAbortProcedure(EndConditionEnum abort_condition)
{
    std::cout <<D_COUT_CUS_YELLOW<<"Goto::handleAbortProcedure, abort_condition: "<<(int)abort_condition<<D_COUT_CUS_RESET <<std::endl;
    rm->goto_obj_.Abort(abort_condition);
}


} //End of namespace rm_state_machine
} //End of namespace robot_manager
