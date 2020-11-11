#ifndef CEQUIPMENT_H
#define CEQUIPMENT_H

#include "open62541.h"

#include "cdb.h"


extern string g_opcuaIP;
extern string g_opcuaPort;

//CNC
struct CNC_PARAM
{
    string cap_input_cmd = "/AAS/Capability_Carving/Input/InputCmd:value@Value";
    string inputcommand = "/AAS/Static/Composition/CNC/AAS/Dynamic/InputCmd:value@Value";//输入指令（可读写）
    string current_status = "/AAS/Static/Composition/CNC/AAS/Dynamic/Status:value@Value";//当前状态（只读）
};
struct CNC_INFO
{
    string value_cap_input_cmd;
    string value_inputcommand;
    string value_current_status ;
};


//ROBOT
struct ROBOT_PARAM
{
    string file_input[3] =
    {
        "/AAS/Static/Composition/Robot/AAS/Dynamic/ProductOption_1:value@Value"
        ,"/AAS/Static/Composition/Robot/AAS/Dynamic/ProductOption_2:value@Value"
        ,"/AAS/Static/Composition/Robot/AAS/Dynamic/ProductOption_3:value@Value"
    }; //****用于ODG上料

    string cap_input_cmd_pick = "/AAS/Capability_Pick/Input/InputCmd:value@Value";
    string cap_input_cmd_upload_material = "/AAS/Capability_UploadMaterial/Input/InputCmd:value@Value";

    string inputcommand = "/AAS/Static/Composition/Robot/AAS/Dynamic/InputCmd:value@Value";//输入指令（可读写）
    string current_status = "/AAS/Static/Composition/Robot/AAS/Dynamic/Status:value@Value";//当前状态（只读）
    string guard1_status = "/AAS/Static/Composition/Cylinder_1/AAS/Dynamic/Status:value@Value";//挡板1状态（可读写）
    string guard2_status = "/AAS/Static/Composition/Cylinder_2/AAS/Dynamic/Status:value@Value";//挡板2状态（可读写）
    string flag_RFID = "/AAS/Static/Composition/RFIDReader/AAS/Dynamic/Status:value@Value";//RFID标志位（可读写）
    string RFID_readdata = "/AAS/Static/Composition/RFIDReader/AAS/Dynamic/ReadData:value@Value";//RFID读出数据（只读）
    string RFID_writedata = "/AAS/Static/Composition/RFIDReader/AAS/Dynamic/WriteData:value@Value";//RFID写入数据（可读写）
    string switch_status = "/AAS/Static/Composition/PhotoelectricSwitch/AAS/Dynamic/Status:value@Value";//光电开关状态（只读）
    string gripper = "/AAS/Static/Composition/Gripper/AAS/Dynamic/Status:value@Value";//夹爪抓取或释放(可读写）
    string suction = "/AAS/Static/Composition/Nozzle/AAS/Dynamic/Status:value@Value";//吸嘴开闭（可读写）
};
struct ROBOT_INFO
{
    string value_cap_input_cmd_pick;
    string value_cap_input_cmd_upload_material;

    string value_inputcommand;//输入指令（可读写）
    string value_current_status;//当前状态（只读）
    string value_guard1_status;//挡板1状态（可读写）
    string value_guard2_status;//挡板2状态（可读写）
    string value_flag_RFID;//RFID标志位（可读写）
    string value_RFID_readdata;//RFID读出数据（只读）
    string value_RFID_writedata;//RFID写入数据（可读写）
    string value_switch_status;//光电开关状态（只读）
    string value_gripper;//夹爪抓取或释放(可读写）
    string value_suction;//吸嘴开闭（可读写）
};

//MACHINE_UPLOAD_BOX
struct MACHINE_UPLOAD_BOX_PARAM
{
    string cap_input_cmd = "/AAS/Capability_UploadBox/Input/InputCmd:value@Value";

    string columns = "/AAS/Static/Composition/UploadBoxMachine/AAS/Dynamic/InputCmd:value@Value";//上料列数（可读写）
    string current_status = "/AAS/Static/Composition/UploadBoxMachine/AAS/Dynamic/Status:value@Value";//当前状态（只读）
    string flag_RFID = "/AAS/Static/Composition/RFIDReader/AAS/Dynamic/Status:value@Value";//RFID标志位（可读写）
    string RFID_readdata = "/AAS/Static/Composition/RFIDReader/AAS/Dynamic/ReadData:value@Value";//RFID读出数据（只读）
    string RFID_writedata = "/AAS/Static/Composition/RFIDReader/AAS/Dynamic/WriteData:value@Value";//RFID写入数据（可读写）
};

struct MACHINE_UPLOAD_BOX_INFO
{
    string value_cap_input_cmd;

    string value_columns ;//上料列数（可读写）
    string value_current_status ;//当前状态（只读）
    string value_flag_RFID ;//RFID标志位（可读写）
    string value_RFID_readdata ;//RFID读出数据（只读）
    string value_RFID_writedata ;//RFID写入数据（可读写）
};


