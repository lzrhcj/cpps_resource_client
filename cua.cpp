
#include "cua.h"

CUA myUA;


CUA::CUA()
{
    client = UA_Client_new();
}

CUA::~CUA()
{
    UA_Client_delete(client);
}

void CUA::UA_connect_TCP()
{
    client = UA_Client_new();
    UA_ClientConfig_setDefault(UA_Client_getConfig(client));

    string equip_url="opc.tcp://"+ g_opcuaIP + ":"+ g_opcuaPort;

    m_equip_UA.lock();
    g_flag_IsEquipmentUAConnected = UA_Client_connect(client, equip_url.c_str());
    UA_Client_run_iterate(client,0);
    m_equip_UA.unlock();

    while(g_flag_IsMainRunning)
    {
        if(g_flag_IsEquipmentUAConnected != UA_STATUSCODE_GOOD)
        {

            if(g_flag_IsEquipmentUAConnected != UA_STATUSCODE_GOOD)
            {
                printf("重连设备的UA_Server\n");
                //重连
                m_equip_UA.lock();
                g_flag_IsEquipmentUAConnected = UA_Client_connect(client, equip_url.c_str());
                UA_Client_run_iterate(client,0);
                m_equip_UA.unlock();
            }
            this_thread::sleep_for(chrono::seconds(2));

        }
        else
        {
            g_flag_IsEquipmentUAConnected = UA_STATUSCODE_GOOD;
        }

        this_thread::sleep_for(chrono::seconds(2));
    }
}


void CUA::UA_readvalue_thrd(bool isrun,string param,string *result)//读节点
{
    if(isrun == true)
    {
        while(g_flag_IsMainRunning && isrun)
        {
            string nodeid = myDB.equip_info.guid + param;

            //没用，因为连的还是cnc的server，就算节点对了也连不过去Denso或keba的server。
            //这里又不能切换去连别的server。因为别的server智能一个客户端去连

//            //若是CNC2，则equip_info.guid变为 Denso的 ac8496d6-c7ee-4512-8e0f-e0ee2ccf0a82
//            //因为CNC2需要Denso的guid
//            if(myDB.equip_info.guid == "609a96ff-85ee-41e9-8ed3-3a8218ece0d6")
//            {
//                nodeid = "ac8496d6-c7ee-4512-8e0f-e0ee2ccf0a82" + param;
//            }

//            //若是CNC1，则equip_info.guid变为 Keba的 5509aade-0b05-45a3-b7be-d0569972204e
//            //因为CNC1需要Keba的guid
//            if(myDB.equip_info.guid == "c5e616d2-89de-4e91-be05-f7111e06fb92")
//            {
//                nodeid = "5509aade-0b05-45a3-b7be-d0569972204e" + param;
//            }


            UA_NodeId NodeID = UA_NODEID_STRING(3, (char *)nodeid.c_str());

            UA_Variant Value;
            UA_Variant_init(&Value);

            m_equip_UA.lock();
            g_flag_IsEquipmentUAConnected = UA_Client_readValueAttribute(client, NodeID, &Value);
            m_equip_UA.unlock();

            if (g_flag_IsEquipmentUAConnected == UA_STATUSCODE_GOOD && Value.type == &UA_TYPES[UA_TYPES_STRING])
            {
                UA_String rawdata = *(UA_String*)(Value.data);
                char *tmp_data = new char[rawdata.length + 1];
                memset(tmp_data, 0, rawdata.length + 1);
                memcpy(tmp_data, rawdata.data, rawdata.length);
                *result = tmp_data;
                delete[] tmp_data;
                UA_Variant_clear(&Value);
            }

            this_thread::sleep_for(chrono::milliseconds(50));

        }
    }
}

