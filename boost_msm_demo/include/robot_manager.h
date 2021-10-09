#ifndef ROBOT_MANAGER_H
#define ROBOT_MANAGER_H
/************************************** INCLUDE *************************************************************/
#include "state_machines/main_state_machine.h"

/************************************** NAMESPACE ***********************************************************/
using namespace rm_state_machine;

namespace robot_manager {
/************************************** CONST DEFINE ********************************************************/
/************************************** MACRO DEFINE ********************************************************/
/************************************** EMUN DEFINE *********************************************************/
/************************************** TYPE DEFINE *********************************************************/
class ConstraintFollowMgr
{
public:
	ConstraintFollowMgr() {};
	~ConstraintFollowMgr() {};

	void Start()
	{
		std::cout <<D_COUT_CUS_WHITE<<"ConstraintFollowMgr::Start"<<D_COUT_CUS_RESET <<std::endl;
	}
	void Complete()
	{
		std::cout <<D_COUT_CUS_WHITE<<"ConstraintFollowMgr::Complete"<<D_COUT_CUS_RESET <<std::endl;
	}
	void Abort(EndConditionEnum abort_condition)
	{
		std::cout <<D_COUT_CUS_WHITE<<"ConstraintFollowMgr::Abort: "<<abort_condition<<D_COUT_CUS_RESET <<std::endl;
	}
};

class BewithMgr
{
public:
	BewithMgr() {};
	~BewithMgr() {};

	void Start()
	{
		std::cout <<D_COUT_CUS_WHITE<<"BewithMgr::Start"<<D_COUT_CUS_RESET <<std::endl;
	}
	void Complete()
	{
		std::cout <<D_COUT_CUS_WHITE<<"BewithMgr::Complete"<<D_COUT_CUS_RESET <<std::endl;
	}
	void Abort(EndConditionEnum abort_condition)
	{
		std::cout <<D_COUT_CUS_WHITE<<"BewithMgr::Abort: "<<abort_condition<<D_COUT_CUS_RESET <<std::endl;
	}
};

class DockingMgr
{
public:
	DockingMgr() {};
	~DockingMgr() {};
	
	void Start()
	{
		std::cout <<D_COUT_CUS_WHITE<<"DockingMgr::Start"<<D_COUT_CUS_RESET <<std::endl;
	}
	void Complete()
	{
		std::cout <<D_COUT_CUS_WHITE<<"DockingMgr::Complete"<<D_COUT_CUS_RESET <<std::endl;
	}
	void Abort(EndConditionEnum abort_condition)
	{
		std::cout <<D_COUT_CUS_WHITE<<"DockingMgr::Abort: "<<abort_condition<<D_COUT_CUS_RESET <<std::endl;
	}
};

class GotoMgr
{
public:
	GotoMgr() {};
	~GotoMgr() {};
	
	void Start(std::string location_name)
	{
		std::cout <<D_COUT_CUS_WHITE<<"GotoMgr::Start"<<D_COUT_CUS_RESET <<std::endl;
	}
	void Complete()
	{
		std::cout <<D_COUT_CUS_WHITE<<"GotoMgr::Complete"<<D_COUT_CUS_RESET <<std::endl;
	}
	void Abort(EndConditionEnum abort_condition)
	{
		std::cout <<D_COUT_CUS_WHITE<<"GotoMgr::Abort: "<<abort_condition<<D_COUT_CUS_RESET <<std::endl;
	}
};

class RobotManager
{
public:
    RobotManager();
    ~RobotManager();

    static void checkState();

    void gotoTest();
    void bewithTest();
    void dockingTest();
    void constraintFollowTest();

    static MainStateMachine rm_msm_;

    GotoMgr             goto_obj_;
    DockingMgr          docking_obj_;
    BewithMgr           bewith_obj_;
    ConstraintFollowMgr constraint_follow_obj_;
};

} //End of namespace robot_manager
#endif //ROBOT_MANAGER_H