//MACHINE_UNLOAD_BOX
struct MACHINE_UNLOAD_BOX_PARAM
{
    string inputcommand = "/AAS/Static/Composition/UnloadBoxMachine/AAS/Dynamic/InputCmd:value@Value";//抓取指令（可读写）
    string current_status = "/AAS/Static/Composition/UnloadBoxMachine/AAS/Dynamic/Status:value@Value";//当前状态（只读）
    string flag_RFID = "/AAS/Static/Composition/RFIDReader/AAS/Dynamic/Status:value@Value";//RFID标志位（可读写）
    string RFID_readdata = "/AAS/Static/Composition/RFIDReader/AAS/Dynamic/ReadData:value@Value";//RFID读出数据（只读）
    string RFID_writedata = "/AAS/Static/Composition/RFIDReader/AAS/Dynamic/WriteData:value@Value";//RFID写入数据（可读写）
};
struct MACHINE_UNLOAD_BOX_INFO
{
    string value_inputcommand;//抓取指令（可读写）
    string value_current_status;//当前状态（只读）
    string value_flag_RFID;//RFID标志位（可读写）
    string value_RFID_readdata;//RFID读出数据（只读）
    string value_RFID_writedata;//RFID写入数据（可读写）
};


//MACHINE_UPLOAD_COVER
struct MACHINE_UPLOAD_COVER_PARAM
{
    string cap_input_cmd = "/AAS/Capability_UploadCover/Input/InputCmd:value@Value";

    string columns = "/AAS/Static/Composition/UploadCoverMachine/AAS/Dynamic/InputCmd:value@Value";//上盖列数（可读写）
    string current_status = "/AAS/Static/Composition/UploadCoverMachine/AAS/Dynamic/Status:value@Value";//当前状态（只读）
    string flag_RFID = "/AAS/Static/Composition/RFIDReader/AAS/Dynamic/Status:value@Value";//RFID标志位（可读写）
    string RFID_readdata = "/AAS/Static/Composition/RFIDReader/AAS/Dynamic/ReadData:value@Value";//RFID读出数据（只读）
    string RFID_writedata = "/AAS/Static/Composition/RFIDReader/AAS/Dynamic/WriteData:value@Value";//RFID写入数据（可读写）
};
struct MACHINE_UPLOAD_COVER_INFO
{
    string value_cap_input_cmd;

    string value_columns;//上盖列数（可读写）
    string value_current_status;//当前状态（只读）
    string value_flag_RFID;//RFID标志位（可读写）
    string value_RFID_readdata;//RFID读出数据（只读）
    string value_RFID_writedata;//RFID写入数据（可读写）
};

//LASER
struct LASER_PARAM
{
    //文本选择列数（可读写）
    string file_input[8] =
    {
        "/AAS/Static/Composition/LaserLabelingMachine/AAS/Dynamic/FileOption_1:value@Value"
        ,"/AAS/Static/Composition/LaserLabelingMachine/AAS/Dynamic/FileOption_2:value@Value"
        ,"/AAS/Static/Composition/LaserLabelingMachine/AAS/Dynamic/FileOption_3:value@Value"
        ,"/AAS/Static/Composition/LaserLabelingMachine/AAS/Dynamic/FileOption_4:value@Value"
        ,"/AAS/Static/Composition/LaserLabelingMachine/AAS/Dynamic/FileOption_5:value@Value"
        ,"/AAS/Static/Composition/LaserLabelingMachine/AAS/Dynamic/FileOption_6:value@Value"
        ,"/AAS/Static/Composition/LaserLabelingMachine/AAS/Dynamic/FileOption_7:value@Value"
        ,"/AAS/Static/Composition/LaserLabelingMachine/AAS/Dynamic/FileOption_8:value@Value"
    };

    string cap_input_cmd = "/AAS/Capability_LaserLabeling/Input/InputCmd:value@Value";

    string current_status = "/AAS/Static/Composition/LaserLabelingMachine/AAS/Dynamic/Status:value@Value";//当前状态（只读）
    string guard1_status = "/AAS/Static/Composition/Cylinder_1/AAS/Dynamic/Status:value@Value";//挡板1状态（可读写）
    string guard2_status = "/AAS/Static/Composition/Cylinder_2/AAS/Dynamic/Status:value@Value";//挡板2状态（可读写）
    string flag_RFID = "/AAS/Static/Composition/RFIDReader/AAS/Dynamic/Status:value@Value";//RFID标志位（可读写）
    string RFID_readdata = "/AAS/Static/Composition/RFIDReader/AAS/Dynamic/ReadData:value@Value";//RFID读出数据（只读）
    string RFID_writedata = "/AAS/Static/Composition/RFIDReader/AAS/Dynamic/WriteData:value@Value";//RFID写入数据（可读写）
    string switch_status = "/AAS/Static/Composition/PhotoelectricSwitch/AAS/Dynamic/Status:value@Value";//光电开关状态（只读）
};

