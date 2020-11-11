#ifndef CDB_H
#define CDB_H

#include <deque>

#include "mysql.h"


#include <string>

#include <thread>
#include <mutex>

#include <chrono>

#include <signal.h>
#include <stdlib.h>

#include "global_flag.h"


//#define Database_Name "cpps_db"
//#define Database_HostName "www.itdili.ml"
//#define Database_Port 30928
//#define Database_UserName "cpps"
//#define Database_Password "cpps"

#define Database_Name "cpps_db"
#define Database_HostName "192.168.137.122"
#define Database_Port 30928
#define Database_UserName "cpps"
#define Database_Password "cpps"

struct SEquipmentInfo
{
    string guid;
    string type;
    int type_subindex;
    string ip_port;
    string actual_name;
    string status;
};

struct SOperationInfo
{
    int id;
    string equipment_guid;
    string product_guid;
    string name;
    string param;
    string status;
    string start_time;
    string end_time;
};

struct SProductInfo
{
    string guid;
    int order_id;
    string name;
    string status;
    string actual_arrival_time;
    string actual_departure_time;
    string operating_time;
    string current_position;
    string pallet_guid;
    string pallet_ip_port;

};

struct SFeatureInfo
{
    int id;
    string product_guid;
    int index;
    string description;
    string parameter;
    string status;
    string start_time;
    string end_time;
};


enum TYPE_EQUIPMENT
{
    MACHINE_UPLOAD_BOX,
    CONVEYOR,
    MACHINE_UNLOAD_BOX,
    ROBOT,
    CNC,
    LASER,
    MACHINE_UPLOAD_COVER,
    AGV,
    NOPE,
};


enum QUERYTYPE
{
    QUERY_INSERT,
    QUERY_UPDATE,
    QUERY_DELETE,
};

enum TABLENAME
{
    TABLE_EQUIPMENTINFO,
    TABLE_ORDERINFO,
    TABLE_OPERATIONINFO,
    TABLE_PRODUCTINFO,
    TABLE_FEATUREINFO,

};


class CDB
{
public:
    CDB();	// 标准构造函数
    ~CDB();	// 标准构造函数

public:
    MYSQL mySQL;

    SEquipmentInfo equip_info;

    SProductInfo product_info;

    int product_row;

    deque<SFeatureInfo> deque_feature_info;

    int feature_row;

    deque<SOperationInfo> deque_oper_info;

    int oper_row;

    deque<SOperationInfo> deque_oper_all_info;

    int oper_all_row;

    deque<SOperationInfo> deque_cnc_info;

    int cnc_row;

    mutex m_db;

public:

    bool init_mysql();

    void connect_mysql();

    void exit_mysql();

    //  char actual_name[100];

    //线程函数，用void
    void getEquipmentInfofrommysql();

    //函数，根据读到的托盘guid，得出产品的ip，以及特征deque
    void getProductInfofrommysql(string pallet_guid);

    void getFeatureInfofrommysql(string product_info);

    //函数，根据产品的guid，得出该产品该设备的操作
    void getOperationInfofrommysql(string product_guid);

    void getOperationInfoAllfrommysql(string product_guid);

    void getCNCInfofrommysql();


    static string get_TYPE_EQUIPMENT_Name(TYPE_EQUIPMENT type);//枚举转字符串
    static TYPE_EQUIPMENT get_TYPE_EQUIPMENT_Enum(string type);//字符串转枚举

    bool run_query(QUERYTYPE querytype, TABLENAME tablename, initializer_list<string> param_list);


};


extern CDB myDB;



#endif // CDB_H
