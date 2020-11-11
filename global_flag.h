#ifndef GLOBAL_FLAG_H
#define GLOBAL_FLAG_H

#include "open62541.h"

#include <iostream>

using namespace std;

extern volatile bool g_flag_IsMainRunning;

extern volatile bool g_flag_IsMySQLConnected;

extern volatile UA_StatusCode g_flag_IsEquipmentUAConnected;

extern volatile UA_StatusCode g_flag_IsProductUAConnected;



extern bool thrd_machine_upload_box;
extern bool thrd_conveyor;
extern bool thrd_machine_unload_box;


extern bool thrd_robot_ODG;
extern bool thrd_robot_others;



extern bool thrd_cnc;
extern bool thrd_laser;
extern bool thrd_machine_upload_cover;
extern bool thrd_agv;
extern bool thrd_nope;







extern string g_opcuaIP;

extern string g_opcuaPort;

#endif // GLOBAL_FLAG_H
