/************************************** INCLUDE *************************************************************/
#include "state_machines/docking_state_machine.h"
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
void Docking::EntryFunc()
{
    rm->docking_obj_.Start();
}

void Docking::ExitFunc(EndConditionEnum end_condition)
{
    std::cout <<D_COUT_CUS_YELLOW<<"Docking::ExitFunc - end_condition_type: "<<end_condition<<D_COUT_CUS_RESET <<std::endl;
    if (end_condition == EndConditionEnum::End_COMPLETE) {
        handleCompleteProcedure();
    }
    else { //abort for any reason
        handleAbortProcedure(end_condition);
    }
}

void Docking::handleCompleteProcedure()
{
    std::cout <<D_COUT_CUS_YELLOW<<"Docking::handleCompleteProcedure"<<D_COUT_CUS_RESET <<std::endl;
    rm->docking_obj_.Complete();
}

void Docking::handleAbortProcedure(EndConditionEnum abort_condition)
{
    std::cout <<D_COUT_CUS_YELLOW<<"Docking::handleAbortProcedure, abort_condition: "<<(int)abort_condition<<D_COUT_CUS_RESET <<std::endl;
    rm->docking_obj_.Abort(abort_condition);
}


} //End of namespace rm_state_machine
} //End of namespace robot_manager