string CUA::UA_readvalue(string param)//读节点
{
    string ret_UA_readvalue = "";

    if(g_flag_IsMainRunning == true)
    {
        string nodeid = myDB.equip_info.guid + param;

        UA_NodeId NodeID = UA_NODEID_STRING(3, (char *)nodeid.c_str());

        UA_Variant Value;
        UA_Variant_init(&Value);

        m_equip_UA.lock();
        g_flag_IsEquipmentUAConnected = UA_Client_readValueAttribute(client, NodeID, &Value);
        m_equip_UA.unlock();

        if (g_flag_IsEquipmentUAConnected == UA_STATUSCODE_GOOD && Value.type == &UA_TYPES[UA_TYPES_STRING])
        {
            UA_String rawdata = *(UA_String*)(Value.data);
            char *tmp_data = new char[rawdata.length + 1];
            memset(tmp_data, 0, rawdata.length + 1);
            memcpy(tmp_data, rawdata.data, rawdata.length);
            ret_UA_readvalue = tmp_data;
            delete[] tmp_data;
            UA_Variant_clear(&Value);

            return ret_UA_readvalue;
        }
    }
    return ret_UA_readvalue;
}


UA_StatusCode CUA::UA_writevalue(string param, string input_data)
{
    UA_StatusCode ret_UA_writevalue = -1;


    string nodeid = myDB.equip_info.guid + param;
    //string nodeid = "fa586e0c-9f2b-45eb-995b-1954c77a79d6:demo-RFID_flag@Value";


    //没用，因为连的还是cnc的server，就算节点对了也连不过去Denso或keba的server。
    //这里又不能切换去连别的server。因为别的server智能一个客户端去连

//    //若是CNC2，则equip_info.guid变为 Denso的 ac8496d6-c7ee-4512-8e0f-e0ee2ccf0a82
//    //因为CNC2需要Denso的guid
//    if(myDB.equip_info.guid == "609a96ff-85ee-41e9-8ed3-3a8218ece0d6")
//    {
//        nodeid = "ac8496d6-c7ee-4512-8e0f-e0ee2ccf0a82" + param;
//    }

//    //若是CNC1，则equip_info.guid变为 Keba的 5509aade-0b05-45a3-b7be-d0569972204e
//    //因为CNC1需要Keba的guid
//    if(myDB.equip_info.guid == "c5e616d2-89de-4e91-be05-f7111e06fb92")
//    {
//        nodeid = "5509aade-0b05-45a3-b7be-d0569972204e" + param;
//    }

    UA_NodeId NodeID = UA_NODEID_STRING(3, (char *)nodeid.c_str());

    UA_Variant Value;
    UA_Variant_init(&Value);

    UA_String ua_input_data = UA_STRING((char *)input_data.c_str());
    UA_Variant_setScalar(&Value, &ua_input_data, &UA_TYPES[UA_TYPES_STRING]);

    m_equip_UA.lock();
    g_flag_IsEquipmentUAConnected = UA_Client_writeValueAttribute(client, NodeID, &Value);
    m_equip_UA.unlock();

    if (g_flag_IsEquipmentUAConnected == UA_STATUSCODE_GOOD)
    {

        printf("写入成功");
    }
    else
    {
        printf("写入失败");
    }

    return ret_UA_writevalue;
}

//void CUA::UA_read_all_value(string equip_type)
//{
//    while(g_flag_IsMainRunning)
//    {
//        TYPE_EQUIPMENT my_equipment = myDB.get_TYPE_EQUIPMENT_Enum(equip_type);

//        switch(my_equipment)
//        {

//        case MACHINE_UPLOAD_BOX:
//        {
//            MACHINE_UPLOAD_BOX_PARAM machine_upload_box_param;

//            machine_upload_box_info.value_cap_input_cmd =
//                    UA_readvalue(myDB.equip_info.guid + machine_upload_box_param.cap_input_cmd);
//            machine_upload_box_info.value_columns =
//                    UA_readvalue(myDB.equip_info.guid + machine_upload_box_param.columns);
//            machine_upload_box_info.value_current_status =
//                    UA_readvalue(myDB.equip_info.guid + machine_upload_box_param.current_status);
//            machine_upload_box_info.value_RFID_readdata =
//                    UA_readvalue(myDB.equip_info.guid + machine_upload_box_param.RFID_readdata);
//        }
//            break;
//        case CONVEYOR:
//        {
//            CONVEYOR_PARAM conveyor_param;
//            conveyor_info.value_RFID_readdata =
//                    UA_readvalue(myDB.equip_info.guid + conveyor_param.RFID_readdata);
//        }
//            break;


