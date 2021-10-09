#ifndef MSM_COMMON_H
#define MSM_COMMON_H

//#define BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
//#define BOOST_MPL_LIMIT_VECTOR_SIZE 30 //or whatever you need
//#define BOOST_MPL_LIMIT_MAP_SIZE  30 //or whatever you need
//#define FUSION_MAX_VECTOR_SIZE 20 // or whatever you need
/************************************** INCLUDE *************************************************************/
#include <iostream>
//--boost--//
#include <boost/mpl/vector/vector50.hpp>
// back-end
#include <boost/msm/back/state_machine.hpp>
// Compile Vs Running
#include <boost/msm/back/favor_compile_time.hpp>
// front-end
#include <boost/msm/front/state_machine_def.hpp>
#include <boost/msm/front/functor_row.hpp>
// manager state machine
#include "msm_events.h"
#include "msm_flags.h"
#include "utilities.h"

/************************************** NAMESPACE ***********************************************************/
namespace robot_manager {
class RobotManager;

namespace msm = boost::msm;
namespace mpl = boost::mpl;
using namespace msm::front;

namespace rm_state_machine {
/************************************** CONST DEFINE ********************************************************/
/************************************** MACRO DEFINE ********************************************************/

/************************************** EMUN DEFINE *********************************************************/
/************************************** TYPE DEFINE *********************************************************/
class CommonToStateMachine
{
public:
    CommonToStateMachine() {}
    static void set_rm_object(robot_manager::RobotManager *rm_object)
    {
    	rm = rm_object;
    }
    static robot_manager::RobotManager* get_rm_object() {return rm;}

    virtual void EntryFunc() {return;}
    virtual void ExitFunc() {return;}

    virtual void handleCompleteProcedure() {return;}
    virtual void handleAbortProcedure() {return;}

protected:
    static robot_manager::RobotManager *rm;
};


} //End of namespace rm_state_machine
} //End of namespace robot_manager
#endif //COMMON_STATE_MACHINE_H