struct LASER_INFO
{
    string value_cap_input_cmd;

    string value_current_status;//当前状态（只读）
    string value_guard1_status;//挡板1状态（可读写）
    string value_guard2_status;//挡板2状态（可读写）
    string value_flag_RFID;//RFID标志位（可读写）
    string value_RFID_readdata;//RFID读出数据（只读）
    string value_RFID_writedata;//RFID写入数据（可读写）
    string value_switch_status;//光电开关状态（只读）

};


//CONVEYOR
struct CONVEYOR_PARAM
{
    string start_stop = "/AAS/Static/Composition/Conveyor/AAS/Dynamic/ON_OFF:value@Value";//传送带启停（可读写）
    string corner_switch = "/AAS/Static/Composition/Conveyor/AAS/Dynamic/PathChoice:value@Value";//拐弯挡板命令（可读写）
    string flag_RFID = "/AAS/Static/Composition/RFIDReader/AAS/Dynamic/Status:value@Value";//RFID标志位（可读写）
    string RFID_readdata = "/AAS/Static/Composition/RFIDReader/AAS/Dynamic/ReadData:value@Value";//RFID读出数据（只读）
    string RFID_writedata = "/AAS/Static/Composition/RFIDReader/AAS/Dynamic/WriteData:value@Value";//RFID写入数据（可读写）
    string cylinder_status = "/AAS/Static/Composition/Cylinder/AAS/Dynamic/Status:value@Value";//阻挡气缸（可读写）
};
struct CONVEYOR_INFO
{
    string value_start_stop;//传送带启停（可读写）
    string value_corner_switch;//拐弯挡板命令（可读写）
    string value_flag_RFID;//RFID标志位（可读写）
    string value_RFID_readdata;//RFID读出数据（只读）
    string value_RFID_writedata;//RFID写入数据（可读写）
    string value_cylinder_status;//阻挡气缸（可读写）
};

//AGV
struct AGV_PARAM
{
    string cuurent_coord = "/AAS/Static/Composition/AGV/AAS/Dynamic/CurrentLocation:value@Value";//当前坐标（只读）
    string target_coord = "/AAS/Static/Composition/AGV/AAS/Dynamic/TargetLocation:value@Value";//目标位置（可读写）
    string identified_coord = "/AAS/Static/Composition/AGV/AAS/Dynamic/ObjectLocation:value@Value";//识别坐标（可读写）
    string flag_isarrived = "/AAS/Static/Composition/AGV/AAS/Dynamic/ArrivedStatus:value@Value";//到位标志（可读写）
    string flag_RFID = "/AAS/Static/Composition/RFIDReader/AAS/Dynamic/Status:value@Value";//RFID标志位（可读写）
    string RFID_readdata = "/AAS/Static/Composition/RFIDReader/AAS/Dynamic/ReadData:value@Value";//RFID读出数据（只读）
    string RFID_writedata = "/AAS/Static/Composition/RFIDReader/AAS/Dynamic/WriteData:value@Value";//RFID写入数据（可读写）
};

struct AGV_INFO
{
    string value_cuurent_coord;//当前坐标（只读）
    string value_target_coord;//目标位置（可读写）
    string value_identified_coord;//识别坐标（可读写）
    string value_flag_isarrived;//到位标志（可读写）
    string value_flag_RFID;//RFID标志位（可读写）
    string value_RFID_readdata;//RFID读出数据（只读）
    string value_RFID_writedata;//RFID写入数据（可读写）
};




/*获取UA的连接状态*/
class CUA
{
public:

    CUA();
    ~CUA();

    UA_Client *client;

    void UA_connect_TCP();

    string UA_readvalue(string param);//读节点

    void UA_readvalue_thrd(bool isrun, string param,string *result);//读节点

    UA_StatusCode UA_writevalue(string param, string input_data);//写节点

    //线程，一直扫Equipment的值
    void UA_read_all_value(string equip_type);


public:


    UA_StatusCode flag_IsUARunning;

    UA_StatusCode flag_IsUARead;
    UA_StatusCode flag_IsUAWrite;

    bool flag_product_new;

    mutex m_equip_UA;
    mutex m_product_UA;

    MACHINE_UPLOAD_BOX_INFO machine_upload_box_info;
    CONVEYOR_INFO conveyor_info;
    ROBOT_INFO robot_info;
    CNC_INFO cnc_info;
    LASER_INFO laser_info;
    MACHINE_UPLOAD_COVER_INFO machine_upload_cover_info;
    MACHINE_UNLOAD_BOX_INFO machine_unload_box_info;


    AGV_INFO agv_info;


protected:

private:

};

extern CUA myUA;


#endif // CEQUIPMENT_H
