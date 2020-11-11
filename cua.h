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
    string inputcommand = "/AAS/Static/Composition/CNC/AAS/Dynamic/InputCmd:value@Value";//����ָ��ɶ�д��
    string current_status = "/AAS/Static/Composition/CNC/AAS/Dynamic/Status:value@Value";//��ǰ״̬��ֻ����
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
    }; //****����ODG����

    string cap_input_cmd_pick = "/AAS/Capability_Pick/Input/InputCmd:value@Value";
    string cap_input_cmd_upload_material = "/AAS/Capability_UploadMaterial/Input/InputCmd:value@Value";

    string inputcommand = "/AAS/Static/Composition/Robot/AAS/Dynamic/InputCmd:value@Value";//����ָ��ɶ�д��
    string current_status = "/AAS/Static/Composition/Robot/AAS/Dynamic/Status:value@Value";//��ǰ״̬��ֻ����
    string guard1_status = "/AAS/Static/Composition/Cylinder_1/AAS/Dynamic/Status:value@Value";//����1״̬���ɶ�д��
    string guard2_status = "/AAS/Static/Composition/Cylinder_2/AAS/Dynamic/Status:value@Value";//����2״̬���ɶ�д��
    string flag_RFID = "/AAS/Static/Composition/RFIDReader/AAS/Dynamic/Status:value@Value";//RFID��־λ���ɶ�д��
    string RFID_readdata = "/AAS/Static/Composition/RFIDReader/AAS/Dynamic/ReadData:value@Value";//RFID�������ݣ�ֻ����
    string RFID_writedata = "/AAS/Static/Composition/RFIDReader/AAS/Dynamic/WriteData:value@Value";//RFIDд�����ݣ��ɶ�д��
    string switch_status = "/AAS/Static/Composition/PhotoelectricSwitch/AAS/Dynamic/Status:value@Value";//��翪��״̬��ֻ����
    string gripper = "/AAS/Static/Composition/Gripper/AAS/Dynamic/Status:value@Value";//��צץȡ���ͷ�(�ɶ�д��
    string suction = "/AAS/Static/Composition/Nozzle/AAS/Dynamic/Status:value@Value";//���쿪�գ��ɶ�д��
};
struct ROBOT_INFO
{
    string value_cap_input_cmd_pick;
    string value_cap_input_cmd_upload_material;

    string value_inputcommand;//����ָ��ɶ�д��
    string value_current_status;//��ǰ״̬��ֻ����
    string value_guard1_status;//����1״̬���ɶ�д��
    string value_guard2_status;//����2״̬���ɶ�д��
    string value_flag_RFID;//RFID��־λ���ɶ�д��
    string value_RFID_readdata;//RFID�������ݣ�ֻ����
    string value_RFID_writedata;//RFIDд�����ݣ��ɶ�д��
    string value_switch_status;//��翪��״̬��ֻ����
    string value_gripper;//��צץȡ���ͷ�(�ɶ�д��
    string value_suction;//���쿪�գ��ɶ�д��
};

//MACHINE_UPLOAD_BOX
struct MACHINE_UPLOAD_BOX_PARAM
{
    string cap_input_cmd = "/AAS/Capability_UploadBox/Input/InputCmd:value@Value";

    string columns = "/AAS/Static/Composition/UploadBoxMachine/AAS/Dynamic/InputCmd:value@Value";//�����������ɶ�д��
    string current_status = "/AAS/Static/Composition/UploadBoxMachine/AAS/Dynamic/Status:value@Value";//��ǰ״̬��ֻ����
    string flag_RFID = "/AAS/Static/Composition/RFIDReader/AAS/Dynamic/Status:value@Value";//RFID��־λ���ɶ�д��
    string RFID_readdata = "/AAS/Static/Composition/RFIDReader/AAS/Dynamic/ReadData:value@Value";//RFID�������ݣ�ֻ����
    string RFID_writedata = "/AAS/Static/Composition/RFIDReader/AAS/Dynamic/WriteData:value@Value";//RFIDд�����ݣ��ɶ�д��
};

struct MACHINE_UPLOAD_BOX_INFO
{
    string value_cap_input_cmd;

