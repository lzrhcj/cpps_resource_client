#include "cua.h"

#include "cdb.h"

#include "global_flag.h"

volatile bool g_flag_IsMainRunning = true;

volatile bool g_flag_IsMySQLConnected = false;

volatile UA_StatusCode g_flag_IsEquipmentUAConnected;

volatile UA_StatusCode g_flag_IsProductUAConnected;

string g_opcuaIP;
string g_opcuaPort;


bool thrd_machine_upload_box = false;
bool thrd_conveyor = false;
bool thrd_machine_unload_box = false;
bool thrd_robot_ODG = false;
bool thrd_robot_others = false;

bool thrd_cnc = false;
bool thrd_laser = false;
bool thrd_machine_upload_cover = false;
bool thrd_agv = false;
bool thrd_nope = false;

const std::string GetCurrentSystemTime()
{
    auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm* ptm = localtime(&t);
    char date[60] = { 0 };
    sprintf(date, "%d-%02d-%02d %02d:%02d:%02d",
            (int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday,
            (int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);
    return move(std::string(date));
}


static void stopHandler(int sig)
{
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "received ctrl-c");

    myDB.~CDB();
    myUA.~CUA();

    g_flag_IsMainRunning = false;
    g_flag_IsMySQLConnected = false;

}

int main(int argc, char *argv[])
{

    signal(SIGINT, stopHandler);
    signal(SIGTERM, stopHandler);

    setbuf(stdout, NULL);

    if (argc == 1)
    {
        printf("�������豸ip���˿ڣ�--ip --ipport\n");
        printf("���磺192.168.137.221 4844");
        return 0;
    }

    if (argc == 2)
    {
        printf("������d�˿ں�\n");
        printf("���磺192.168.1.118 4844");
        return 0;
    }

    if (argc == 3)
    {
        g_opcuaIP = argv[1];
        g_opcuaPort = argv[2];
    }

    g_flag_IsMySQLConnected = myDB.init_mysql();

    myDB.getEquipmentInfofrommysql();

    //������ݿ��Ƿ���߲�����
    thread thrd_connect_mysql(&CDB::connect_mysql,&myDB);

    //����豸UA�Ƿ����Ӳ�����
    thread thrd_connect_equipment_ua(&CUA::UA_connect_TCP,&myUA);

    TYPE_EQUIPMENT my_equipment = myDB.get_TYPE_EQUIPMENT_Enum(myDB.equip_info.type);

    MACHINE_UPLOAD_BOX_PARAM machine_upload_box_param;

    CONVEYOR_PARAM conveyor_param;

    ROBOT_PARAM robot_param;

    MACHINE_UNLOAD_BOX_PARAM machine_unload_box_param;

    LASER_PARAM laser_param;

    MACHINE_UPLOAD_COVER_PARAM machine_upload_cover_param;

    switch(my_equipment)
    {
    case MACHINE_UPLOAD_BOX:
    {
        thrd_machine_upload_box = true;
    }
        break;

    case CONVEYOR:
    {
        thrd_conveyor = true;
    }
        break;

    case ROBOT:
    {
        if(myDB.equip_info.actual_name == "ODG")
        {
            thrd_robot_ODG = true;
        }
        else
        {
            thrd_robot_others = true;
        }

    }
        break;

    case CNC:
    {
        thrd_cnc = true;

    }
        break;
    case LASER:
    {
        thrd_laser = true;

    }
        break;

    case MACHINE_UPLOAD_COVER:
    {
        thrd_machine_upload_cover = true;
    }
        break;

    case MACHINE_UNLOAD_BOX:
    {
        thrd_machine_unload_box = true;
    }
        break;

    case AGV:
    {
        thrd_agv = false;

    }
        break;

    case NOPE:
    {
        thrd_nope = false;

    }
        break;
    }

    /*--�Ϻл��̣߳����ǲ����Ϻл���ֱ�����--*/


    thread thrd_UA_read_machine_upload_box_info_value_current_status
            (&CUA::UA_readvalue_thrd,&myUA,
             thrd_machine_upload_box,
             machine_upload_box_param.current_status,
             &myUA.machine_upload_box_info.value_current_status);

    thread thrd_UA_read_machine_upload_box_info_RFID_readdata
            (&CUA::UA_readvalue_thrd,&myUA,
             thrd_machine_upload_box,
             machine_upload_box_param.RFID_readdata,
             &myUA.machine_upload_box_info.value_RFID_readdata);

    thrd_UA_read_machine_upload_box_info_value_current_status.detach();
    thrd_UA_read_machine_upload_box_info_RFID_readdata.detach();


    /*--���ʹ��̣߳����ǲ��Ǵ��ʹ���ֱ�����--*/

    thread thrd_UA_read_conveyor_info_value_cylinder_status
            (&CUA::UA_readvalue_thrd,&myUA,
             thrd_conveyor,
             conveyor_param.cylinder_status,
             &myUA.conveyor_info.value_cylinder_status);

    thread thrd_UA_read_conveyor_info_value_RFID_readdata
            (&CUA::UA_readvalue_thrd,&myUA,
             thrd_conveyor,
             conveyor_param.RFID_readdata,
             &myUA.conveyor_info.value_RFID_readdata);

    thrd_UA_read_conveyor_info_value_cylinder_status.detach();
    thrd_UA_read_conveyor_info_value_RFID_readdata.detach();


    /*--������ODG�̣߳����ǲ��ǻ�����ODG��ֱ�����--*/

    thread thrd_UA_read_robot_ODG_info_value_switch_status
            (&CUA::UA_readvalue_thrd,&myUA,
             thrd_robot_ODG,
             robot_param.switch_status,
             &myUA.robot_info.value_switch_status);

    thread thrd_UA_read_robot_ODG_info_value_current_status
            (&CUA::UA_readvalue_thrd,&myUA,
             thrd_robot_ODG,
             robot_param.current_status,
             &myUA.robot_info.value_current_status);

    thread thrd_UA_read_robot_ODG_info_value_RFID_readdata
            (&CUA::UA_readvalue_thrd,&myUA,
             thrd_robot_ODG,
             robot_param.RFID_readdata,
             &myUA.robot_info.value_RFID_readdata);

    thrd_UA_read_robot_ODG_info_value_switch_status.detach();
    thrd_UA_read_robot_ODG_info_value_current_status.detach();
    thrd_UA_read_robot_ODG_info_value_RFID_readdata.detach();

    /*--�����������̣߳����ǻ�����ODG��ֱ�����--*/

    thread thrd_UA_read_robot_others_info_value_switch_status
            (&CUA::UA_readvalue_thrd,&myUA,
             thrd_robot_others,
             robot_param.switch_status,
             &myUA.robot_info.value_switch_status);

    thread thrd_UA_read_robot_others_info_value_RFID_readdata
            (&CUA::UA_readvalue_thrd,&myUA,
             thrd_robot_others,
             robot_param.RFID_readdata,
             &myUA.robot_info.value_RFID_readdata);

    thrd_UA_read_robot_others_info_value_switch_status.detach();

    thrd_UA_read_robot_others_info_value_RFID_readdata.detach();


    /*--�ºл��̣߳����ǲ����ºл���ֱ�����--*/

    thread thrd_UA_read_machine_unload_box_info_value_current_status
            (&CUA::UA_readvalue_thrd,&myUA,
             thrd_machine_unload_box,
             machine_unload_box_param.current_status,
             &myUA.machine_unload_box_info.value_current_status);

    thread thrd_UA_read_machine_unload_box_info_value_RFID_readdata
            (&CUA::UA_readvalue_thrd,&myUA,
             thrd_machine_unload_box,
             machine_unload_box_param.RFID_readdata,
             &myUA.machine_unload_box_info.value_RFID_readdata);


    thrd_UA_read_machine_unload_box_info_value_current_status.detach();
    thrd_UA_read_machine_unload_box_info_value_RFID_readdata.detach();


    /*--CNC�̣߳����ǲ���CNC��ֱ�����--*/

    //CNCһ����Ҫread�Ķ�û��
    //    thread thrd_UA_read_cnc_info_value_RFID_readdata
    //            (&CUA::UA_readvalue_thrd,&myUA,
    //             thrd_cnc,
    //             robot_param.RFID_readdata,
    //             &myUA.robot_info.value_RFID_readdata);

    //    thrd_UA_read_cnc_info_value_RFID_readdata.detach();


    /*--�����̣߳������Ǽ��⣬ֱ�����--*/

    thread thrd_UA_read_laser_info_value_switch_status
            (&CUA::UA_readvalue_thrd,&myUA,
             thrd_laser,
             laser_param.switch_status,
             &myUA.laser_info.value_switch_status);

    thread thrd_UA_read_laser_info_value_current_status
            (&CUA::UA_readvalue_thrd,&myUA,
             thrd_laser,
             laser_param.current_status,
             &myUA.laser_info.value_current_status);

    thread thrd_UA_read_laser_info_value_RFID_readdata
            (&CUA::UA_readvalue_thrd,&myUA,
             thrd_laser,
             laser_param.RFID_readdata,
             &myUA.laser_info.value_RFID_readdata);

    thrd_UA_read_laser_info_value_switch_status.detach();
    thrd_UA_read_laser_info_value_current_status.detach();
    thrd_UA_read_laser_info_value_RFID_readdata.detach();


    /*--�ϸǻ��̣߳��������ϸǣ�ֱ�����--*/

    thread thrd_UA_read_machine_upload_cover_info_value_current_status
            (&CUA::UA_readvalue_thrd,&myUA,
             thrd_machine_upload_cover,
             machine_upload_cover_param.current_status,
             &myUA.machine_upload_cover_info.value_current_status);

    thread thrd_UA_read_machine_upload_cover_info_RFID_readdata
            (&CUA::UA_readvalue_thrd,&myUA,
             thrd_machine_upload_cover,
             machine_upload_cover_param.RFID_readdata,
             &myUA.machine_upload_cover_info.value_RFID_readdata);

    thrd_UA_read_machine_upload_cover_info_value_current_status.detach();
    thrd_UA_read_machine_upload_cover_info_RFID_readdata.detach();


    /*���￪ʼд�߼�*/

    while(g_flag_IsMainRunning)
    {
        TYPE_EQUIPMENT my_equipment = myDB.get_TYPE_EQUIPMENT_Enum(myDB.equip_info.type);

        string str1, str2;


        this_thread::sleep_for(chrono::milliseconds(100));

        switch(my_equipment)
        {
        /*--�Ϻл��Ѳ������  10.25 16:08--*/
        case MACHINE_UPLOAD_BOX:
        {
            MACHINE_UPLOAD_BOX_PARAM machine_upload_box_param;

            printf("��ʼ�����Ϻл����߼�\n");

            //�ȴ����̵���
            printf("���ڵȴ����̵������豸Ϊ %s", myDB.equip_info.actual_name.c_str());

            //���̵������������޺���
            while (g_flag_IsMainRunning && myUA.machine_upload_box_info.value_current_status != "11")
            {
                this_thread::sleep_for(chrono::milliseconds(500));
            }

            //�޸��豸״̬Ϊworking
            g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,TABLE_EQUIPMENTINFO,{"status","working","guid",myDB.equip_info.guid});

            //RFID��־λ��1
            myUA.UA_writevalue(machine_upload_box_param.flag_RFID,"1");

            for(int t=0;t<10;t++)
            {
                //���RFID��ֵ
                if((myUA.machine_upload_box_info.value_RFID_readdata != "0")&&
                        (myUA.machine_upload_box_info.value_RFID_readdata != ""))
                {
                    //�����Ļ���break������ִ�������
                    break;
                }
                this_thread::sleep_for(chrono::milliseconds(500));
            }

            //�����Ƕ�����RFID���򵯳�������һ�㲻������������
            if((myUA.machine_upload_box_info.value_RFID_readdata == "0")||
                    (myUA.machine_upload_box_info.value_RFID_readdata == ""))
            {

                //�ȸĳ�0���ٷ���
                myUA.UA_writevalue(machine_unload_box_param.inputcommand,"0");

                //д��14����
                myUA.UA_writevalue(machine_upload_box_param.columns,"14");

                //�����޸��豸״̬Ϊready
                g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                         TABLE_EQUIPMENTINFO,{"status","ready",
                                                                              "guid",myDB.equip_info.guid});
                //RFID��־λ��0
                myUA.UA_writevalue(machine_upload_box_param.flag_RFID,"0");
                //�������switch-case,��������һ��
                break;
            }

            //���������ݶ���������guid���ó���Ʒ��ip���Լ�����deque
            myDB.getProductInfofrommysql(myUA.machine_upload_box_info.value_RFID_readdata);

            //RFID��־λ��0���رն�дRFID
            myUA.UA_writevalue(machine_upload_box_param.flag_RFID,"0");

            //������û�зֵ����κβ�Ʒ
            if(myDB.product_row == 0)
            {

                //�ȸĳ�0���ٷ���
                myUA.UA_writevalue(machine_unload_box_param.inputcommand,"0");

                //д��14����
                myUA.UA_writevalue(machine_upload_box_param.columns,"14");

                //�����޸��豸״̬Ϊready
                g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                         TABLE_EQUIPMENTINFO,{"status","ready",
                                                                              "guid",myDB.equip_info.guid});

                //RFID��־λ��0���رն�дRFID
                myUA.UA_writevalue(machine_upload_box_param.flag_RFID,"0");

                //�������switch-case
                break;
            }

            //���������ݲ�Ʒ��guid���ó��ò�Ʒ���豸�Ĳ���
            myDB.getOperationInfofrommysql(myDB.product_info.guid);

            //���������Ʒ����Ҫ����豸�ɻ�
            if(myDB.oper_row == 0)
            {
                //�ȸĳ�0���ٷ���
                myUA.UA_writevalue(machine_upload_box_param.columns,"0");

                //д��14����
                myUA.UA_writevalue(machine_upload_box_param.columns,"14");

                //�����޸��豸״̬Ϊready
                g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                         TABLE_EQUIPMENTINFO,{"status","ready",
                                                                              "guid",myDB.equip_info.guid});

                //�������switch-case
                break;
            }
            else
            {
                //Ҫ�������������Ļ���������һ������һ������˾����ڶ���
                for(int t = 0;t < myDB.oper_row;t++)
                {

                    if(myDB.deque_oper_info.at(t).status == "ready")
                    {
                        //�ò�����ready״̬
                        //�޸Ĳ���״̬Ϊworking
                        g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                 TABLE_OPERATIONINFO,{"status","ready",
                                                                                      "id",to_string(myDB.deque_oper_info.at(t).id)});

                        //ִ�����
                        printf("���̵�λ�����ڷ�������ָ�����Ϊ %s\n", myDB.deque_oper_info.at(t).param.c_str());


                        //д������
                        myUA.UA_writevalue(machine_upload_box_param.columns,
                                           myDB.deque_oper_info.at(t).param);


                        //��Ʒ״̬�ĳ�working
                        g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                 TABLE_PRODUCTINFO,{"status","working",
                                                                                    "guid",myDB.product_info.guid});

                        //�޸Ĳ���״̬Ϊworking
                        g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                 TABLE_OPERATIONINFO,{"status","working",
                                                                                      "id",to_string(myDB.deque_oper_info.at(t).id)});
                        string strCurTime = GetCurrentSystemTime();

                        //����״̬��ʱ��
                        g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                 TABLE_OPERATIONINFO,{"start_time",strCurTime,
                                                                                      "id",to_string(myDB.deque_oper_info.at(t).id)});


                        //��Ʒ����1�ĳ�working
                        g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                 TABLE_FEATUREINFO,{"status","working",
                                                                                    "id",to_string(myDB.deque_feature_info.at(0).id)});



                        //��Ʒ����1��ʱ��
                        g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                 TABLE_FEATUREINFO,{"start_time",strCurTime,
                                                                                    "id",to_string(myDB.deque_feature_info.at(0).id)});


                        //�ȵ������Ѿ����ڷ�������
                        while (g_flag_IsMainRunning && myUA.machine_upload_box_info.value_current_status != "12")
                        {
                            this_thread::sleep_for(chrono::milliseconds(500));
                        }

                        string strCurTime_2 = GetCurrentSystemTime();
                        //��Ʒ������߼�ʱ��
                        g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                 TABLE_PRODUCTINFO,{"actual_arrival_time",strCurTime_2,
                                                                                    "guid",myDB.product_info.guid});

                        //�ȸĳ�0���ٷ���
                        myUA.UA_writevalue(machine_upload_box_param.columns,"0");

                        //д��14����
                        myUA.UA_writevalue(machine_upload_box_param.columns,"14");


                        string strCurTime_3 = GetCurrentSystemTime();

                        //���Ʒ����״̬Ϊdone
                        g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                 TABLE_FEATUREINFO,{"status","done",
                                                                                    "id",to_string(myDB.deque_feature_info.at(0).id)});

                        //���Ʒ����״̬��ʱ��
                        g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                 TABLE_FEATUREINFO,{"end_time",strCurTime_3,
                                                                                    "id",to_string(myDB.deque_feature_info.at(0).id)});


                        //�޸Ĳ���״̬Ϊdone
                        g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                 TABLE_OPERATIONINFO,{"status","done",
                                                                                      "id",to_string(myDB.deque_oper_info.at(t).id)});

                        //�޸Ĳ���״̬��ʱ��
                        g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                 TABLE_OPERATIONINFO,{"end_time",strCurTime_3,
                                                                                      "id",to_string(myDB.deque_oper_info.at(t).id)});


                        //�����޸��豸״̬Ϊready
                        g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                 TABLE_EQUIPMENTINFO,{"status","ready",
                                                                                      "guid",myDB.equip_info.guid});

                    }
                    else
                    {
                        //����ready�Ļ�����һ��
                        continue;
                    }
                }

                //���������������ready״̬�������

                //�ȸĳ�0���ٷ���
                myUA.UA_writevalue(machine_upload_box_param.columns,"0");

                //д��14����
                myUA.UA_writevalue(machine_upload_box_param.columns,"14");

                //RFID��־λ��0���رն�дRFID
                myUA.UA_writevalue(machine_upload_box_param.flag_RFID,"0");

                //�����޸��豸״̬Ϊready
                g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                         TABLE_EQUIPMENTINFO,{"status","ready",
                                                                              "guid",myDB.equip_info.guid});

            }

        }
            break;

            /*--���ʹ�5������� 2020.10.25 17:30--*/
            /*--���ʹ�1������� 2020.10.26 01:30--*/
            /*--���ʹ�3������� 2020.10.26 01:30--*/
        case CONVEYOR:
        {
            CONVEYOR_PARAM conveyor_param;

            printf("��ʼ���д��ʹ�%i���߼�\n",myDB.equip_info.type_subindex);
            //�ȴ����̵���

            //�ȴ����̵���
            printf("���ڵȴ����̵������豸Ϊ %s\n", myDB.equip_info.actual_name.c_str());

            //��ʼ��д2���������
            myUA.UA_writevalue(conveyor_param.cylinder_status,"2");

            this_thread::sleep_for(chrono::milliseconds(500));

            //һֱ�ȵ�����Ӧ��
            while (g_flag_IsMainRunning && myUA.conveyor_info.value_cylinder_status != "1")
            {
                this_thread::sleep_for(chrono::milliseconds(500));
            }

            //�޸��豸״̬Ϊworking
            g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,TABLE_EQUIPMENTINFO,
            {"status","working","guid",myDB.equip_info.guid});

            //RFID��־λ��1
            myUA.UA_writevalue(conveyor_param.flag_RFID,"1");

            for(int t=0;t<10;t++)
            {
                //���RFID��ֵ
                if((myUA.conveyor_info.value_RFID_readdata != "0")&&
                        ((myUA.conveyor_info.value_RFID_readdata != "")))
                {
                    //�����Ļ���break������ִ�������
                    break;
                }
                this_thread::sleep_for(chrono::milliseconds(500));

            }

            //�����Ƕ�����RFID������У�ֱ�У�������һ�㲻������������
            if((myUA.conveyor_info.value_RFID_readdata == "0")||
                    ((myUA.conveyor_info.value_RFID_readdata == "")))
            {
                //д2,Ĭ��ֱ��
                myUA.UA_writevalue(conveyor_param.corner_switch,"2");

                //��һ��
                this_thread::sleep_for(chrono::seconds(1));

                //д2���������
                myUA.UA_writevalue(conveyor_param.cylinder_status,"2");

                //RFID��־λ��0���رն�дRFID
                myUA.UA_writevalue(conveyor_param.flag_RFID,"0");

                //�����޸��豸״̬Ϊready
                g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                         TABLE_EQUIPMENTINFO,{"status","ready",
                                                                              "guid",myDB.equip_info.guid});
                //�������switch-case
                break;
            }

            //���������ݶ���������guid���ó���Ʒ��ip���Լ�����deque
            myDB.getProductInfofrommysql(myUA.conveyor_info.value_RFID_readdata);

            //RFID��־λ��0���رն�дRFID
            myUA.UA_writevalue(conveyor_param.flag_RFID,"0");


            //������û�зֵ����κβ�Ʒ
            if(myDB.product_row == 0)
            {
                //д2,Ĭ��ֱ��
                myUA.UA_writevalue(conveyor_param.corner_switch,"2");

                //��һ��
                this_thread::sleep_for(chrono::milliseconds(1500));

                //д2���������
                myUA.UA_writevalue(conveyor_param.cylinder_status,"2");

                //RFID��־λ��0���رն�дRFID
                myUA.UA_writevalue(conveyor_param.flag_RFID,"0");

                //�����޸��豸״̬Ϊready
                g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                         TABLE_EQUIPMENTINFO,{"status","ready",
                                                                              "guid",myDB.equip_info.guid});

                //�������switch-case
                break;
            }

            //���������ݲ�Ʒ��guid���ó��ò�Ʒȫ���Ĳ���
            myDB.getOperationInfoAllfrommysql(myDB.product_info.guid);


            //��¼����豸�����ǵڼ�������
            int num_oper = 0;

            for(int k =0;k < myDB.oper_all_row ; k++)
            {
                //���ʹ���Ҫ���豸��
                if((myDB.deque_oper_all_info.at(k).name == "Operation_switch")&&
                        (myDB.deque_oper_all_info.at(k).equipment_guid == myDB.equip_info.guid)&&
                        (myDB.deque_oper_all_info.at(k).status == "ready"))
                {
                    num_oper = k;
                    //�ҵ���break�����for
                    break;

                }
            }

            //Ȼ��Ϳ��������ǰ��Ĳ����������û
            for(int t =0;t <= num_oper ; t++)
            {
                if((myDB.deque_oper_all_info.at(t).status != "done")&&(t < num_oper))
                {
                    //��ǰ�滹�й���û����ɣ��Ǿ�break��forѭ����ִ�з���
                    break;
                }

                if(myDB.deque_oper_all_info.at(t).status == "done")
                {
                    //�������������ˣ���ô���������һ������
                    continue;
                }

                //����break������continue����������һ��
                if(t == num_oper)
                {
                    //�ò�����ready״̬

                    //�޸Ĳ���״̬Ϊworking
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_OPERATIONINFO,{"status","ready",
                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});

                    string strCurTime = GetCurrentSystemTime();

                    //���Ӳ���״̬ʱ��
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_OPERATIONINFO,{"start_time",strCurTime,
                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});

                    //ִ�����
                    printf("���̵�λ�����ڷ����Ƿ�ת��ָ�����Ϊ %s\n", myDB.deque_oper_all_info.at(t).param.c_str());


                    //д������
                    myUA.UA_writevalue(conveyor_param.corner_switch,
                                       myDB.deque_oper_all_info.at(t).param);

                    //��һ��
                    this_thread::sleep_for(chrono::seconds(1));

                    //д2���������
                    myUA.UA_writevalue(conveyor_param.cylinder_status,"2");

                    //�޸Ĳ���״̬Ϊdone
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_OPERATIONINFO,{"status","done",
                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});

                    string strCurTime_2 = GetCurrentSystemTime();

                    //���Ӳ���״̬ʱ��
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_OPERATIONINFO,{"end_time",strCurTime_2,
                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});

                    //�����޸��豸״̬Ϊready
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_EQUIPMENTINFO,{"status","ready",
                                                                                  "guid",myDB.equip_info.guid});
                    //���������һ���������˳����for��һ��ֻ����һ������
                    break;

                }

                //�������for
                //break;
            }


            //�����޸��豸״̬Ϊready
            g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                     TABLE_EQUIPMENTINFO,{"status","ready",
                                                                          "guid",myDB.equip_info.guid});



        }
            break;

            /*--ODG�����ֵ������ 2020.10.26 00:08--*/
            /*--Densou������ɣ�2020.10.28 15:55-*/
            /*--SCUT������� 2020.10.28 04:32--*/
        case ROBOT:
        {

            ROBOT_PARAM robot_param;

            printf("��ʼ���л����ֵ��߼�\n");

            //�ȴ����̵���
            printf("���ڵȴ����̵������豸Ϊ %s", myDB.equip_info.actual_name.c_str());

            //���̵����翪��״̬��1->0��
            while (g_flag_IsMainRunning && myUA.robot_info.value_switch_status != "1")
            {
                this_thread::sleep_for(chrono::milliseconds(2));
            }

            while (g_flag_IsMainRunning && myUA.robot_info.value_switch_status != "0")
            {
                this_thread::sleep_for(chrono::milliseconds(2));
            }

            myUA.UA_writevalue(robot_param.guard2_status, "1");//д��1-����2����
            this_thread::sleep_for(chrono::seconds(3));
            myUA.UA_writevalue(robot_param.guard1_status, "1");//д��1-����1����


            //�޸��豸״̬Ϊworking
            g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                     TABLE_EQUIPMENTINFO,{"status","working","guid",myDB.equip_info.guid});

            //RFID��־λ��1
            myUA.UA_writevalue(robot_param.flag_RFID,"1");

            for(int t=0;t<10;t++)
            {
                //���RFID��ֵ
                if((myUA.robot_info.value_RFID_readdata != "0")&&
                        (myUA.robot_info.value_RFID_readdata != ""))
                {
                    //�����Ļ���break������ִ�������
                    break;
                }
                this_thread::sleep_for(chrono::milliseconds(500));

            }

            //�����Ƕ�����RFID������У�����һ�㲻������������
            if((myUA.robot_info.value_RFID_readdata == "0")||
                    (myUA.robot_info.value_RFID_readdata == ""))
            {
                //����
                myUA.UA_writevalue(robot_param.guard1_status, "0");//д��0-����1����
                this_thread::sleep_for(chrono::milliseconds(800));
                myUA.UA_writevalue(robot_param.guard2_status, "0");//д��0-����2����

                //RFID��־λ��0
                myUA.UA_writevalue(robot_param.flag_RFID,"0");

                //�޸��豸״̬Ϊready
                g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                         TABLE_EQUIPMENTINFO,{"status","ready","guid",myDB.equip_info.guid});

                //����switch-case
                break;
            }

            //���������ݶ���������guid���ó���Ʒ��ip���Լ�����deque
            myDB.getProductInfofrommysql(myUA.robot_info.value_RFID_readdata);

            //RFID��־λ��0
            myUA.UA_writevalue(robot_param.flag_RFID,"0");

            //������û�зֵ����κβ�Ʒ
            if(myDB.product_row == 0)
            {
                //����
                myUA.UA_writevalue(robot_param.guard1_status, "0");//д��0-����1����
                this_thread::sleep_for(chrono::milliseconds(800));
                myUA.UA_writevalue(robot_param.guard2_status, "0");//д��0-����2����

                //RFID��־λ��0
                myUA.UA_writevalue(robot_param.flag_RFID,"0");

                //�޸��豸״̬Ϊready
                g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                         TABLE_EQUIPMENTINFO,{"status","ready","guid",myDB.equip_info.guid});

                //����switch-case
                break;

            }


            //�������ǻ��deque��һ�����豸*��Ʒ��һ���ǲ�Ʒ
            //���������ݲ�Ʒ��guid���ó��ò�Ʒ���豸�Ĳ���
            myDB.getOperationInfofrommysql(myDB.product_info.guid);

            //���������ݲ�Ʒ��guid���ó��ò�Ʒ���豸�Ĳ���
            myDB.getOperationInfoAllfrommysql(myDB.product_info.guid);

            //RFID��־λ��1��������1�����ں���CNC�Ķ�ֵ
            //myUA.UA_writevalue(robot_param.flag_RFID,"1");

            string oper_name;

            int num_oper = 0;

            //���������Ʒ����Ҫ����豸�ɻ�
            if(myDB.oper_row == 0)
            {
                //����
                myUA.UA_writevalue(robot_param.guard1_status, "0");//д��0-����1����
                this_thread::sleep_for(chrono::milliseconds(800));
                myUA.UA_writevalue(robot_param.guard2_status, "0");//д��0-����2����

                //RFID��־λ��0
                myUA.UA_writevalue(robot_param.flag_RFID,"0");

                //�޸��豸״̬Ϊready
                g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                         TABLE_EQUIPMENTINFO,{"status","ready","guid",myDB.equip_info.guid});

                //�������switch-case
                break;
            }
            else
            {
                //Ҫ�������������Ļ���������һ������һ������˾����ڶ���
                for(int u = 0;u < myDB.oper_row;u++)
                {
                    oper_name = myDB.deque_oper_info.at(u).name;

                    if(myDB.deque_oper_info.at(u).status == "ready")
                    {

                        //��Ҫ��ǰ����������û
                        //��¼����豸�Ĳ����ǲ�Ʒ�ĵڼ�������
                        for(int t =0;t < myDB.oper_all_row ; t++)
                        {
                            if(myDB.deque_oper_all_info.at(t).name == oper_name)
                            {
                                num_oper = t ;
                                //�ҵ����˳����forѭ��
                                break;
                            }
                        }

                        //Ȼ��Ϳ��������ǰ��Ĳ����������û
                        //������<=,��Ϊ��������
                        for(int t =0;t <= num_oper  ; t++)
                        {
                            //����ĸ�Դ�ǣ���ʹ���ݿ���д��Operation_pick2CNC��ɣ����ǳ�������û�и��µ�
                            myDB.getOperationInfoAllfrommysql(myDB.product_info.guid);

                            //�����и�������ǣ�Ҫ���жϵ���CNC������Ҫ��CNC��ɣ��ٵ�����Operation_pick2pallet�Ĳ���
                            if(myDB.deque_oper_all_info.at(t).name == "Operation_machine")
                            {
                                //���ϵ�ȥɨ���ݿ��״̬���ȵ�CNC������ˣ���ִ�������
                                while(myDB.deque_oper_all_info.at(t).status != "done")
                                {
                                    myDB.getOperationInfoAllfrommysql(myDB.product_info.guid);
                                    this_thread::sleep_for(chrono::milliseconds(800));

                                }
                                t = num_oper;
                            }

                            if((myDB.deque_oper_all_info.at(t).status != "done")&&(t < num_oper))
                            {
                                //��ǰ�滹�й���û����ɣ��Ǿ�break��forѭ����ִ�з���

                                break;
                            }

                            if(myDB.deque_oper_all_info.at(t).status == "done")
                            {
                                //�������������ˣ���ô���������һ������
                                continue;
                            }

                            //����break������continue����������һ��
                            //�������һ��û��done������
                            if(t == num_oper )
                            {

                                //ִ��ODG����
                                if(myDB.deque_oper_all_info.at(t).name == "Operation_LM")
                                {

                                    ROBOT_PARAM robot_param;

                                    printf("RFID��֤��ɣ����ڷ���ץȡָ��豸Ϊ %s\n",
                                           myDB.equip_info.actual_name.c_str());

                                    string strCurTime = GetCurrentSystemTime();

                                    //�޸Ĳ���״̬Ϊworking
                                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                             TABLE_OPERATIONINFO,{"status","working",
                                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});

                                    //����״̬��ʱ��
                                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                             TABLE_OPERATIONINFO,{"start_time",strCurTime,
                                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});

                                    //�޸�����2״̬Ϊworking
                                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                             TABLE_FEATUREINFO,{"status","working",
                                                                                                "id",to_string(myDB.deque_feature_info.at(1).id)});
                                    //����2״̬��ʱ��
                                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                             TABLE_FEATUREINFO,{"start_time",strCurTime,
                                                                                                "id",to_string(myDB.deque_feature_info.at(1).id)});


                                    int file_input_num = atoi(myDB.deque_oper_all_info.at(t).param.c_str());

                                    if (file_input_num > 3 || file_input_num < 1)
                                    {
                                        file_input_num = 3;
                                    }


                                    myUA.UA_writevalue(robot_param.file_input[file_input_num - 1], "1");//д��1��file�ı�־λ��ѡ��Op�������ļ�

                                    this_thread::sleep_for(chrono::seconds(3));

                                    myUA.UA_writevalue(robot_param.file_input[file_input_num - 1], "0");


                                    //ָ��ִ�����
                                    while (g_flag_IsMainRunning && myUA.robot_info.value_current_status != "0")
                                    {
                                        this_thread::sleep_for(chrono::milliseconds(200));
                                    }
                                    while (g_flag_IsMainRunning && myUA.robot_info.value_current_status != "1")
                                    {
                                        this_thread::sleep_for(chrono::milliseconds(200));
                                    }


                                    string strCurTime_1 = GetCurrentSystemTime();

                                    //���Ʒ Server ������д������2�����
                                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                             TABLE_FEATUREINFO,{"status","done",
                                                                                                "id",to_string(myDB.deque_feature_info.at(1).id)});
                                    //����2��ʱ��
                                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                             TABLE_FEATUREINFO,{"end_time",strCurTime_1,
                                                                                                "id",to_string(myDB.deque_feature_info.at(1).id)});

                                    //�޸Ĳ���״̬Ϊdone
                                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                             TABLE_OPERATIONINFO,{"status","done",
                                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});

                                    //����״̬��ʱ��
                                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                             TABLE_OPERATIONINFO,{"end_time",strCurTime_1,
                                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});

                                }

                                //������ʶ��������������Operation_pick2CNC������Operation_pick2pallet
                                //����ʶ��Operation_pick2CNC����Ҫͬʱ�����������Ҳ���������һ���Ȳ�break
                                if(myDB.deque_oper_all_info.at(t).name == "Operation_pick2CNC")
                                {

                                    ROBOT_PARAM robot_param;

                                    printf("���ڷ���Start1,ָ��豸Ϊ %s", myDB.equip_info.actual_name.c_str());

                                    //�޸Ĳ���״̬Ϊworking
                                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                             TABLE_OPERATIONINFO,{"status","working",
                                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});
                                    string currenttime = GetCurrentSystemTime();

                                    //����״̬��ʱ��
                                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                             TABLE_OPERATIONINFO,{"start_time",currenttime,
                                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});


                                    /*--�Ϸ�����дֵ����--*/

                                    //                                    myUA.UA_writevalue(robot_param.inputcommand, "Start,\n");//д��Start,��ʼץȡ

                                    //                                    //Start,ָ��ִ�����
                                    //                                    while (g_flag_IsMainRunning && (myUA.UA_readvalue(robot_param.current_status).find("OK1") == string::npos))
                                    //                                    {
                                    //                                        Sleep(500);
                                    //                                    }



                                    /*20201006*/

                                    /*--call�����ڵ����--*/
                                    myUA.UA_writevalue(robot_param.cap_input_cmd_pick, "In");//д��In

                                    UA_Variant input;
                                    UA_Variant_init(&input);
                                    size_t outputSize;
                                    UA_Variant *output;

                                    str1 = myDB.equip_info.guid + "/AAS/Capability_Pick/Start";
                                    str2 = myDB.equip_info.guid + "/AAS/Capability_Pick/Start@Start";


                                    g_flag_IsEquipmentUAConnected = UA_Client_call(myUA.client,UA_NODEID_STRING(3, (char*)str1.c_str()),
                                                                                   UA_NODEID_STRING(3, (char*)str2.c_str()),
                                                                                   0, &input, &outputSize, &output);

                                    this_thread::sleep_for(chrono::milliseconds(500));

                                    while (g_flag_IsMainRunning && myUA.UA_readvalue("/AAS/Capability_Pick/Output/Information:value@Value") != "Idle")
                                    {
                                        this_thread::sleep_for(chrono::milliseconds(500));
                                    }

                                    UA_Array_delete(output, outputSize, &UA_TYPES[UA_TYPES_VARIANT]);
                                    UA_Variant_clear(&input);


                                    //�޸Ĳ���״̬Ϊdone
                                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                             TABLE_OPERATIONINFO,{"status","done",
                                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});
                                    string currenttime_1 = GetCurrentSystemTime();

                                    //����״̬��ʱ��
                                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                             TABLE_OPERATIONINFO,{"end_time",currenttime_1,
                                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});


                                }


                                //�����Operation_pick2pallet�������������������ǵò����ݿ����
                                if(myDB.deque_oper_all_info.at(t).name == "Operation_pick2pallet")
                                {

                                    ROBOT_PARAM robot_param;

                                    printf("���ڷ���Back,ָ��豸Ϊ %s", myDB.equip_info.actual_name.c_str());

                                    //�޸Ĳ���״̬Ϊworking
                                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                             TABLE_OPERATIONINFO,{"status","working",
                                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});

                                    string currenttime = GetCurrentSystemTime();

                                    //����״̬��ʱ��
                                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                             TABLE_OPERATIONINFO,{"start_time",currenttime,
                                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});


                                    /*20201006*/
                                    myUA.UA_writevalue(robot_param.cap_input_cmd_pick, "Out");//д��OUT

                                    UA_Variant input;
                                    UA_Variant_init(&input);
                                    size_t outputSize;
                                    UA_Variant *output;

                                    str1 = myDB.equip_info.guid + "/AAS/Capability_Pick/Start";
                                    str2 = myDB.equip_info.guid + "/AAS/Capability_Pick/Start@Start";

                                    g_flag_IsEquipmentUAConnected = UA_Client_call(myUA.client,UA_NODEID_STRING(3, (char*)str1.c_str()),
                                                                                   UA_NODEID_STRING(3, (char*)str2.c_str()),
                                                                                   0, &input, &outputSize, &output);

                                    this_thread::sleep_for(chrono::milliseconds(500));

                                    while (g_flag_IsMainRunning && myUA.UA_readvalue("/AAS/Capability_Pick/Output/Information:value@Value") != "Idle")
                                    {
                                        this_thread::sleep_for(chrono::milliseconds(500));
                                    }

                                    UA_Array_delete(output, outputSize, &UA_TYPES[UA_TYPES_VARIANT]);
                                    UA_Variant_clear(&input);


                                    //�޸Ĳ���״̬Ϊdone
                                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                             TABLE_OPERATIONINFO,{"status","done",
                                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});

                                    string currenttime_1 = GetCurrentSystemTime();

                                    //����״̬��ʱ��
                                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                             TABLE_OPERATIONINFO,{"end_time",currenttime_1,
                                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});

                                }


                                if(myDB.deque_oper_all_info.at(t).name == "Operation_package")
                                {

                                    printf("���ڷ���Start1,ָ��豸Ϊ %s", myDB.equip_info.actual_name.c_str());

                                    string strCurTime = GetCurrentSystemTime();

                                    //�޸Ĳ���״̬Ϊworking
                                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                             TABLE_OPERATIONINFO,{"status","working",
                                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});

                                    //����״̬��ʱ��
                                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                             TABLE_OPERATIONINFO,{"start_time",strCurTime,
                                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});

                                    //�޸�����4״̬Ϊworking
                                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                             TABLE_FEATUREINFO,{"status","working",
                                                                                                "id",to_string(myDB.deque_feature_info.at(3).id)});
                                    //����4״̬��ʱ��
                                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                             TABLE_FEATUREINFO,{"start_time",strCurTime,
                                                                                                "id",to_string(myDB.deque_feature_info.at(3).id)});


                                    /*20201006*/
                                    UA_Variant input;
                                    UA_Variant_init(&input);
                                    size_t outputSize;
                                    UA_Variant *output;
                                    str1 = myDB.equip_info.guid + "/AAS/Capability_Packaging/Start";
                                    str2 = myDB.equip_info.guid + "/AAS/Capability_Packaging/Start@Start";
                                    g_flag_IsEquipmentUAConnected = UA_Client_call(myUA.client,UA_NODEID_STRING(3, (char*)str1.c_str()),
                                                                                   UA_NODEID_STRING(3, (char*)str2.c_str()), 0, &input, &outputSize, &output);

                                    Sleep(500);

                                    while (g_flag_IsMainRunning && myUA.UA_readvalue("/AAS/Capability_Packaging/Output/Information:value@Value") != "Idle")
                                    {
                                        this_thread::sleep_for(chrono::milliseconds(500));
                                    }
                                    UA_Array_delete(output, outputSize, &UA_TYPES[UA_TYPES_VARIANT]);
                                    UA_Variant_clear(&input);


                                    string strCurTime_1 = GetCurrentSystemTime();

                                    //�޸Ĳ���״̬Ϊdone
                                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                             TABLE_OPERATIONINFO,{"status","done",
                                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});

                                    //����״̬��ʱ��
                                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                             TABLE_OPERATIONINFO,{"end_time",strCurTime,
                                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});

                                    //�޸�����4״̬Ϊworking
                                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                             TABLE_FEATUREINFO,{"status","done",
                                                                                                "id",to_string(myDB.deque_feature_info.at(3).id)});
                                    //����4״̬��ʱ��
                                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                             TABLE_FEATUREINFO,{"end_time",strCurTime,
                                                                                                "id",to_string(myDB.deque_feature_info.at(3).id)});

                                }

                            }

                        }





                        //���������һ���������˳����for��һ��ֻ����һ������
                        //���ﲻ��Ҫ�����������Ҫһ�������������
                        //break;
                    }
                    else
                    {
                        //Ҫ���������ready�Ļ�������һ��
                        continue;
                    }
                }

            }

            //����
            myUA.UA_writevalue(robot_param.guard1_status, "0");//д��0-����1����
            this_thread::sleep_for(chrono::milliseconds(800));
            myUA.UA_writevalue(robot_param.guard2_status, "0");//д��0-����2����

            //RFID��־λ��0
            myUA.UA_writevalue(robot_param.flag_RFID,"0");


            //�����޸��豸״̬Ϊready
            g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                     TABLE_EQUIPMENTINFO,{"status","ready",
                                                                          "guid",myDB.equip_info.guid});

        }
            break;

            /*--CNC������� 2020.10.28 21:12----*/
        case CNC:
        {
            CNC_PARAM cnc_param;

            printf("��ʼ����CNC���߼�\n");

            //�ȴ����̵���
            printf("���ڵȴ�CNC���У��豸Ϊ %s", myDB.equip_info.actual_name.c_str());

            //����CNC���߼�ֻ�ܴ����ݿ�����

            while(g_flag_IsMainRunning)
            {
                this_thread::sleep_for(chrono::milliseconds(500));

                myDB.getCNCInfofrommysql();

                for(int t = 0; t < myDB.cnc_row; t++)
                {
                    //������pick2cnc��ɵ�
                    if((myDB.deque_cnc_info.at(t).name == "Operation_pick2CNC")&&
                            (myDB.deque_cnc_info.at(t).status == "done"))
                    {
                        //����pick2cncn��ɵĺ��棬�ǲ���ready���ǲ�����̨cnc
                        if  ((myDB.deque_cnc_info.at(t+1).name == "Operation_machine")&&
                             (myDB.deque_cnc_info.at(t+1).status == "ready")&&
                             (myDB.deque_cnc_info.at(t+1).equipment_guid == myDB.equip_info.guid))
                        {

                            //�����������ˣ��Ϳ��Ը��ݲ�Ʒguid�ҳ�����
                            myDB.getFeatureInfofrommysql(myDB.deque_cnc_info.at(t+1).product_guid);


                            printf("�豸���У����ڷ��ͼӹ��������豸Ϊ %s", myDB.equip_info.actual_name.c_str());

                            string currenttime = GetCurrentSystemTime();


                            //�޸��豸״̬Ϊworking
                            g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                     TABLE_EQUIPMENTINFO,{"status","working","guid",myDB.equip_info.guid});

                            //�޸Ĳ���״̬Ϊworking
                            g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                     TABLE_OPERATIONINFO,{"status","working",
                                                                                          "id",to_string(myDB.deque_cnc_info.at(t+1).id)});

                            //����״̬Ϊworking
                            g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                     TABLE_FEATUREINFO,{"status","working",
                                                                                        "id",to_string(myDB.deque_feature_info.at(2).id)});


                            //����״̬��ʱ��
                            g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                     TABLE_OPERATIONINFO,{"start_time",currenttime,
                                                                                          "id",to_string(myDB.deque_cnc_info.at(t+1).id)});

                            //����״̬��ʱ��
                            g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                     TABLE_FEATUREINFO,{"start_time",currenttime,
                                                                                        "id",to_string(myDB.deque_feature_info.at(2).id)});


                            /*20201006*/
                            myUA.UA_writevalue(cnc_param.cap_input_cmd, myDB.deque_cnc_info.at(t+1).param);


                            UA_Variant input;
                            UA_Variant_init(&input);
                            size_t outputSize;
                            UA_Variant *output;
                            str1 = myDB.equip_info.guid + "/AAS/Capability_Carving/Start";
                            str2 = myDB.equip_info.guid + "/AAS/Capability_Carving/Start@Start";

                            g_flag_IsEquipmentUAConnected = UA_Client_call(myUA.client,
                                                                           UA_NODEID_STRING(3, (char*)str1.c_str()),
                                                                           UA_NODEID_STRING(3, (char*)str2.c_str()),
                                                                           0, &input, &outputSize, &output);

                            Sleep(500);

                            while (g_flag_IsMainRunning && myUA.UA_readvalue("/AAS/Capability_Carving/Output/Information:value@Value") != "Idle")
                            {
                                this_thread::sleep_for(chrono::milliseconds(500));
                            }

                            UA_Array_delete(output, outputSize, &UA_TYPES[UA_TYPES_VARIANT]);
                            UA_Variant_clear(&input);


                            string currenttime_1 = GetCurrentSystemTime();

                            //�޸Ĳ���״̬Ϊdone
                            g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                     TABLE_OPERATIONINFO,{"status","done",
                                                                                          "id",to_string(myDB.deque_cnc_info.at(t+1).id)});


                            //�޸�����״̬Ϊdone
                            g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                     TABLE_FEATUREINFO,{"status","done",
                                                                                        "id",to_string(myDB.deque_feature_info.at(2).id)});
                            //����״̬��ʱ��
                            g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                     TABLE_OPERATIONINFO,{"end_time",currenttime_1,
                                                                                          "id",to_string(myDB.deque_cnc_info.at(t+1).id)});

                            //����״̬��ʱ��
                            g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                     TABLE_FEATUREINFO,{"end_time",currenttime_1,
                                                                                        "id",to_string(myDB.deque_feature_info.at(2).id)});

                            //�޸��豸״̬Ϊready
                            g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                     TABLE_EQUIPMENTINFO,{"status","ready","guid",myDB.equip_info.guid});
                        }

                    }

                }

            }


            //CNCû�з���

            //�����޸��豸״̬Ϊready
            g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                     TABLE_EQUIPMENTINFO,{"status","ready",
                                                                          "guid",myDB.equip_info.guid});

        }
            break;

            /*--�����Ѳ������  10.28 03:42--*/
        case LASER:
        {

            LASER_PARAM laser_param;

            printf("��ʼ���м�����߼�\n");

            //�ȴ����̵���
            printf("���ڵȴ����̵������豸Ϊ %s", myDB.equip_info.actual_name.c_str());

            //���̵����翪��״̬��1->0��

            while (g_flag_IsMainRunning && myUA.laser_info.value_switch_status != "1")
            {
                this_thread::sleep_for(chrono::milliseconds(2));
            }

            while (g_flag_IsMainRunning && myUA.laser_info.value_switch_status != "0")
            {
                this_thread::sleep_for(chrono::milliseconds(2));
            }

            myUA.UA_writevalue(laser_param.guard2_status, "1");//д��1-����2����
            this_thread::sleep_for(chrono::seconds(2));
            myUA.UA_writevalue(laser_param.guard1_status, "1");//д��1-����1����

            //�޸��豸״̬Ϊworking
            g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,TABLE_EQUIPMENTINFO,{"status","working","guid",myDB.equip_info.guid});

            //RFID��־λ��1
            myUA.UA_writevalue(laser_param.flag_RFID,"1");

            for(int t=0;t<10;t++)
            {
                //���RFID��ֵ
                if((myUA.laser_info.value_RFID_readdata != "0")&&
                        (myUA.laser_info.value_RFID_readdata != ""))
                {
                    //�����Ļ���break������ִ�������
                    break;
                }
                this_thread::sleep_for(chrono::milliseconds(500));
            }

            //�����Ƕ�����RFID���򵯳�������һ�㲻������������
            if((myUA.laser_info.value_RFID_readdata == "0")||
                (myUA.laser_info.value_RFID_readdata == ""))
            {

                //����
                myUA.UA_writevalue(laser_param.guard1_status, "0");//д��0-����1����
                this_thread::sleep_for(chrono::milliseconds(800));
                myUA.UA_writevalue(laser_param.guard2_status, "0");//д��0-����2���� //�������switch-case

                //RFID��־λ��0
                myUA.UA_writevalue(laser_param.flag_RFID,"0");

                //�޸��豸״̬Ϊready
                g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                         TABLE_EQUIPMENTINFO,{"status","ready","guid",myDB.equip_info.guid});

                //�������switch-case
                break;
            }


            //���������ݶ���������guid���ó���Ʒ��ip���Լ�����deque
            myDB.getProductInfofrommysql(myUA.laser_info.value_RFID_readdata);

            //RFID��־λ��0���رն�дRFID
            myUA.UA_writevalue(laser_param.flag_RFID,"0");

            //������û�зֵ����κβ�Ʒ
            if(myDB.product_row == 0)
            {

                //����
                myUA.UA_writevalue(laser_param.guard1_status, "0");//д��0-����1����
                this_thread::sleep_for(chrono::milliseconds(800));
                myUA.UA_writevalue(laser_param.guard2_status, "0");//д��0-����2���� //�������switch-case

                //RFID��־λ��0
                myUA.UA_writevalue(laser_param.flag_RFID,"0");

                //�޸��豸״̬Ϊready
                g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,TABLE_EQUIPMENTINFO,{"status","ready","guid",myDB.equip_info.guid});

                //�������switch-case
                break;
            }

            //���������ݲ�Ʒ��guid���ó��ò�Ʒȫ���Ĳ���
            myDB.getOperationInfoAllfrommysql(myDB.product_info.guid);


            //��¼����豸�����ǵڼ�������
            int num_oper = 0;

            for(int k =0;k < myDB.oper_all_row ; k++)
            {
                if(myDB.deque_oper_all_info.at(k).name == "Operation_laser")
                {
                    num_oper = k;
                    break;
                }
            }

            //Ȼ��Ϳ��������ǰ��Ĳ����������û
            for(int t =0;t <= num_oper ; t++)
            {
                if((myDB.deque_oper_all_info.at(t).status != "done")&&(t < num_oper))
                {
                    //��ǰ�滹�й���û����ɣ��Ǿ�break��forѭ����ִ�з���
                    break;
                }

                if(myDB.deque_oper_all_info.at(t).status == "done")
                {
                    //�������������ˣ���ô���������һ������
                    continue;
                }

                //����break������continue����������һ��
                if( t == num_oper )
                {
                    //ִ�в���

                    //�޸Ĳ���״̬Ϊworking
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_OPERATIONINFO,{"status","working",
                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});


                    //�޸�����״̬Ϊworking
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_FEATUREINFO,{"status","working",
                                                                                "id",to_string(myDB.deque_feature_info.at(2).id)});


                    int file_input_num = atoi(myDB.deque_oper_all_info.at(t).param.c_str());

                    if (file_input_num > 8 || file_input_num < 1)
                    {
                        file_input_num = 8;
                    }

                    string currenttime = GetCurrentSystemTime();

                    //����״̬��ʱ��
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_OPERATIONINFO,{"start_time",currenttime,
                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});

                    //����״̬��ʱ��
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_FEATUREINFO,{"start_time",currenttime,
                                                                                "id",to_string(myDB.deque_feature_info.at(2).id)});


                    myUA.UA_writevalue(laser_param.file_input[file_input_num - 1], "1");//д��1��file�ı�־λ��ѡ��Op�������ļ�
                    Sleep(1000);
                    myUA.UA_writevalue(laser_param.file_input[file_input_num - 1], "0");


                    //****�������������ź�ʱ��ʱ�ᶶ�����˴�������������
