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
        printf("请输入设备ip及端口：--ip --ipport\n");
        printf("比如：192.168.137.221 4844");
        return 0;
    }

    if (argc == 2)
    {
        printf("请输入d端口号\n");
        printf("比如：192.168.1.118 4844");
        return 0;
    }

    if (argc == 3)
    {
        g_opcuaIP = argv[1];
        g_opcuaPort = argv[2];
    }

    g_flag_IsMySQLConnected = myDB.init_mysql();

    myDB.getEquipmentInfofrommysql();

    //检测数据库是否断线并重连
    thread thrd_connect_mysql(&CDB::connect_mysql,&myDB);

    //检测设备UA是否连接并重连
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

    /*--上盒机线程，若是不是上盒机，直接完成--*/


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


    /*--传送带线程，若是不是传送带，直接完成--*/

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


    /*--机器手ODG线程，若是不是机器手ODG，直接完成--*/

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

    /*--其他机器手线程，若是机器手ODG，直接完成--*/

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


    /*--下盒机线程，若是不是下盒机，直接完成--*/

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


    /*--CNC线程，若是不是CNC，直接完成--*/

    //CNC一个需要read的都没有
    //    thread thrd_UA_read_cnc_info_value_RFID_readdata
    //            (&CUA::UA_readvalue_thrd,&myUA,
    //             thrd_cnc,
    //             robot_param.RFID_readdata,
    //             &myUA.robot_info.value_RFID_readdata);

    //    thrd_UA_read_cnc_info_value_RFID_readdata.detach();


    /*--激光线程，若不是激光，直接完成--*/

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


    /*--上盖机线程，若不是上盖，直接完成--*/

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


    /*这里开始写逻辑*/

    while(g_flag_IsMainRunning)
    {
        TYPE_EQUIPMENT my_equipment = myDB.get_TYPE_EQUIPMENT_Enum(myDB.equip_info.type);

        string str1, str2;


        this_thread::sleep_for(chrono::milliseconds(100));

        switch(my_equipment)
        {
        /*--上盒机已测试完成  10.25 16:08--*/
        case MACHINE_UPLOAD_BOX:
        {
            MACHINE_UPLOAD_BOX_PARAM machine_upload_box_param;

            printf("开始运行上盒机的逻辑\n");

            //等待托盘到达
            printf("正在等待托盘到来，设备为 %s", myDB.equip_info.actual_name.c_str());

            //托盘到达且托盘上无盒子
            while (g_flag_IsMainRunning && myUA.machine_upload_box_info.value_current_status != "11")
            {
                this_thread::sleep_for(chrono::milliseconds(500));
            }

            //修改设备状态为working
            g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,TABLE_EQUIPMENTINFO,{"status","working","guid",myDB.equip_info.guid});

            //RFID标志位置1
            myUA.UA_writevalue(machine_upload_box_param.flag_RFID,"1");

            for(int t=0;t<10;t++)
            {
                //检测RFID的值
                if((myUA.machine_upload_box_info.value_RFID_readdata != "0")&&
                        (myUA.machine_upload_box_info.value_RFID_readdata != ""))
                {
                    //读到的话，break出来，执行下面的
                    break;
                }
                this_thread::sleep_for(chrono::milliseconds(500));
            }

            //若还是读不出RFID，则弹出，不过一般不会出现这种情况
            if((myUA.machine_upload_box_info.value_RFID_readdata == "0")||
                    (myUA.machine_upload_box_info.value_RFID_readdata == ""))
            {

                //先改成0，再放行
                myUA.UA_writevalue(machine_unload_box_param.inputcommand,"0");

                //写入14放行
                myUA.UA_writevalue(machine_upload_box_param.columns,"14");

                //重新修改设备状态为ready
                g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                         TABLE_EQUIPMENTINFO,{"status","ready",
                                                                              "guid",myDB.equip_info.guid});
                //RFID标志位置0
                myUA.UA_writevalue(machine_upload_box_param.flag_RFID,"0");
                //跳出这个switch-case,紧接着下一个
                break;
            }

            //函数，根据读到的托盘guid，得出产品的ip，以及特征deque
            myDB.getProductInfofrommysql(myUA.machine_upload_box_info.value_RFID_readdata);

            //RFID标志位置0，关闭读写RFID
            myUA.UA_writevalue(machine_upload_box_param.flag_RFID,"0");

            //该托盘没有分到到任何产品
            if(myDB.product_row == 0)
            {

                //先改成0，再放行
                myUA.UA_writevalue(machine_unload_box_param.inputcommand,"0");

                //写入14放行
                myUA.UA_writevalue(machine_upload_box_param.columns,"14");

                //重新修改设备状态为ready
                g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                         TABLE_EQUIPMENTINFO,{"status","ready",
                                                                              "guid",myDB.equip_info.guid});

                //RFID标志位置0，关闭读写RFID
                myUA.UA_writevalue(machine_upload_box_param.flag_RFID,"0");

                //跳出这个switch-case
                break;
            }

            //函数，根据产品的guid，得出该产品该设备的操作
            myDB.getOperationInfofrommysql(myDB.product_info.guid);

            //若是这个产品不需要这个设备干活
            if(myDB.oper_row == 0)
            {
                //先改成0，再放行
                myUA.UA_writevalue(machine_upload_box_param.columns,"0");

                //写入14放行
                myUA.UA_writevalue(machine_upload_box_param.columns,"14");

                //重新修改设备状态为ready
                g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                         TABLE_EQUIPMENTINFO,{"status","ready",
                                                                              "guid",myDB.equip_info.guid});

                //跳出这个switch-case
                break;
            }
            else
            {
                //要是有两个操作的话，先做第一个。第一个完成了就做第二个
                for(int t = 0;t < myDB.oper_row;t++)
                {

                    if(myDB.deque_oper_info.at(t).status == "ready")
                    {
                        //该操作是ready状态
                        //修改操作状态为working
                        g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                 TABLE_OPERATIONINFO,{"status","ready",
                                                                                      "id",to_string(myDB.deque_oper_info.at(t).id)});

                        //执行这个
                        printf("托盘到位，正在发送列数指令，参数为 %s\n", myDB.deque_oper_info.at(t).param.c_str());


                        //写入列数
                        myUA.UA_writevalue(machine_upload_box_param.columns,
                                           myDB.deque_oper_info.at(t).param);


                        //产品状态改成working
                        g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                 TABLE_PRODUCTINFO,{"status","working",
                                                                                    "guid",myDB.product_info.guid});

                        //修改操作状态为working
                        g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                 TABLE_OPERATIONINFO,{"status","working",
                                                                                      "id",to_string(myDB.deque_oper_info.at(t).id)});
                        string strCurTime = GetCurrentSystemTime();

                        //操作状态加时间
                        g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                 TABLE_OPERATIONINFO,{"start_time",strCurTime,
                                                                                      "id",to_string(myDB.deque_oper_info.at(t).id)});


                        //产品特征1改成working
                        g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                 TABLE_FEATUREINFO,{"status","working",
                                                                                    "id",to_string(myDB.deque_feature_info.at(0).id)});



                        //产品特征1加时间
                        g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                 TABLE_FEATUREINFO,{"start_time",strCurTime,
                                                                                    "id",to_string(myDB.deque_feature_info.at(0).id)});


                        //等到盒子已经放在放托盘上
                        while (g_flag_IsMainRunning && myUA.machine_upload_box_info.value_current_status != "12")
                        {
                            this_thread::sleep_for(chrono::milliseconds(500));
                        }

                        string strCurTime_2 = GetCurrentSystemTime();
                        //产品到达产线加时间
                        g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                 TABLE_PRODUCTINFO,{"actual_arrival_time",strCurTime_2,
                                                                                    "guid",myDB.product_info.guid});

                        //先改成0，再放行
                        myUA.UA_writevalue(machine_upload_box_param.columns,"0");

                        //写入14放行
                        myUA.UA_writevalue(machine_upload_box_param.columns,"14");


                        string strCurTime_3 = GetCurrentSystemTime();

                        //向产品特征状态为done
                        g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                 TABLE_FEATUREINFO,{"status","done",
                                                                                    "id",to_string(myDB.deque_feature_info.at(0).id)});

                        //向产品特征状态加时间
                        g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                 TABLE_FEATUREINFO,{"end_time",strCurTime_3,
                                                                                    "id",to_string(myDB.deque_feature_info.at(0).id)});


                        //修改操作状态为done
                        g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                 TABLE_OPERATIONINFO,{"status","done",
                                                                                      "id",to_string(myDB.deque_oper_info.at(t).id)});

                        //修改操作状态加时间
                        g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                 TABLE_OPERATIONINFO,{"end_time",strCurTime_3,
                                                                                      "id",to_string(myDB.deque_oper_info.at(t).id)});


                        //重新修改设备状态为ready
                        g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                 TABLE_EQUIPMENTINFO,{"status","ready",
                                                                                      "guid",myDB.equip_info.guid});

                    }
                    else
                    {
                        //不是ready的话，下一个
                        continue;
                    }
                }

                //若多个操作都不是ready状态，则放行

                //先改成0，再放行
                myUA.UA_writevalue(machine_upload_box_param.columns,"0");

                //写入14放行
                myUA.UA_writevalue(machine_upload_box_param.columns,"14");

                //RFID标志位置0，关闭读写RFID
                myUA.UA_writevalue(machine_upload_box_param.flag_RFID,"0");

                //重新修改设备状态为ready
                g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                         TABLE_EQUIPMENTINFO,{"status","ready",
                                                                              "guid",myDB.equip_info.guid});

            }

        }
            break;

            /*--传送带5调试完毕 2020.10.25 17:30--*/
            /*--传送带1调试完毕 2020.10.26 01:30--*/
            /*--传送带3调试完毕 2020.10.26 01:30--*/
        case CONVEYOR:
        {
            CONVEYOR_PARAM conveyor_param;

            printf("开始运行传送带%i的逻辑\n",myDB.equip_info.type_subindex);
            //等待托盘到达

            //等待托盘到达
            printf("正在等待托盘到来，设备为 %s\n", myDB.equip_info.actual_name.c_str());

            //开始先写2，挡板放行
            myUA.UA_writevalue(conveyor_param.cylinder_status,"2");

            this_thread::sleep_for(chrono::milliseconds(500));

            //一直等到光电感应到
            while (g_flag_IsMainRunning && myUA.conveyor_info.value_cylinder_status != "1")
            {
                this_thread::sleep_for(chrono::milliseconds(500));
            }

            //修改设备状态为working
            g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,TABLE_EQUIPMENTINFO,
            {"status","working","guid",myDB.equip_info.guid});

            //RFID标志位置1
            myUA.UA_writevalue(conveyor_param.flag_RFID,"1");

            for(int t=0;t<10;t++)
            {
                //检测RFID的值
                if((myUA.conveyor_info.value_RFID_readdata != "0")&&
                        ((myUA.conveyor_info.value_RFID_readdata != "")))
                {
                    //读到的话，break出来，执行下面的
                    break;
                }
                this_thread::sleep_for(chrono::milliseconds(500));

            }

            //若还是读不出RFID，则放行（直行），不过一般不会出现这种情况
            if((myUA.conveyor_info.value_RFID_readdata == "0")||
                    ((myUA.conveyor_info.value_RFID_readdata == "")))
            {
                //写2,默认直行
                myUA.UA_writevalue(conveyor_param.corner_switch,"2");

                //等一下
                this_thread::sleep_for(chrono::seconds(1));

                //写2，挡板放行
                myUA.UA_writevalue(conveyor_param.cylinder_status,"2");

                //RFID标志位置0，关闭读写RFID
                myUA.UA_writevalue(conveyor_param.flag_RFID,"0");

                //重新修改设备状态为ready
                g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                         TABLE_EQUIPMENTINFO,{"status","ready",
                                                                              "guid",myDB.equip_info.guid});
                //跳出这个switch-case
                break;
            }

            //函数，根据读到的托盘guid，得出产品的ip，以及特征deque
            myDB.getProductInfofrommysql(myUA.conveyor_info.value_RFID_readdata);

            //RFID标志位置0，关闭读写RFID
            myUA.UA_writevalue(conveyor_param.flag_RFID,"0");


            //该托盘没有分到到任何产品
            if(myDB.product_row == 0)
            {
                //写2,默认直行
                myUA.UA_writevalue(conveyor_param.corner_switch,"2");

                //等一下
                this_thread::sleep_for(chrono::milliseconds(1500));

                //写2，挡板放行
                myUA.UA_writevalue(conveyor_param.cylinder_status,"2");

                //RFID标志位置0，关闭读写RFID
                myUA.UA_writevalue(conveyor_param.flag_RFID,"0");

                //重新修改设备状态为ready
                g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                         TABLE_EQUIPMENTINFO,{"status","ready",
                                                                              "guid",myDB.equip_info.guid});

                //跳出这个switch-case
                break;
            }

            //函数，根据产品的guid，得出该产品全部的操作
            myDB.getOperationInfoAllfrommysql(myDB.product_info.guid);


            //记录这个设备操作是第几个操作
            int num_oper = 0;

            for(int k =0;k < myDB.oper_all_row ; k++)
            {
                //传送带还要跟设备绑定
                if((myDB.deque_oper_all_info.at(k).name == "Operation_switch")&&
                        (myDB.deque_oper_all_info.at(k).equipment_guid == myDB.equip_info.guid)&&
                        (myDB.deque_oper_all_info.at(k).status == "ready"))
                {
                    num_oper = k;
                    //找到就break出这个for
                    break;

                }
            }

            //然后就看这个操作前面的操作都完成了没
            for(int t =0;t <= num_oper ; t++)
            {
                if((myDB.deque_oper_all_info.at(t).status != "done")&&(t < num_oper))
                {
                    //若前面还有工序没有完成，那就break出for循环，执行放行
                    break;
                }

                if(myDB.deque_oper_all_info.at(t).status == "done")
                {
                    //若这个工序完成了，那么继续检查下一个工序
                    continue;
                }

                //不被break，不被continue才能来到这一步
                if(t == num_oper)
                {
                    //该操作是ready状态

                    //修改操作状态为working
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_OPERATIONINFO,{"status","ready",
                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});

                    string strCurTime = GetCurrentSystemTime();

                    //增加操作状态时间
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_OPERATIONINFO,{"start_time",strCurTime,
                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});

                    //执行这个
                    printf("托盘到位，正在发送是否转弯指令，参数为 %s\n", myDB.deque_oper_all_info.at(t).param.c_str());


                    //写入命令
                    myUA.UA_writevalue(conveyor_param.corner_switch,
                                       myDB.deque_oper_all_info.at(t).param);

                    //等一下
                    this_thread::sleep_for(chrono::seconds(1));

                    //写2，挡板放行
                    myUA.UA_writevalue(conveyor_param.cylinder_status,"2");

                    //修改操作状态为done
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_OPERATIONINFO,{"status","done",
                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});

                    string strCurTime_2 = GetCurrentSystemTime();

                    //增加操作状态时间
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_OPERATIONINFO,{"end_time",strCurTime_2,
                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});

                    //重新修改设备状态为ready
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_EQUIPMENTINFO,{"status","ready",
                                                                                  "guid",myDB.equip_info.guid});
                    //若是完成了一个操作后，退出这个for，一次只能做一个操作
                    break;

                }

                //跳出这个for
                //break;
            }


            //重新修改设备状态为ready
            g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                     TABLE_EQUIPMENTINFO,{"status","ready",
                                                                          "guid",myDB.equip_info.guid});



        }
            break;

            /*--ODG机器手调试完毕 2020.10.26 00:08--*/
            /*--Densou调试完成，2020.10.28 15:55-*/
            /*--SCUT调试完毕 2020.10.28 04:32--*/
        case ROBOT:
        {

            ROBOT_PARAM robot_param;

            printf("开始运行机器手的逻辑\n");

            //等待托盘到达
            printf("正在等待托盘到来，设备为 %s", myDB.equip_info.actual_name.c_str());

            //托盘到达（光电开关状态由1->0）
            while (g_flag_IsMainRunning && myUA.robot_info.value_switch_status != "1")
            {
                this_thread::sleep_for(chrono::milliseconds(2));
            }

            while (g_flag_IsMainRunning && myUA.robot_info.value_switch_status != "0")
            {
                this_thread::sleep_for(chrono::milliseconds(2));
            }

            myUA.UA_writevalue(robot_param.guard2_status, "1");//写入1-挡板2触发
            this_thread::sleep_for(chrono::seconds(3));
            myUA.UA_writevalue(robot_param.guard1_status, "1");//写入1-挡板1触发


            //修改设备状态为working
            g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                     TABLE_EQUIPMENTINFO,{"status","working","guid",myDB.equip_info.guid});

            //RFID标志位置1
            myUA.UA_writevalue(robot_param.flag_RFID,"1");

            for(int t=0;t<10;t++)
            {
                //检测RFID的值
                if((myUA.robot_info.value_RFID_readdata != "0")&&
                        (myUA.robot_info.value_RFID_readdata != ""))
                {
                    //读到的话，break出来，执行下面的
                    break;
                }
                this_thread::sleep_for(chrono::milliseconds(500));

            }

            //若还是读不出RFID，则放行，不过一般不会出现这种情况
            if((myUA.robot_info.value_RFID_readdata == "0")||
                    (myUA.robot_info.value_RFID_readdata == ""))
            {
                //放行
                myUA.UA_writevalue(robot_param.guard1_status, "0");//写入0-挡板1触发
                this_thread::sleep_for(chrono::milliseconds(800));
                myUA.UA_writevalue(robot_param.guard2_status, "0");//写入0-挡板2触发

                //RFID标志位置0
                myUA.UA_writevalue(robot_param.flag_RFID,"0");

                //修改设备状态为ready
                g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                         TABLE_EQUIPMENTINFO,{"status","ready","guid",myDB.equip_info.guid});

                //跳出switch-case
                break;
            }

            //函数，根据读到的托盘guid，得出产品的ip，以及特征deque
            myDB.getProductInfofrommysql(myUA.robot_info.value_RFID_readdata);

            //RFID标志位置0
            myUA.UA_writevalue(robot_param.flag_RFID,"0");

            //该托盘没有分到到任何产品
            if(myDB.product_row == 0)
            {
                //放行
                myUA.UA_writevalue(robot_param.guard1_status, "0");//写入0-挡板1触发
                this_thread::sleep_for(chrono::milliseconds(800));
                myUA.UA_writevalue(robot_param.guard2_status, "0");//写入0-挡板2触发

                //RFID标志位置0
                myUA.UA_writevalue(robot_param.flag_RFID,"0");

                //修改设备状态为ready
                g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                         TABLE_EQUIPMENTINFO,{"status","ready","guid",myDB.equip_info.guid});

                //跳出switch-case
                break;

            }


            //两个都是获得deque，一个是设备*产品，一个是产品
            //函数，根据产品的guid，得出该产品该设备的操作
            myDB.getOperationInfofrommysql(myDB.product_info.guid);

            //函数，根据产品的guid，得出该产品该设备的操作
            myDB.getOperationInfoAllfrommysql(myDB.product_info.guid);

            //RFID标志位置1。这里置1，用于后面CNC的读值
            //myUA.UA_writevalue(robot_param.flag_RFID,"1");

            string oper_name;

            int num_oper = 0;

            //若是这个产品不需要这个设备干活
            if(myDB.oper_row == 0)
            {
                //放行
                myUA.UA_writevalue(robot_param.guard1_status, "0");//写入0-挡板1触发
                this_thread::sleep_for(chrono::milliseconds(800));
                myUA.UA_writevalue(robot_param.guard2_status, "0");//写入0-挡板2触发

                //RFID标志位置0
                myUA.UA_writevalue(robot_param.flag_RFID,"0");

                //修改设备状态为ready
                g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                         TABLE_EQUIPMENTINFO,{"status","ready","guid",myDB.equip_info.guid});

                //跳出这个switch-case
                break;
            }
            else
            {
                //要是有两个操作的话，先做第一个。第一个完成了就做第二个
                for(int u = 0;u < myDB.oper_row;u++)
                {
                    oper_name = myDB.deque_oper_info.at(u).name;

                    if(myDB.deque_oper_info.at(u).status == "ready")
                    {

                        //还要它前面的已完成了没
                        //记录这个设备的操作是产品的第几个操作
                        for(int t =0;t < myDB.oper_all_row ; t++)
                        {
                            if(myDB.deque_oper_all_info.at(t).name == oper_name)
                            {
                                num_oper = t ;
                                //找到就退出这个for循环
                                break;
                            }
                        }

                        //然后就看这个操作前面的操作都完成了没
                        //这里是<=,因为包含在内
                        for(int t =0;t <= num_oper  ; t++)
                        {
                            //问题的根源是，即使数据库上写了Operation_pick2CNC完成，但是程序这里没有更新到
                            myDB.getOperationInfoAllfrommysql(myDB.product_info.guid);

                            //这里有个例外就是，要是判断的是CNC，则需要等CNC完成，再到下面Operation_pick2pallet的操作
                            if(myDB.deque_oper_all_info.at(t).name == "Operation_machine")
                            {
                                //不断的去扫数据库的状态，等到CNC的完成了，再执行下面的
                                while(myDB.deque_oper_all_info.at(t).status != "done")
                                {
                                    myDB.getOperationInfoAllfrommysql(myDB.product_info.guid);
                                    this_thread::sleep_for(chrono::milliseconds(800));

                                }
                                t = num_oper;
                            }

                            if((myDB.deque_oper_all_info.at(t).status != "done")&&(t < num_oper))
                            {
                                //若前面还有工序没有完成，那就break出for循环，执行放行

                                break;
                            }

                            if(myDB.deque_oper_all_info.at(t).status == "done")
                            {
                                //若这个工序完成了，那么继续检查下一个工序
                                continue;
                            }

                            //不被break，不被continue才能来到这一步
                            //来到最后一个没有done的序列
                            if(t == num_oper )
                            {

                                //执行ODG步骤
                                if(myDB.deque_oper_all_info.at(t).name == "Operation_LM")
                                {

                                    ROBOT_PARAM robot_param;

                                    printf("RFID验证完成，正在发送抓取指令，设备为 %s\n",
                                           myDB.equip_info.actual_name.c_str());

                                    string strCurTime = GetCurrentSystemTime();

                                    //修改操作状态为working
                                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                             TABLE_OPERATIONINFO,{"status","working",
                                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});

                                    //操作状态加时间
                                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                             TABLE_OPERATIONINFO,{"start_time",strCurTime,
                                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});

                                    //修改特征2状态为working
                                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                             TABLE_FEATUREINFO,{"status","working",
                                                                                                "id",to_string(myDB.deque_feature_info.at(1).id)});
                                    //特征2状态加时间
                                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                             TABLE_FEATUREINFO,{"start_time",strCurTime,
                                                                                                "id",to_string(myDB.deque_feature_info.at(1).id)});


                                    int file_input_num = atoi(myDB.deque_oper_all_info.at(t).param.c_str());

                                    if (file_input_num > 3 || file_input_num < 1)
                                    {
                                        file_input_num = 3;
                                    }


                                    myUA.UA_writevalue(robot_param.file_input[file_input_num - 1], "1");//写入1进file的标志位，选择Op参数的文件

                                    this_thread::sleep_for(chrono::seconds(3));

                                    myUA.UA_writevalue(robot_param.file_input[file_input_num - 1], "0");


                                    //指令执行完成
                                    while (g_flag_IsMainRunning && myUA.robot_info.value_current_status != "0")
                                    {
                                        this_thread::sleep_for(chrono::milliseconds(200));
                                    }
                                    while (g_flag_IsMainRunning && myUA.robot_info.value_current_status != "1")
                                    {
                                        this_thread::sleep_for(chrono::milliseconds(200));
                                    }


                                    string strCurTime_1 = GetCurrentSystemTime();

                                    //向产品 Server 特征里写入特征2已完成
                                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                             TABLE_FEATUREINFO,{"status","done",
                                                                                                "id",to_string(myDB.deque_feature_info.at(1).id)});
                                    //特征2加时间
                                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                             TABLE_FEATUREINFO,{"end_time",strCurTime_1,
                                                                                                "id",to_string(myDB.deque_feature_info.at(1).id)});

                                    //修改操作状态为done
                                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                             TABLE_OPERATIONINFO,{"status","done",
                                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});

                                    //操作状态加时间
                                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                             TABLE_OPERATIONINFO,{"end_time",strCurTime_1,
                                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});

                                }

                                //本来是识别到两个工序，做完Operation_pick2CNC，再做Operation_pick2pallet
                                //这里识别到Operation_pick2CNC，就要同时完成两个工序，也就是完成了一个先不break
                                if(myDB.deque_oper_all_info.at(t).name == "Operation_pick2CNC")
                                {

                                    ROBOT_PARAM robot_param;

                                    printf("正在发送Start1,指令，设备为 %s", myDB.equip_info.actual_name.c_str());

                                    //修改操作状态为working
                                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                             TABLE_OPERATIONINFO,{"status","working",
                                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});
                                    string currenttime = GetCurrentSystemTime();

                                    //操作状态加时间
                                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                             TABLE_OPERATIONINFO,{"start_time",currenttime,
                                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});


                                    /*--老方法，写值调用--*/

                                    //                                    myUA.UA_writevalue(robot_param.inputcommand, "Start,\n");//写入Start,开始抓取

                                    //                                    //Start,指令执行完成
                                    //                                    while (g_flag_IsMainRunning && (myUA.UA_readvalue(robot_param.current_status).find("OK1") == string::npos))
                                    //                                    {
                                    //                                        Sleep(500);
                                    //                                    }



                                    /*20201006*/

                                    /*--call方法节点调用--*/
                                    myUA.UA_writevalue(robot_param.cap_input_cmd_pick, "In");//写入In

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


                                    //修改操作状态为done
                                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                             TABLE_OPERATIONINFO,{"status","done",
                                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});
                                    string currenttime_1 = GetCurrentSystemTime();

                                    //操作状态加时间
                                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                             TABLE_OPERATIONINFO,{"end_time",currenttime_1,
                                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});


                                }


                                //完成了Operation_pick2pallet，紧接着完成这个，但是得查数据库才行
                                if(myDB.deque_oper_all_info.at(t).name == "Operation_pick2pallet")
                                {

                                    ROBOT_PARAM robot_param;

                                    printf("正在发送Back,指令，设备为 %s", myDB.equip_info.actual_name.c_str());

                                    //修改操作状态为working
                                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                             TABLE_OPERATIONINFO,{"status","working",
                                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});

                                    string currenttime = GetCurrentSystemTime();

                                    //操作状态加时间
                                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                             TABLE_OPERATIONINFO,{"start_time",currenttime,
                                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});


                                    /*20201006*/
                                    myUA.UA_writevalue(robot_param.cap_input_cmd_pick, "Out");//写入OUT

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


                                    //修改操作状态为done
                                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                             TABLE_OPERATIONINFO,{"status","done",
                                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});

                                    string currenttime_1 = GetCurrentSystemTime();

                                    //操作状态加时间
                                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                             TABLE_OPERATIONINFO,{"end_time",currenttime_1,
                                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});

                                }


                                if(myDB.deque_oper_all_info.at(t).name == "Operation_package")
                                {

                                    printf("正在发送Start1,指令，设备为 %s", myDB.equip_info.actual_name.c_str());

                                    string strCurTime = GetCurrentSystemTime();

                                    //修改操作状态为working
                                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                             TABLE_OPERATIONINFO,{"status","working",
                                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});

                                    //操作状态加时间
                                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                             TABLE_OPERATIONINFO,{"start_time",strCurTime,
                                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});

                                    //修改特征4状态为working
                                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                             TABLE_FEATUREINFO,{"status","working",
                                                                                                "id",to_string(myDB.deque_feature_info.at(3).id)});
                                    //特征4状态加时间
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

                                    //修改操作状态为done
                                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                             TABLE_OPERATIONINFO,{"status","done",
                                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});

                                    //操作状态加时间
                                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                             TABLE_OPERATIONINFO,{"end_time",strCurTime,
                                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});

                                    //修改特征4状态为working
                                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                             TABLE_FEATUREINFO,{"status","done",
                                                                                                "id",to_string(myDB.deque_feature_info.at(3).id)});
                                    //特征4状态加时间
                                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                             TABLE_FEATUREINFO,{"end_time",strCurTime,
                                                                                                "id",to_string(myDB.deque_feature_info.at(3).id)});

                                }

                            }

                        }





                        //若是完成了一个操作后，退出这个for，一次只能做一个操作
                        //这里不需要，这里就是需要一次完成两个动作
                        //break;
                    }
                    else
                    {
                        //要是这个不是ready的话，就下一个
                        continue;
                    }
                }

            }

            //放行
            myUA.UA_writevalue(robot_param.guard1_status, "0");//写入0-挡板1触发
            this_thread::sleep_for(chrono::milliseconds(800));
            myUA.UA_writevalue(robot_param.guard2_status, "0");//写入0-挡板2触发

            //RFID标志位置0
            myUA.UA_writevalue(robot_param.flag_RFID,"0");


            //重新修改设备状态为ready
            g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                     TABLE_EQUIPMENTINFO,{"status","ready",
                                                                          "guid",myDB.equip_info.guid});

        }
            break;

            /*--CNC调试完毕 2020.10.28 21:12----*/
        case CNC:
        {
            CNC_PARAM cnc_param;

            printf("开始运行CNC的逻辑\n");

            //等待托盘到达
            printf("正在等待CNC运行，设备为 %s", myDB.equip_info.actual_name.c_str());

            //所以CNC的逻辑只能从数据库入手

            while(g_flag_IsMainRunning)
            {
                this_thread::sleep_for(chrono::milliseconds(500));

                myDB.getCNCInfofrommysql();

                for(int t = 0; t < myDB.cnc_row; t++)
                {
                    //先是找pick2cnc完成的
                    if((myDB.deque_cnc_info.at(t).name == "Operation_pick2CNC")&&
                            (myDB.deque_cnc_info.at(t).status == "done"))
                    {
                        //看看pick2cncn完成的后面，是不是ready，是不是这台cnc
                        if  ((myDB.deque_cnc_info.at(t+1).name == "Operation_machine")&&
                             (myDB.deque_cnc_info.at(t+1).status == "ready")&&
                             (myDB.deque_cnc_info.at(t+1).equipment_guid == myDB.equip_info.guid))
                        {

                            //两个都满足了，就可以根据产品guid找出特征
                            myDB.getFeatureInfofrommysql(myDB.deque_cnc_info.at(t+1).product_guid);


                            printf("设备空闲，正在发送加工参数，设备为 %s", myDB.equip_info.actual_name.c_str());

                            string currenttime = GetCurrentSystemTime();


                            //修改设备状态为working
                            g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                     TABLE_EQUIPMENTINFO,{"status","working","guid",myDB.equip_info.guid});

                            //修改操作状态为working
                            g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                     TABLE_OPERATIONINFO,{"status","working",
                                                                                          "id",to_string(myDB.deque_cnc_info.at(t+1).id)});

                            //特征状态为working
                            g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                     TABLE_FEATUREINFO,{"status","working",
                                                                                        "id",to_string(myDB.deque_feature_info.at(2).id)});


                            //操作状态加时间
                            g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                     TABLE_OPERATIONINFO,{"start_time",currenttime,
                                                                                          "id",to_string(myDB.deque_cnc_info.at(t+1).id)});

                            //特征状态加时间
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

                            //修改操作状态为done
                            g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                     TABLE_OPERATIONINFO,{"status","done",
                                                                                          "id",to_string(myDB.deque_cnc_info.at(t+1).id)});


                            //修改特征状态为done
                            g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                     TABLE_FEATUREINFO,{"status","done",
                                                                                        "id",to_string(myDB.deque_feature_info.at(2).id)});
                            //操作状态加时间
                            g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                     TABLE_OPERATIONINFO,{"end_time",currenttime_1,
                                                                                          "id",to_string(myDB.deque_cnc_info.at(t+1).id)});

                            //特征状态加时间
                            g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                     TABLE_FEATUREINFO,{"end_time",currenttime_1,
                                                                                        "id",to_string(myDB.deque_feature_info.at(2).id)});

                            //修改设备状态为ready
                            g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                                     TABLE_EQUIPMENTINFO,{"status","ready","guid",myDB.equip_info.guid});
                        }

                    }

                }

            }


            //CNC没有放行

            //重新修改设备状态为ready
            g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                     TABLE_EQUIPMENTINFO,{"status","ready",
                                                                          "guid",myDB.equip_info.guid});

        }
            break;

            /*--激光已测试完成  10.28 03:42--*/
        case LASER:
        {

            LASER_PARAM laser_param;

            printf("开始运行激光的逻辑\n");

            //等待托盘到达
            printf("正在等待托盘到来，设备为 %s", myDB.equip_info.actual_name.c_str());

            //托盘到达（光电开关状态由1->0）

            while (g_flag_IsMainRunning && myUA.laser_info.value_switch_status != "1")
            {
                this_thread::sleep_for(chrono::milliseconds(2));
            }

            while (g_flag_IsMainRunning && myUA.laser_info.value_switch_status != "0")
            {
                this_thread::sleep_for(chrono::milliseconds(2));
            }

            myUA.UA_writevalue(laser_param.guard2_status, "1");//写入1-挡板2触发
            this_thread::sleep_for(chrono::seconds(2));
            myUA.UA_writevalue(laser_param.guard1_status, "1");//写入1-挡板1触发

            //修改设备状态为working
            g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,TABLE_EQUIPMENTINFO,{"status","working","guid",myDB.equip_info.guid});

            //RFID标志位置1
            myUA.UA_writevalue(laser_param.flag_RFID,"1");

            for(int t=0;t<10;t++)
            {
                //检测RFID的值
                if((myUA.laser_info.value_RFID_readdata != "0")&&
                        (myUA.laser_info.value_RFID_readdata != ""))
                {
                    //读到的话，break出来，执行下面的
                    break;
                }
                this_thread::sleep_for(chrono::milliseconds(500));
            }

            //若还是读不出RFID，则弹出，不过一般不会出现这种情况
            if((myUA.laser_info.value_RFID_readdata == "0")||
                (myUA.laser_info.value_RFID_readdata == ""))
            {

                //放行
                myUA.UA_writevalue(laser_param.guard1_status, "0");//写入0-挡板1触发
                this_thread::sleep_for(chrono::milliseconds(800));
                myUA.UA_writevalue(laser_param.guard2_status, "0");//写入0-挡板2触发 //跳出这个switch-case

                //RFID标志位置0
                myUA.UA_writevalue(laser_param.flag_RFID,"0");

                //修改设备状态为ready
                g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                         TABLE_EQUIPMENTINFO,{"status","ready","guid",myDB.equip_info.guid});

                //跳出这个switch-case
                break;
            }


            //函数，根据读到的托盘guid，得出产品的ip，以及特征deque
            myDB.getProductInfofrommysql(myUA.laser_info.value_RFID_readdata);

            //RFID标志位置0，关闭读写RFID
            myUA.UA_writevalue(laser_param.flag_RFID,"0");

            //该托盘没有分到到任何产品
            if(myDB.product_row == 0)
            {

                //放行
                myUA.UA_writevalue(laser_param.guard1_status, "0");//写入0-挡板1触发
                this_thread::sleep_for(chrono::milliseconds(800));
                myUA.UA_writevalue(laser_param.guard2_status, "0");//写入0-挡板2触发 //跳出这个switch-case

                //RFID标志位置0
                myUA.UA_writevalue(laser_param.flag_RFID,"0");

                //修改设备状态为ready
                g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,TABLE_EQUIPMENTINFO,{"status","ready","guid",myDB.equip_info.guid});

                //跳出这个switch-case
                break;
            }

            //函数，根据产品的guid，得出该产品全部的操作
            myDB.getOperationInfoAllfrommysql(myDB.product_info.guid);


            //记录这个设备操作是第几个操作
            int num_oper = 0;

            for(int k =0;k < myDB.oper_all_row ; k++)
            {
                if(myDB.deque_oper_all_info.at(k).name == "Operation_laser")
                {
                    num_oper = k;
                    break;
                }
            }

            //然后就看这个操作前面的操作都完成了没
            for(int t =0;t <= num_oper ; t++)
            {
                if((myDB.deque_oper_all_info.at(t).status != "done")&&(t < num_oper))
                {
                    //若前面还有工序没有完成，那就break出for循环，执行放行
                    break;
                }

                if(myDB.deque_oper_all_info.at(t).status == "done")
                {
                    //若这个工序完成了，那么继续检查下一个工序
                    continue;
                }

                //不被break，不被continue才能来到这一步
                if( t == num_oper )
                {
                    //执行步骤

                    //修改操作状态为working
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_OPERATIONINFO,{"status","working",
                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});


                    //修改特征状态为working
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_FEATUREINFO,{"status","working",
                                                                                "id",to_string(myDB.deque_feature_info.at(2).id)});


                    int file_input_num = atoi(myDB.deque_oper_all_info.at(t).param.c_str());

                    if (file_input_num > 8 || file_input_num < 1)
                    {
                        file_input_num = 8;
                    }

                    string currenttime = GetCurrentSystemTime();

                    //操作状态加时间
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_OPERATIONINFO,{"start_time",currenttime,
                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});

                    //特征状态加时间
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_FEATUREINFO,{"start_time",currenttime,
                                                                                "id",to_string(myDB.deque_feature_info.at(2).id)});


                    myUA.UA_writevalue(laser_param.file_input[file_input_num - 1], "1");//写入1进file的标志位，选择Op参数的文件
                    Sleep(1000);
                    myUA.UA_writevalue(laser_param.file_input[file_input_num - 1], "0");


                    //****激光打标机的输出信号时不时会抖动，此处做了消抖处理
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

                    //修改操作状态为done
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_OPERATIONINFO,{"status","done",
                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});


                    //向产品 Server 特征里写入特征1已完成,本来是写产品Server的，先这样先吧
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_FEATUREINFO,{"status","done",
                                                                                "id",to_string(myDB.deque_feature_info.at(2).id)});

                    string currenttime_1 = GetCurrentSystemTime();

                    //操作状态加时间
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_OPERATIONINFO,{"end_time",currenttime_1,
                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});

                    //特征状态加时间
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_FEATUREINFO,{"end_time",currenttime_1,
                                                                                "id",to_string(myDB.deque_feature_info.at(2).id)});

                }

            }

            //RFID标志位置0
            myUA.UA_writevalue(laser_param.flag_RFID,"0");

            //放行
            myUA.UA_writevalue(laser_param.guard1_status, "0");//写入0-挡板1触发
            this_thread::sleep_for(chrono::milliseconds(800));
            myUA.UA_writevalue(laser_param.guard2_status, "0");//写入0-挡板2触发



            //重新修改设备状态为ready
            g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                     TABLE_EQUIPMENTINFO,{"status","ready",
                                                                          "guid",myDB.equip_info.guid});

        }
            break;


        case MACHINE_UPLOAD_COVER:
        {

            MACHINE_UPLOAD_COVER_PARAM machine_upload_cover_param;

            printf("开始运行上盖机的逻辑\n");

            //等待托盘到达
            printf("正在等待托盘到来，设备为 %s", myDB.equip_info.actual_name.c_str());

            //托盘到达
            while (g_flag_IsMainRunning && myUA.machine_upload_cover_info.value_current_status != "11")
            {
                this_thread::sleep_for(chrono::milliseconds(500));
            }

            //修改设备状态为working
            g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                     TABLE_EQUIPMENTINFO,{"status","working","guid",myDB.equip_info.guid});

            //RFID标志位置1
            myUA.UA_writevalue(machine_upload_cover_param.flag_RFID,"1");

            for(int t=0;t<10;t++)
            {
                //检测RFID的值
                if((myUA.machine_upload_cover_info.value_RFID_readdata != "0")&&
                    (myUA.machine_upload_cover_info.value_RFID_readdata != ""))
                {
                    //读到的话，break出来，执行下面的
                    break;
                }
                this_thread::sleep_for(chrono::milliseconds(500));
            }

            //若还是读不出RFID，则弹出，不过一般不会出现这种情况
            if((myUA.machine_upload_cover_info.value_RFID_readdata == "0")||
                (myUA.machine_upload_cover_info.value_RFID_readdata == ""))
            {
                //
                myUA.UA_readvalue(machine_upload_cover_param.columns);

                //先写0
                myUA.UA_writevalue(machine_upload_cover_param.columns,"0");

                //写入14放行
                myUA.UA_writevalue(machine_upload_cover_param.columns,"14");

                //RFID标志位置0
                myUA.UA_writevalue(machine_upload_cover_param.flag_RFID,"0");

                //修改设备状态为ready
                g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                         TABLE_EQUIPMENTINFO,{"status","ready","guid",myDB.equip_info.guid});

                //跳出这个switch-case
                break;
            }

            //函数，根据读到的托盘guid，得出产品的ip，以及特征deque
            myDB.getProductInfofrommysql(myUA.machine_upload_cover_info.value_RFID_readdata);

            //RFID标志位置0，关闭读写RFID
            myUA.UA_writevalue(machine_upload_cover_param.flag_RFID,"0");

            //该托盘没有分到到任何产品
            if(myDB.product_row == 0)
            {
                //
                myUA.UA_readvalue(machine_upload_cover_param.columns);

                //先写0
                myUA.UA_writevalue(machine_upload_cover_param.columns,"0");

                //写入14放行
                myUA.UA_writevalue(machine_upload_cover_param.columns,"14");

                //RFID标志位置0
                myUA.UA_writevalue(machine_upload_cover_param.flag_RFID,"0");

                //修改设备状态为ready
                g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                         TABLE_EQUIPMENTINFO,{"status","ready","guid",myDB.equip_info.guid});

                //跳出这个switch-case
                break;
            }

            //函数，根据产品的guid，得出该产品全部的操作
            myDB.getOperationInfoAllfrommysql(myDB.product_info.guid);


            //记录这个设备操作是第几个操作
            int num_oper = 0;

            for(int t =0;t < myDB.oper_all_row ; t++)
            {
                if(myDB.deque_oper_all_info.at(t).name == "Operation_LC")
                {
                    num_oper = t;
                    //找到就break出这个for
                    break;

                }
            }

            //然后就看这个操作前面的操作都完成了没
            for(int t =0;t <= num_oper ; t++)
            {
                if((myDB.deque_oper_all_info.at(t).status != "done")&&(t < num_oper))
                {
                    //若前面还有工序没有完成，那就break出for循环，执行放行
                    break;
                }

                if(myDB.deque_oper_all_info.at(t).status == "done")
                {
                    //若这个工序完成了，那么继续检查下一个工序
                    continue;
                }

                //不被break，不被continue才能来到这一步
                if(t == num_oper)
                {

                    //写入上盖列数
                    //                    myUA.UA_writevalue(machine_upload_cover_param.columns,
                    //                                myDB.deque_oper_all_info.at(t).param);

                    myUA.UA_writevalue(machine_upload_cover_param.columns,"2");

                    string currenttime = GetCurrentSystemTime();

                    //修改操作状态为working
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_OPERATIONINFO,{"status","working",
                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});

                    //修改特征状态为working
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_FEATUREINFO,{"status","working",
                                                                                "id",to_string(myDB.deque_feature_info.at(4).id)});

                    //操作状态加时间
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_OPERATIONINFO,{"start_time",currenttime,
                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});

                    //特征状态加时间
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_FEATUREINFO,{"start_time",currenttime,
                                                                                "id",to_string(myDB.deque_feature_info.at(4).id)});


                    //等待上盖完成
                    while (g_flag_IsMainRunning && myUA.machine_upload_cover_info.value_current_status!= "12")
                    {
                        this_thread::sleep_for(chrono::milliseconds(500));
                    }

                    string currenttime_1 = GetCurrentSystemTime();


                    //修改操作状态为done
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_OPERATIONINFO,{"status","done",
                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});


                    //修改特征状态为done
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_FEATUREINFO,{"status","done",
                                                                                "id",to_string(myDB.deque_feature_info.at(4).id)});

                    //操作状态加时间
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_OPERATIONINFO,{"end_time",currenttime_1,
                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});


                    //特征状态加时间
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_FEATUREINFO,{"end_time",currenttime_1,
                                                                                "id",to_string(myDB.deque_feature_info.at(4).id)});

                }

            }

            //
            myUA.UA_readvalue(machine_upload_cover_param.columns);

            //先写0
            myUA.UA_writevalue(machine_upload_cover_param.columns,"0");

            //写入14放行
            myUA.UA_writevalue(machine_upload_cover_param.columns,"14");


            //重新修改设备状态为ready
            g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                     TABLE_EQUIPMENTINFO,{"status","ready",
                                                                          "guid",myDB.equip_info.guid});

        }
            break;


            /*---下盒机半调试完成  2020.10.25 01:16---*/
        case MACHINE_UNLOAD_BOX:
        {
            MACHINE_UNLOAD_BOX_PARAM machine_unload_box_param;

            printf("开始运行下盒机的逻辑\n");

            //等待托盘到达
            printf("正在等待托盘到来，设备为 %s", myDB.equip_info.actual_name.c_str());

            //托盘到达且托盘上无盒子
            while (g_flag_IsMainRunning && myUA.machine_unload_box_info.value_current_status != "11")
            {
                this_thread::sleep_for(chrono::milliseconds(5));
            }

            //修改设备状态为working
            g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                     TABLE_EQUIPMENTINFO,{"status","working","guid",myDB.equip_info.guid});

            //RFID标志位置1
            myUA.UA_writevalue(machine_unload_box_param.flag_RFID,"1");

            for(int t=0;t<10;t++)
            {
                //检测RFID的值
                if((myUA.machine_unload_box_info.value_RFID_readdata != "0")&&
                        (myUA.machine_unload_box_info.value_RFID_readdata != ""))
                {
                    //读到的话，break出来，执行下面的
                    break;
                }
                this_thread::sleep_for(chrono::milliseconds(500));
            }

            //若还是读不出RFID，则弹出，不过一般不会出现这种情况
            if((myUA.machine_unload_box_info.value_RFID_readdata == "0")&&
                (myUA.machine_unload_box_info.value_RFID_readdata == ""))
            {

                //先改成0，再放行
                myUA.UA_writevalue(machine_unload_box_param.inputcommand,"0");

                //写入14放行
                myUA.UA_writevalue(machine_unload_box_param.inputcommand,"14");

                //RFID标志位置0
                myUA.UA_writevalue(machine_unload_box_param.flag_RFID,"0");

                //修改设备状态为ready
                g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                         TABLE_EQUIPMENTINFO,{"status","ready","guid",myDB.equip_info.guid});

                //跳出这个switch-case
                break;
            }

            //函数，根据读到的托盘guid，得出产品的ip，以及特征deque
            myDB.getProductInfofrommysql(myUA.machine_unload_box_info.value_RFID_readdata);

            //RFID标志位置0，关闭读写RFID
            myUA.UA_writevalue(machine_unload_box_param.flag_RFID,"0");

            //该托盘没有分到到任何产品
            if(myDB.product_row == 0)
            {
                //先改成0，再放行
                myUA.UA_writevalue(machine_unload_box_param.inputcommand,"0");

                //写入14放行
                myUA.UA_writevalue(machine_unload_box_param.inputcommand,"14");

                //RFID标志位置0
                myUA.UA_writevalue(machine_unload_box_param.flag_RFID,"0");

                //修改设备状态为ready
                g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                         TABLE_EQUIPMENTINFO,{"status","ready","guid",myDB.equip_info.guid});

                //跳出这个switch-case
                break;
            }

            //函数，根据产品的guid，得出该产品全部的操作
            myDB.getOperationInfoAllfrommysql(myDB.product_info.guid);


            //记录这个设备操作是第几个操作
            int num_oper = 0;

            for(int t =0; t <= myDB.oper_all_row ; t++)
            {
                if(myDB.deque_oper_all_info.at(t).name == "Operation_UB")
                {
                    num_oper = t;
                    break;
                }
            }

            //然后就看这个操作前面的操作都完成了没
            for(int t =0;t <= num_oper ; t++)
            {
                if((myDB.deque_oper_all_info.at(t).status != "done")&&(t < num_oper))
                {
                    //若前面还有工序没有完成，那就break出for循环，执行放行
                    break;
                }

                if(myDB.deque_oper_all_info.at(t).status == "done")
                {
                    //若这个工序完成了，那么继续检查下一个工序
                    continue;
                }

                //不被break，不被continue才能来到这一步
                if(t == num_oper)
                {

                    string currenttime = GetCurrentSystemTime();

                    //修改操作状态为working
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_OPERATIONINFO,{"status","working",
                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});
                    //操作状态加时间
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_OPERATIONINFO,{"start_time",currenttime,
                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});

                    //修改特征状态为working
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_FEATUREINFO,{"status","working",
                                                                                "id",to_string(myDB.deque_feature_info.at(5).id)});

                    //特征状态加时间
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_FEATUREINFO,{"start_time",currenttime,
                                                                                "id",to_string(myDB.deque_feature_info.at(5).id)});

                    //执行步骤
                    myUA.UA_writevalue(machine_unload_box_param.inputcommand, "1");//写入1--抓取

                    //等到抓取完成
                    while (g_flag_IsMainRunning && myUA.machine_unload_box_info.value_current_status != "12")
                    {
                        this_thread::sleep_for(chrono::milliseconds(500));
                    }

                    string currenttime_1 = GetCurrentSystemTime();

                    //修改操作状态为done
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_OPERATIONINFO,{"status","done",
                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});

                    //操作状态加时间
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_OPERATIONINFO,{"end_time",currenttime_1,
                                                                                  "id",to_string(myDB.deque_oper_all_info.at(t).id)});


                    //修改特征状态为done
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_FEATUREINFO,{"status","done",
                                                                                "id",to_string(myDB.deque_feature_info.at(5).id)});

                    //特征状态加时间
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_FEATUREINFO,{"end_time",currenttime_1,
                                                                                "id",to_string(myDB.deque_feature_info.at(5).id)});



                    //产品状态完成
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_PRODUCTINFO,{"status","done",
                                                                                "guid",myDB.product_info.guid});
                    //产品状态完成加时间
                    g_flag_IsMySQLConnected = myDB.run_query(QUERY_UPDATE,
                                                             TABLE_PRODUCTINFO,{"actual_departure_time",currenttime_1,
                                                                                "guid",myDB.product_info.guid});

                    //先改成0，再放行
                    myUA.UA_writevalue(machine_unload_box_param.inputcommand,"0");

                    //写入14放行
                    myUA.UA_writevalue(machine_unload_box_param.inputcommand,"14");

                }

            }

            //RFID标志位置0
            myUA.UA_writevalue(machine_unload_box_param.flag_RFID,"0");


            //先改成0，再放行
            myUA.UA_writevalue(machine_unload_box_param.inputcommand,"0");

            //写入14放行
            myUA.UA_writevalue(machine_unload_box_param.inputcommand,"14");




            //重新修改设备状态为ready
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
            printf("连不上啊连不上\n");
        }
            break;

        default:
            break;


        }





    }

    /*

    //    while(1)
    //    {

    //        printf("开始\n");

    //        //函数，根据读到的托盘guid，得出产品的ip，已经特征deque

    //        myDB.getProductInfofrommysql("111");

    //        //void getOperationInfofrommysql(string product_guid);
    //        printf("%s\n",myDB.product_info.guid.c_str());

    //        printf("%s\n",myDB.product_info.pallet_ip_port.c_str());


    //        //检测设备UA是否连接并重连

    //        //       UA_connect_product_TCP(product_url);

    //        this_thread::sleep_for(chrono::seconds(2));

    //        //Sleep(2000);
    //    }

    //thrd_connect_mysql.detach();

    //thrd_connect_equipment_ua.detach();

*/




}