    string value_columns ;//�����������ɶ�д��
    string value_current_status ;//��ǰ״̬��ֻ����
    string value_flag_RFID ;//RFID��־λ���ɶ�д��
    string value_RFID_readdata ;//RFID�������ݣ�ֻ����
    string value_RFID_writedata ;//RFIDд�����ݣ��ɶ�д��
};


//MACHINE_UNLOAD_BOX
struct MACHINE_UNLOAD_BOX_PARAM
{
    string inputcommand = "/AAS/Static/Composition/UnloadBoxMachine/AAS/Dynamic/InputCmd:value@Value";//ץȡָ��ɶ�д��
    string current_status = "/AAS/Static/Composition/UnloadBoxMachine/AAS/Dynamic/Status:value@Value";//��ǰ״̬��ֻ����
    string flag_RFID = "/AAS/Static/Composition/RFIDReader/AAS/Dynamic/Status:value@Value";//RFID��־λ���ɶ�д��
    string RFID_readdata = "/AAS/Static/Composition/RFIDReader/AAS/Dynamic/ReadData:value@Value";//RFID�������ݣ�ֻ����
    string RFID_writedata = "/AAS/Static/Composition/RFIDReader/AAS/Dynamic/WriteData:value@Value";//RFIDд�����ݣ��ɶ�д��
};
struct MACHINE_UNLOAD_BOX_INFO
{
    string value_inputcommand;//ץȡָ��ɶ�д��
    string value_current_status;//��ǰ״̬��ֻ����
    string value_flag_RFID;//RFID��־λ���ɶ�д��
    string value_RFID_readdata;//RFID�������ݣ�ֻ����
    string value_RFID_writedata;//RFIDд�����ݣ��ɶ�д��
};


//MACHINE_UPLOAD_COVER
struct MACHINE_UPLOAD_COVER_PARAM
{
    string cap_input_cmd = "/AAS/Capability_UploadCover/Input/InputCmd:value@Value";

    string columns = "/AAS/Static/Composition/UploadCoverMachine/AAS/Dynamic/InputCmd:value@Value";//�ϸ��������ɶ�д��
    string current_status = "/AAS/Static/Composition/UploadCoverMachine/AAS/Dynamic/Status:value@Value";//��ǰ״̬��ֻ����
    string flag_RFID = "/AAS/Static/Composition/RFIDReader/AAS/Dynamic/Status:value@Value";//RFID��־λ���ɶ�д��
    string RFID_readdata = "/AAS/Static/Composition/RFIDReader/AAS/Dynamic/ReadData:value@Value";//RFID�������ݣ�ֻ����
    string RFID_writedata = "/AAS/Static/Composition/RFIDReader/AAS/Dynamic/WriteData:value@Value";//RFIDд�����ݣ��ɶ�д��
};
struct MACHINE_UPLOAD_COVER_INFO
{
    string value_cap_input_cmd;

    string value_columns;//�ϸ��������ɶ�д��
    string value_current_status;//��ǰ״̬��ֻ����
    string value_flag_RFID;//RFID��־λ���ɶ�д��
    string value_RFID_readdata;//RFID�������ݣ�ֻ����
    string value_RFID_writedata;//RFIDд�����ݣ��ɶ�д��
};

//LASER
struct LASER_PARAM
{
    //�ı�ѡ���������ɶ�д��
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

    string current_status = "/AAS/Static/Composition/LaserLabelingMachine/AAS/Dynamic/Status:value@Value";//��ǰ״̬��ֻ����
    string guard1_status = "/AAS/Static/Composition/Cylinder_1/AAS/Dynamic/Status:value@Value";//����1״̬���ɶ�д��
    string guard2_status = "/AAS/Static/Composition/Cylinder_2/AAS/Dynamic/Status:value@Value";//����2״̬���ɶ�д��
    string flag_RFID = "/AAS/Static/Composition/RFIDReader/AAS/Dynamic/Status:value@Value";//RFID��־λ���ɶ�д��
    string RFID_readdata = "/AAS/Static/Composition/RFIDReader/AAS/Dynamic/ReadData:value@Value";//RFID�������ݣ�ֻ����
    string RFID_writedata = "/AAS/Static/Composition/RFIDReader/AAS/Dynamic/WriteData:value@Value";//RFIDд�����ݣ��ɶ�д��
    string switch_status = "/AAS/Static/Composition/PhotoelectricSwitch/AAS/Dynamic/Status:value@Value";//��翪��״̬��ֻ����
};

