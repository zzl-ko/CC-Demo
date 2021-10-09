#ifndef MSM_EVENTS_H
#define MSM_EVENTS_H
/************************************** INCLUDE *************************************************************/
#include <iostream>

/************************************** CONST DEFINE ********************************************************/
/************************************** MACRO DEFINE ********************************************************/
//Robot Manager Commands
#define None_CMD                "none"
#define MOVE_CMD                "move"
#define BeWith_CMD              "bewith"
#define ConstraintFollow_CMD    "constraint_follow"
/************************************** EMUN DEFINE *********************************************************/
// A "complicated" event type that carries some data.
// H_ stands for High Level
enum HighLevelFeaturesEnum
{
    H_Idle                = 0,
    H_Goto,
    H_Bewith,
    H_ConstraintFollow,

    NumOfFeatures
};

//--High Level Feature END--//
/**
 * @brief The EndConditionEnum enum
 * need to add enums by current order. (complete before abort coditions.)
 * if we add another condition which not abort nor complete - please be aware you have to change the abort Guards in all state machines
 * Complete < Abort < num of options
 */
enum EndConditionEnum
{
    End_COMPLETE          = 0,
    End_ABORT_GENERAL     = 1,
    End_ABORT_USER        = 2,
    End_ABORT_NO_MOVEMENT = 3,
    End_ABORT_TIMEOUT     = 4,

    NumOfOptions
};

const std::unordered_map <std::string, HighLevelFeaturesEnum> parseStringCommand {
    {MOVE_CMD,             HighLevelFeaturesEnum::H_Goto},
    {BeWith_CMD,           HighLevelFeaturesEnum::H_Bewith},
    {ConstraintFollow_CMD, HighLevelFeaturesEnum::H_ConstraintFollow}
};

/************************************** TYPE DEFINE *********************************************************/
typedef struct 
{
    std::string mainCommand;
    std::string subCommand;
    std::string status;
    int32_t     a;
    float       f;
    float       x;
    float       y;
} T_UserCommandMsg;

struct e_Activate
{
    e_Activate() :
        main_command_("none"),
        sub_command_("none"),
        status_("none"),
        a_(0),
        f_(0),
        x_(0),
        y_(0)
    {}

    //variables
    //acording to arduino_msg::command - TODO - change command type
    std::string main_command_;
    std::string sub_command_;
    std::string status_;
    int32_t a_;
    float f_;
    float x_;
    float y_;

    //EndCondition
    EndConditionEnum end_condition_ = End_ABORT_GENERAL;
};

class e_UserCommandRequest
{

protected:
    HighLevelFeaturesEnum desired_feature_enum_;

public:
    // 为修正编译警告: "warning: ‘xxx’ will be initialized after [-Wreorder]"
    // 需将变量定义顺序(从上到下)与初始化顺序一一对应
    e_UserCommandRequest(T_UserCommandMsg command_msg) :
        desired_feature_enum_(parseStringCommand.at(command_msg.mainCommand)),
        end_condition_(EndConditionEnum::End_ABORT_GENERAL),
        main_command_(command_msg.mainCommand),
        sub_command_(command_msg.subCommand),
        status_(command_msg.status),
        a_(command_msg.a),
        f_(command_msg.f),
        x_(command_msg.x),
        y_(command_msg.y)
    {}

    e_UserCommandRequest(HighLevelFeaturesEnum desired_command) :

        desired_feature_enum_(desired_command),
        end_condition_(EndConditionEnum::End_ABORT_GENERAL),
        main_command_("none"),
        sub_command_("none"),
        status_("none"),
        a_(0),
        f_(0),
        x_(0),
        y_(0)
    {}

    e_UserCommandRequest(HighLevelFeaturesEnum desired_command, std::string sub_cmd) :
        desired_feature_enum_(desired_command),
        end_condition_(EndConditionEnum::End_ABORT_GENERAL),
        main_command_("none"),
        sub_command_(sub_cmd),
        status_("none"),
        a_(0),
        f_(0),
        x_(0),
        y_(0)
    {}

    e_UserCommandRequest(HighLevelFeaturesEnum desired_command, EndConditionEnum end_condition_type) :
        desired_feature_enum_(desired_command),
        end_condition_(end_condition_type),
        main_command_("none"),
        sub_command_("none"),
        status_("none"),
        a_(0),
        f_(0),
        x_(0),
        y_(0)
    {}

    //functions
    //--getters--//
    HighLevelFeaturesEnum DesiredFeature() const {return desired_feature_enum_;}

    //variables
    //EndCondition
    EndConditionEnum end_condition_;
    //acording to arduino_msg::command - TODO - change command type
    std::string main_command_;
    std::string sub_command_;
    std::string status_;
    int32_t a_;
    float   f_;
    float   x_;
    float   y_;
};

class e_SelfTriggeredCommand : public e_UserCommandRequest
{
public:
    e_SelfTriggeredCommand(HighLevelFeaturesEnum desired_command) : e_UserCommandRequest(desired_command) {}
    e_SelfTriggeredCommand(HighLevelFeaturesEnum desired_command, EndConditionEnum end_condition_type) : e_UserCommandRequest(desired_command, end_condition_type) {}
};

class e_HighLevelFeatureEND
{
public:
    e_HighLevelFeatureEND(EndConditionEnum end_condition_type) : end_condition_(end_condition_type) {}

    EndConditionEnum end_condition_;
};

class e_GotoEnd //can't be child of e_HighLevelFeatureEND because they treated the same in the transition tables
{
public:
    e_GotoEnd(EndConditionEnum end_condition_type) : end_condition_(end_condition_type) {}

    EndConditionEnum end_condition_;
};

class e_Stop
{
public:
    e_Stop() : end_condition_ (EndConditionEnum::End_COMPLETE) {}
    // Event propergation -the class has to have the conversion constructors for the possible events to enter the same target state.
    e_Stop(e_HighLevelFeatureEND const&) : end_condition_(EndConditionEnum::End_COMPLETE) {}

    e_Stop(EndConditionEnum end_condition_type) : end_condition_(end_condition_type) {}

    EndConditionEnum end_condition_;
};


/*------------------------------------- Start Docking Event ----------------------------------------------*/
class e_StartDocking
{
public:
    e_StartDocking() {}
};

class e_DockingEnd
{
public:
    e_DockingEnd (EndConditionEnum end_condition_type) : end_condition_(end_condition_type) {}

    EndConditionEnum end_condition_;
};
/*------------------------------------- EndOf Docking Event ----------------------------------------------*/


/*------------------------------------- Start Leave Docking Event ----------------------------------------------*/
class e_GetAwayFromDocking
{
public:
    e_GetAwayFromDocking() {}
};

class e_LeavingDockingEnd
{
public:
    e_LeavingDockingEnd (EndConditionEnum end_condition_type) : end_condition_(end_condition_type) {}

    EndConditionEnum end_condition_;
};
/*------------------------------------- EndOf Leave Docking Event ----------------------------------------------*/

#endif //MSM_EVENTS_H