Label_Laser_Debounce:
                    while (g_flag_IsMainRunning && myUA.laser_info.value_current_status != "0")
                    {
                        this_thread::sleep_for(chrono::milliseconds(200));
                    }

                    this_thread::sleep_for(chrono::milliseconds(800));

                    if (myUA.laser_info.value_current_status != "0")
                    {
                        goto Label_Laser_Debounce;
                    }


                    while (g_flag_IsMainRunning && myUA.laser_info.value_current_status != "1")
                    {
                        this_thread::sleep_for(chrono::milliseconds(200));
                    }

                    //�޸Ĳ���״̬Ϊdone
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_OPERATIONINFO,{"status","done",
                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});


                    //���Ʒ Server ������д������1�����,������д��ƷServer�ģ��������Ȱ�
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_FEATUREINFO,{"status","done",
                                                                                "id",to_string(myDB.deque_feature_info.at(2).id)});

                    string currenttime_1 = GetCurrentSystemTime();

                    //����״̬��ʱ��
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_OPERATIONINFO,{"end_time",currenttime_1,
                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});

                    //����״̬��ʱ��
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_FEATUREINFO,{"end_time",currenttime_1,
                                                                                "id",to_string(myDB.deque_feature_info.at(2).id)});

                }

            }

            //RFID��־λ��0
            myUA.UA_writevalue(laser_param.flag_RFID,"0");

            //����
            myUA.UA_writevalue(laser_param.guard1_status, "0");//д��0-����1����
            this_thread::sleep_for(chrono::milliseconds(800));
            myUA.UA_writevalue(laser_param.guard2_status, "0");//д��0-����2����



            //�����޸��豸״̬Ϊready
            g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                     TABLE_EQUIPMENTINFO,{"status","ready",
                                                                          "guid",myDB.equip_info.guid});

        }
            break;


        case MACHINE_UPLOAD_COVER:
        {

            MACHINE_UPLOAD_COVER_PARAM machine_upload_cover_param;

            printf("��ʼ�����ϸǻ����߼�\n");

            //�ȴ����̵���
            printf("���ڵȴ����̵������豸Ϊ %s", myDB.equip_info.actual_name.c_str());

            //���̵���
            while (g_flag_IsMainRunning && myUA.machine_upload_cover_info.value_current_status != "11")
            {
                this_thread::sleep_for(chrono::milliseconds(500));
            }

            //�޸��豸״̬Ϊworking
            g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                     TABLE_EQUIPMENTINFO,{"status","working","guid",myDB.equip_info.guid});

            //RFID��־λ��1
            myUA.UA_writevalue(machine_upload_cover_param.flag_RFID,"1");

            for(int t=0;t<10;t++)
            {
                //���RFID��ֵ
                if((myUA.machine_upload_cover_info.value_RFID_readdata != "0")&&
                    (myUA.machine_upload_cover_info.value_RFID_readdata != ""))
                {
                    //�����Ļ���break������ִ�������
                    break;
                }
                this_thread::sleep_for(chrono::milliseconds(500));
            }

            //�����Ƕ�����RFID���򵯳�������һ�㲻������������
            if((myUA.machine_upload_cover_info.value_RFID_readdata == "0")||
                (myUA.machine_upload_cover_info.value_RFID_readdata == ""))
            {
                //
                myUA.UA_readvalue(machine_upload_cover_param.columns);

                //��д0
                myUA.UA_writevalue(machine_upload_cover_param.columns,"0");

                //д��14����
                myUA.UA_writevalue(machine_upload_cover_param.columns,"14");

                //RFID��־λ��0
                myUA.UA_writevalue(machine_upload_cover_param.flag_RFID,"0");

                //�޸��豸״̬Ϊready
                g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                         TABLE_EQUIPMENTINFO,{"status","ready","guid",myDB.equip_info.guid});

                //�������switch-case
                break;
            }

            //���������ݶ���������guid���ó���Ʒ��ip���Լ�����deque
            myDB.getProductInfofrommysql(myUA.machine_upload_cover_info.value_RFID_readdata);

            //RFID��־λ��0���رն�дRFID
            myUA.UA_writevalue(machine_upload_cover_param.flag_RFID,"0");

            //������û�зֵ����κβ�Ʒ
            if(myDB.product_row == 0)
            {
                //
                myUA.UA_readvalue(machine_upload_cover_param.columns);

                //��д0
                myUA.UA_writevalue(machine_upload_cover_param.columns,"0");

                //д��14����
                myUA.UA_writevalue(machine_upload_cover_param.columns,"14");

                //RFID��־λ��0
                myUA.UA_writevalue(machine_upload_cover_param.flag_RFID,"0");

                //�޸��豸״̬Ϊready
                g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                         TABLE_EQUIPMENTINFO,{"status","ready","guid",myDB.equip_info.guid});

                //�������switch-case
                break;
            }

            //���������ݲ�Ʒ��guid���ó��ò�Ʒȫ���Ĳ���
            myDB.getOperationInfoAllfrommysql(myDB.product_info.guid);


            //��¼����豸�����ǵڼ�������
            int num_oper = 0;

            for(int t =0;t < myDB.oper_all_row ; t++)
            {
                if(myDB.deque_oper_all_info.at(t).name == "Operation_LC")
                {
                    num_oper = t;
                    //�ҵ���break�����for
                    break;

                }
            }

            //Ȼ��Ϳ��������ǰ��Ĳ����������û
            for(int t =0;t <= num_oper ; t++)
            {
                if((myDB.deque_oper_all_info.at(t).status != "done")&&(t < num_oper))
                {
                    //��ǰ�滹�й���û����ɣ��Ǿ�break��forѭ����ִ�з���
                    break;
                }

                if(myDB.deque_oper_all_info.at(t).status == "done")
                {
                    //�������������ˣ���ô���������һ������
                    continue;
                }

                //����break������continue����������һ��
                if(t == num_oper)
                {

                    //д���ϸ�����
                    //                    myUA.UA_writevalue(machine_upload_cover_param.columns,
                    //                                myDB.deque_oper_all_info.at(t).param);

                    myUA.UA_writevalue(machine_upload_cover_param.columns,"2");

                    string currenttime = GetCurrentSystemTime();

                    //�޸Ĳ���״̬Ϊworking
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_OPERATIONINFO,{"status","working",
                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});

                    //�޸�����״̬Ϊworking
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_FEATUREINFO,{"status","working",
                                                                                "id",to_string(myDB.deque_feature_info.at(4).id)});

                    //����״̬��ʱ��
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_OPERATIONINFO,{"start_time",currenttime,
                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});

                    //����״̬��ʱ��
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_FEATUREINFO,{"start_time",currenttime,
                                                                                "id",to_string(myDB.deque_feature_info.at(4).id)});


                    //�ȴ��ϸ����
                    while (g_flag_IsMainRunning && myUA.machine_upload_cover_info.value_current_status!= "12")
                    {
                        this_thread::sleep_for(chrono::milliseconds(500));
                    }

                    string currenttime_1 = GetCurrentSystemTime();


                    //�޸Ĳ���״̬Ϊdone
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_OPERATIONINFO,{"status","done",
                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});


                    //�޸�����״̬Ϊdone
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_FEATUREINFO,{"status","done",
                                                                                "id",to_string(myDB.deque_feature_info.at(4).id)});

                    //����״̬��ʱ��
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_OPERATIONINFO,{"end_time",currenttime_1,
                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});


                    //����״̬��ʱ��
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_FEATUREINFO,{"end_time",currenttime_1,
                                                                                "id",to_string(myDB.deque_feature_info.at(4).id)});

                }

            }

            //
            myUA.UA_readvalue(machine_upload_cover_param.columns);

            //��д0
            myUA.UA_writevalue(machine_upload_cover_param.columns,"0");

            //д��14����
            myUA.UA_writevalue(machine_upload_cover_param.columns,"14");


            //�����޸��豸״̬Ϊready
            g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                     TABLE_EQUIPMENTINFO,{"status","ready",
                                                                          "guid",myDB.equip_info.guid});

        }
            break;


            /*---�ºл���������  2020.10.25 01:16---*/
        case MACHINE_UNLOAD_BOX:
        {
            MACHINE_UNLOAD_BOX_PARAM machine_unload_box_param;

            printf("��ʼ�����ºл����߼�\n");

            //�ȴ����̵���
            printf("���ڵȴ����̵������豸Ϊ %s", myDB.equip_info.actual_name.c_str());

            //���̵������������޺���
            while (g_flag_IsMainRunning && myUA.machine_unload_box_info.value_current_status != "11")
            {
                this_thread::sleep_for(chrono::milliseconds(5));
            }

            //�޸��豸״̬Ϊworking
            g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                     TABLE_EQUIPMENTINFO,{"status","working","guid",myDB.equip_info.guid});

            //RFID��־λ��1
            myUA.UA_writevalue(machine_unload_box_param.flag_RFID,"1");

            for(int t=0;t<10;t++)
            {
                //���RFID��ֵ
                if((myUA.machine_unload_box_info.value_RFID_readdata != "0")&&
                        (myUA.machine_unload_box_info.value_RFID_readdata != ""))
                {
                    //�����Ļ���break������ִ�������
                    break;
                }
                this_thread::sleep_for(chrono::milliseconds(500));
            }

            //�����Ƕ�����RFID���򵯳�������һ�㲻������������
            if((myUA.machine_unload_box_info.value_RFID_readdata == "0")&&
                (myUA.machine_unload_box_info.value_RFID_readdata == ""))
            {

                //�ȸĳ�0���ٷ���
                myUA.UA_writevalue(machine_unload_box_param.inputcommand,"0");

                //д��14����
                myUA.UA_writevalue(machine_unload_box_param.inputcommand,"14");

                //RFID��־λ��0
                myUA.UA_writevalue(machine_unload_box_param.flag_RFID,"0");

                //�޸��豸״̬Ϊready
                g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                         TABLE_EQUIPMENTINFO,{"status","ready","guid",myDB.equip_info.guid});

                //�������switch-case
                break;
            }

            //���������ݶ���������guid���ó���Ʒ��ip���Լ�����deque
            myDB.getProductInfofrommysql(myUA.machine_unload_box_info.value_RFID_readdata);

            //RFID��־λ��0���رն�дRFID
            myUA.UA_writevalue(machine_unload_box_param.flag_RFID,"0");

            //������û�зֵ����κβ�Ʒ
            if(myDB.product_row == 0)
            {
                //�ȸĳ�0���ٷ���
                myUA.UA_writevalue(machine_unload_box_param.inputcommand,"0");

                //д��14����
                myUA.UA_writevalue(machine_unload_box_param.inputcommand,"14");

                //RFID��־λ��0
                myUA.UA_writevalue(machine_unload_box_param.flag_RFID,"0");

                //�޸��豸״̬Ϊready
                g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                         TABLE_EQUIPMENTINFO,{"status","ready","guid",myDB.equip_info.guid});

                //�������switch-case
                break;
            }

            //���������ݲ�Ʒ��guid���ó��ò�Ʒȫ���Ĳ���
            myDB.getOperationInfoAllfrommysql(myDB.product_info.guid);


            //��¼����豸�����ǵڼ�������
            int num_oper = 0;

            for(int t =0; t <= myDB.oper_all_row ; t++)
            {
                if(myDB.deque_oper_all_info.at(t).name == "Operation_UB")
                {
                    num_oper = t;
                    break;
                }
            }

            //Ȼ��Ϳ��������ǰ��Ĳ����������û
            for(int t =0;t <= num_oper ; t++)
            {
                if((myDB.deque_oper_all_info.at(t).status != "done")&&(t < num_oper))
                {
                    //��ǰ�滹�й���û����ɣ��Ǿ�break��forѭ����ִ�з���
                    break;
                }

                if(myDB.deque_oper_all_info.at(t).status == "done")
                {
                    //�������������ˣ���ô���������һ������
                    continue;
                }

                //����break������continue����������һ��
                if(t == num_oper)
                {

                    string currenttime = GetCurrentSystemTime();

                    //�޸Ĳ���״̬Ϊworking
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_OPERATIONINFO,{"status","working",
                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});
                    //����״̬��ʱ��
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_OPERATIONINFO,{"start_time",currenttime,
                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});

                    //�޸�����״̬Ϊworking
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_FEATUREINFO,{"status","working",
                                                                                "id",to_string(myDB.deque_feature_info.at(5).id)});

                    //����״̬��ʱ��
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_FEATUREINFO,{"start_time",currenttime,
                                                                                "id",to_string(myDB.deque_feature_info.at(5).id)});

                    //ִ�в���
                    myUA.UA_writevalue(machine_unload_box_param.inputcommand, "1");//д��1--ץȡ

                    //�ȵ�ץȡ���
                    while (g_flag_IsMainRunning && myUA.machine_unload_box_info.value_current_status != "12")
                    {
                        this_thread::sleep_for(chrono::milliseconds(500));
                    }

                    string currenttime_1 = GetCurrentSystemTime();

                    //�޸Ĳ���״̬Ϊdone
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_OPERATIONINFO,{"status","done",
                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});

                    //����״̬��ʱ��
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_OPERATIONINFO,{"end_time",currenttime_1,
                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});


                    //�޸�����״̬Ϊdone
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_FEATUREINFO,{"status","done",
                                                                                "id",to_string(myDB.deque_feature_info.at(5).id)});

                    //����״̬��ʱ��
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_FEATUREINFO,{"end_time",currenttime_1,
                                                                                "id",to_string(myDB.deque_feature_info.at(5).id)});



                    //��Ʒ״̬���
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_PRODUCTINFO,{"status","done",
                                                                                "guid",myDB.product_info.guid});
                    //��Ʒ״̬��ɼ�ʱ��
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_PRODUCTINFO,{"actual_departure_time",currenttime_1,
                                                                                "guid",myDB.product_info.guid});

                    //�ȸĳ�0���ٷ���
                    myUA.UA_writevalue(machine_unload_box_param.inputcommand,"0");

                    //д��14����
                    myUA.UA_writevalue(machine_unload_box_param.inputcommand,"14");

                }

            }

            //RFID��־λ��0
            myUA.UA_writevalue(machine_unload_box_param.flag_RFID,"0");


            //�ȸĳ�0���ٷ���
            myUA.UA_writevalue(machine_unload_box_param.inputcommand,"0");

            //д��14����
            myUA.UA_writevalue(machine_unload_box_param.inputcommand,"14");




            //�����޸��豸״̬Ϊready
            g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                     TABLE_EQUIPMENTINFO,{"status","ready",
                                                                          "guid",myDB.equip_info.guid});

        }

            break;

        case AGV:
        {
            printf("888fssfdfds\n");
        }
            break;

        case NOPE:
        {
            printf("�����ϰ�������\n");
        }
            break;

        default:
            break;


        }





    }

    /*

    //    while(1)
    //    {

    //        printf("��ʼ\n");

    //        //���������ݶ���������guid���ó���Ʒ��ip���Ѿ�����deque

    //        myDB.getProductInfofrommysql("111");

    //        //void getOperationInfofrommysql(string product_guid);
    //        printf("%s\n",myDB.product_info.guid.c_str());

    //        printf("%s\n",myDB.product_info.pallet_ip_port.c_str());


    //        //����豸UA�Ƿ����Ӳ�����

    //        //       UA_connect_product_TCP(product_url);

    //        this_thread::sleep_for(chrono::seconds(2));

    //        //Sleep(2000);
    //    }

    //thrd_connect_mysql.detach();

    //thrd_connect_equipment_ua.detach();

*/




}