struct LASER_INFO
{
    string value_cap_input_cmd;

    string value_current_status;//��ǰ״̬��ֻ����
    string value_guard1_status;//����1״̬���ɶ�д��
    string value_guard2_status;//����2״̬���ɶ�д��
    string value_flag_RFID;//RFID��־λ���ɶ�д��
    string value_RFID_readdata;//RFID�������ݣ�ֻ����
    string value_RFID_writedata;//RFIDд�����ݣ��ɶ�д��
    string value_switch_status;//��翪��״̬��ֻ����

};


//CONVEYOR
struct CONVEYOR_PARAM
{
    string start_stop = "/AAS/Static/Composition/Conveyor/AAS/Dynamic/ON_OFF:value@Value";//���ʹ���ͣ���ɶ�д��
    string corner_switch = "/AAS/Static/Composition/Conveyor/AAS/Dynamic/PathChoice:value@Value";//���䵲������ɶ�д��
    string flag_RFID = "/AAS/Static/Composition/RFIDReader/AAS/Dynamic/Status:value@Value";//RFID��־λ���ɶ�д��
    string RFID_readdata = "/AAS/Static/Composition/RFIDReader/AAS/Dynamic/ReadData:value@Value";//RFID�������ݣ�ֻ����
    string RFID_writedata = "/AAS/Static/Composition/RFIDReader/AAS/Dynamic/WriteData:value@Value";//RFIDд�����ݣ��ɶ�д��
    string cylinder_status = "/AAS/Static/Composition/Cylinder/AAS/Dynamic/Status:value@Value";//�赲���ף��ɶ�д��
};
struct CONVEYOR_INFO
{
    string value_start_stop;//���ʹ���ͣ���ɶ�д��
    string value_corner_switch;//���䵲������ɶ�д��
    string value_flag_RFID;//RFID��־λ���ɶ�д��
    string value_RFID_readdata;//RFID�������ݣ�ֻ����
    string value_RFID_writedata;//RFIDд�����ݣ��ɶ�д��
    string value_cylinder_status;//�赲���ף��ɶ�д��
};

//AGV
struct AGV_PARAM
{
    string cuurent_coord = "/AAS/Static/Composition/AGV/AAS/Dynamic/CurrentLocation:value@Value";//��ǰ���ֻ꣨����
    string target_coord = "/AAS/Static/Composition/AGV/AAS/Dynamic/TargetLocation:value@Value";//Ŀ��λ�ã��ɶ�д��
    string identified_coord = "/AAS/Static/Composition/AGV/AAS/Dynamic/ObjectLocation:value@Value";//ʶ�����꣨�ɶ�д��
    string flag_isarrived = "/AAS/Static/Composition/AGV/AAS/Dynamic/ArrivedStatus:value@Value";//��λ��־���ɶ�д��
    string flag_RFID = "/AAS/Static/Composition/RFIDReader/AAS/Dynamic/Status:value@Value";//RFID��־λ���ɶ�д��
    string RFID_readdata = "/AAS/Static/Composition/RFIDReader/AAS/Dynamic/ReadData:value@Value";//RFID�������ݣ�ֻ����
    string RFID_writedata = "/AAS/Static/Composition/RFIDReader/AAS/Dynamic/WriteData:value@Value";//RFIDд�����ݣ��ɶ�д��
};

struct AGV_INFO
{
    string value_cuurent_coord;//��ǰ���ֻ꣨����
    string value_target_coord;//Ŀ��λ�ã��ɶ�д��
    string value_identified_coord;//ʶ�����꣨�ɶ�д��
    string value_flag_isarrived;//��λ��־���ɶ�д��
    string value_flag_RFID;//RFID��־λ���ɶ�д��
    string value_RFID_readdata;//RFID�������ݣ�ֻ����
    string value_RFID_writedata;//RFIDд�����ݣ��ɶ�д��
};




/*��ȡUA������״̬*/
class CUA
{
public:

    CUA();
    ~CUA();

    UA_Client *client;

    void UA_connect_TCP();

    string UA_readvalue(string param);//���ڵ�

    void UA_readvalue_thrd(bool isrun, string param,string *result);//���ڵ�

    UA_StatusCode UA_writevalue(string param, string input_data);//д�ڵ�

    //�̣߳�һֱɨEquipment��ֵ
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