//        case ROBOT:
//        {
//            ROBOT_PARAM robot_param;

//            robot_info.value_cap_input_cmd_pick =
//                    UA_readvalue(myDB.equip_info.guid + robot_param.cap_input_cmd_pick);

//            robot_info.value_cap_input_cmd_upload_material =
//                    UA_readvalue(myDB.equip_info.guid + robot_param.cap_input_cmd_upload_material);

//            robot_info.value_inputcommand =
//                    UA_readvalue(myDB.equip_info.guid + robot_param.inputcommand);
//            robot_info.value_current_status =
//                    UA_readvalue(myDB.equip_info.guid + robot_param.current_status);

//            robot_info.value_guard1_status =
//                    UA_readvalue(myDB.equip_info.guid + robot_param.guard1_status);

//            robot_info.value_guard2_status =
//                    UA_readvalue(myDB.equip_info.guid + robot_param.guard2_status);

//            robot_info.value_RFID_readdata =
//                    UA_readvalue(myDB.equip_info.guid + robot_param.RFID_readdata);

//            robot_info.value_switch_status =
//                    UA_readvalue(myDB.equip_info.guid + robot_param.switch_status);
//        }
//            break;

//        case CNC:
//        {
//            CNC_PARAM cnc_param;

//            cnc_info.value_cap_input_cmd=
//                    UA_readvalue(myDB.equip_info.guid + cnc_param.cap_input_cmd);

//            cnc_info.value_inputcommand=
//                    UA_readvalue(myDB.equip_info.guid + cnc_param.inputcommand);

//            cnc_info.value_current_status=
//                    UA_readvalue(myDB.equip_info.guid + cnc_param.current_status);

//        }
//            break;
//        case LASER:
//        {
//            LASER_PARAM laser_param;

//            laser_info.value_cap_input_cmd=
//                    UA_readvalue(myDB.equip_info.guid + laser_param.cap_input_cmd);

//            laser_info.value_current_status=
//                    UA_readvalue(myDB.equip_info.guid + laser_param.current_status);

//            laser_info.value_guard1_status=
//                    UA_readvalue(myDB.equip_info.guid + laser_param.guard1_status);

//            laser_info.value_guard2_status=
//                    UA_readvalue(myDB.equip_info.guid + laser_param.guard2_status);

//            laser_info.value_RFID_readdata=
//                    UA_readvalue(myDB.equip_info.guid + laser_param.RFID_readdata);

//            laser_info.value_switch_status=
//                    UA_readvalue(myDB.equip_info.guid + laser_param.switch_status);

//        }
//            break;




//        case MACHINE_UPLOAD_COVER:
//        {
//            MACHINE_UPLOAD_COVER_PARAM machine_upload_cover_param;

//            machine_upload_cover_info.value_cap_input_cmd =
//                    UA_readvalue(myDB.equip_info.guid + machine_upload_cover_param.cap_input_cmd);
//            machine_upload_cover_info.value_columns =
//                    UA_readvalue(myDB.equip_info.guid + machine_upload_cover_param.columns);
//            machine_upload_cover_info.value_current_status =
//                    UA_readvalue(myDB.equip_info.guid + machine_upload_cover_param.current_status);
//            machine_upload_cover_info.value_RFID_readdata =
//                    UA_readvalue(myDB.equip_info.guid + machine_upload_cover_param.RFID_readdata);


//        }
//            break;


//        case MACHINE_UNLOAD_BOX:
//        {
//            MACHINE_UNLOAD_BOX_PARAM machine_unload_box_param;
//            machine_unload_box_info.value_current_status =
//                    UA_readvalue(myDB.equip_info.guid + machine_unload_box_param.current_status);

//            machine_unload_box_info.value_RFID_readdata =
//                    UA_readvalue(myDB.equip_info.guid + machine_unload_box_param.RFID_readdata);

//        }
//            break;


//        case AGV:
//        {


//        }
//            break;

//        case NOPE:
//        {


//        }
//            break;

//        }

//        //必须快速的去读值
//        this_thread::sleep_for(chrono::milliseconds(20));

//    }
//}
