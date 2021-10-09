#ifndef MSM_FLAGS_H
#define MSM_FLAGS_H

#include <iostream>

//---Flags---//
class flag_type{};

//High Level States
struct f_Idle : public flag_type {};
struct f_Active : public flag_type {};
struct f_Goto : public flag_type {};
struct f_Bewith : public flag_type {};
struct f_ConstraintFollow : public flag_type {};

//Demo
struct f_Dreidel : public flag_type {};
struct f_TiltNod : public flag_type {};

//Docking
struct f_GetAwayFromDocking{};
struct f_Docking{};

//Charging
struct f_Charging{};
struct f_NotCharging{};

#endif //MSM_FLAGS_H
