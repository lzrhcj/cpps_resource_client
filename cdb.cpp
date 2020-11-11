
#include "cdb.h"


CDB myDB;


CDB::CDB()
{

}

CDB::~CDB()
{
    g_flag_IsMySQLConnected = false;
    mysql_close(&mySQL);//断开连接
}

bool CDB::init_mysql()
{
    bool ret_init_mysql = false;

    mysql_init(&mySQL);//初始化mysql连接

    if (nullptr!= mysql_real_connect(&mySQL,Database_HostName,Database_UserName,
                                     Database_Password,Database_Name,Database_Port,nullptr,0))
    {
        //连接成功
        printf("数据库连接成功\n");
        ret_init_mysql = true;
    }
    else
    {
        printf("数据库连接失败\n");
        ret_init_mysql = false;
    }

    return ret_init_mysql;
}


//一直检测数据库连接状态，若连接不上，立刻重连
void CDB::connect_mysql()
{
    while(g_flag_IsMainRunning)
    {
        if(g_flag_IsMySQLConnected == false)
        {
            for(int i=1;i<=10;i++)
            {
                if(g_flag_IsMySQLConnected == false)
                {
                    printf("重连数据库第%i次\n",i);
                    m_db.lock();
                    g_flag_IsMySQLConnected = init_mysql();//建立数据库连接
                    m_db.unlock();

                }
                this_thread::sleep_for(chrono::seconds(2));
            }
        }
        else
        {
            g_flag_IsMySQLConnected = true;
        }
        //每两秒检查一次数据库连接状态
        this_thread::sleep_for(chrono::seconds(2));
    }
}


void CDB::exit_mysql()
{
    g_flag_IsMySQLConnected = false;

    mysql_close(&mySQL);//断开连接
}

void CDB::getEquipmentInfofrommysql()
{

    MYSQL_RES *result = nullptr;
    MYSQL_ROW sql_row = NULL;

    m_db.lock();
    string query_equipment= string("SELECT * FROM cpps_db.equipments WHERE ip_port = 'opc.tcp://")+
            g_opcuaIP+":"+g_opcuaPort+"';";

    //两秒更新一下设备的信息
    this_thread::sleep_for(chrono::seconds(2));

    if (!mysql_query(&mySQL, query_equipment.c_str()))
    {
        result = mysql_store_result(&mySQL);//保存查询到的数据到result
        if (result)
        {
            int totalrows = (int)mysql_num_rows(result);
            for(int i = 0; i < totalrows; i++)
            {
                sql_row = mysql_fetch_row(result);

                memset(&equip_info, 0, sizeof(equip_info));

                if(sql_row[0]!= NULL)
                    equip_info.guid = sql_row[0];
                if(sql_row[1]!= NULL)
                    equip_info.type = sql_row[1];
                if(sql_row[2]!= NULL)
                    equip_info.type_subindex = atoi(sql_row[2]);
                if(sql_row[3]!= NULL)
                    equip_info.ip_port = sql_row[3];
                if(sql_row[4]!= NULL)
                    equip_info.actual_name = sql_row[4];
                if(sql_row[5]!= NULL)
                    equip_info.status = sql_row[5];
            }
        }
        g_flag_IsMySQLConnected =  true;
    }
    else
    {
        g_flag_IsMySQLConnected =  false;
    }


    mysql_free_result(result);//释放结果资源
    result = nullptr;

    if (result != nullptr)
    {
        mysql_free_result(result);//释放结果资源

        if (result != nullptr)
        {
            result = nullptr;
        }
    }

    m_db.unlock();

}


void CDB::getProductInfofrommysql(string pallet_guid)
{
    MYSQL_RES *result = nullptr;
    MYSQL_ROW sql_row = NULL;

    m_db.lock();

    //SELECT * FROM cpps_db.products WHERE pallet_guid = '111';

    string query_product_guid= string("SELECT * FROM cpps_db.products WHERE pallet_guid = '")+
            pallet_guid + "' AND (status = 'ready' OR status = 'working');";

    if (!mysql_query(&mySQL, query_product_guid.c_str()))
    {
        result = mysql_store_result(&mySQL);//保存查询到的数据到result

        if (result)
        {
            int totalrows = (int)mysql_num_rows(result);

            product_row = totalrows;

            for(int i = 0; i < totalrows; i++)
            {
                sql_row = mysql_fetch_row(result);

                memset(&product_info, 0, sizeof(product_info));

                //不知道为什么这里就算赋值""会报错，只能先这么个处理先
                if(sql_row[0]!= NULL)
                    product_info.guid = sql_row[0];
                if(sql_row[1]!= NULL)
                    product_info.order_id = atoi(sql_row[1]);
                if(sql_row[2]!= NULL)
                    product_info.name = sql_row[2];
                if(sql_row[3]!= NULL)
                    product_info.status = sql_row[3];
//                if(sql_row[4]!= NULL)
//                    product_info.actual_arrival_time = sql_row[4];
//                if(sql_row[5]!= NULL)
//                    product_info.actual_departure_time = sql_row[5];
//                if(sql_row[6]!= NULL)
//                    product_info.operating_time = sql_row[6];
//                if(sql_row[7]!= NULL)
//                    product_info.current_position = sql_row[7];
                if(sql_row[8]!= NULL)
                    product_info.pallet_guid = sql_row[8];
                if(sql_row[9]!= NULL)
                    product_info.pallet_ip_port = sql_row[9];
            }
        }
        g_flag_IsMySQLConnected =  true;
    }
    else
    {
        g_flag_IsMySQLConnected =  false;
    }

    mysql_free_result(result);//释放结果资源
    result = nullptr;

    if (result != nullptr)
    {
        mysql_free_result(result);//释放结果资源

        if (result != nullptr)
        {
            result = nullptr;
        }
    }

    //获取完了产品的guid，然后获取特征

    string query_feature = string("SELECT * FROM cpps_db.features WHERE product_guid = '") +
            product_info.guid + "' ORDER BY 'index' ASC;";

    if (!mysql_query(&mySQL, query_feature.c_str()))
    {
        result = mysql_store_result(&mySQL);//保存查询到的数据到result
        if (result)
        {
            int totalrows = (int)mysql_num_rows(result);

            feature_row = totalrows;

            deque_feature_info.clear();

            for(int i = 0; i < feature_row; i++)
            {
                sql_row = mysql_fetch_row(result);

                SFeatureInfo feature_info;
                if(sql_row[0]!= NULL)
                    feature_info.id = atoi(sql_row[0]);
                if(sql_row[1]!= NULL)
                    feature_info.product_guid = sql_row[1];
                if(sql_row[2]!= NULL)
                    feature_info.index = atoi(sql_row[2]);
                if(sql_row[3]!= NULL)
                    feature_info.description = sql_row[3];
                if(sql_row[4]!= NULL)
                    feature_info.parameter = sql_row[4];
                if(sql_row[5]!= NULL)
                    feature_info.status = sql_row[5];
//                if(sql_row[6]!= NULL)
//                    feature_info.start_time = sql_row[6];
//                if(sql_row[7]!= NULL)
//                    feature_info.end_time = sql_row[7];

                deque_feature_info.push_back(feature_info);
            }

        }
        g_flag_IsMySQLConnected = true;
    }
    else
    {
        g_flag_IsMySQLConnected = false;
    }

    mysql_free_result(result);//释放结果资源
    result = nullptr;

    if (result != nullptr)
    {
        mysql_free_result(result);//释放结果资源

        if (result != nullptr)
        {
            result = nullptr;
        }
    }

    m_db.unlock();

}

void CDB::getFeatureInfofrommysql(string product_guid)
{
    MYSQL_RES *result = nullptr;
    MYSQL_ROW sql_row = NULL;

    m_db.lock();

    string query_feature = string("SELECT * FROM cpps_db.features WHERE product_guid = '") +
            product_guid + "' ORDER BY 'index' ASC;";

    if (!mysql_query(&mySQL, query_feature.c_str()))
    {
        result = mysql_store_result(&mySQL);//保存查询到的数据到result
        if (result)
        {
            int totalrows = (int)mysql_num_rows(result);

            feature_row = totalrows;

            deque_feature_info.clear();

            for(int i = 0; i < feature_row; i++)
            {
                sql_row = mysql_fetch_row(result);

                SFeatureInfo feature_info;
                if(sql_row[0]!= NULL)
                    feature_info.id = atoi(sql_row[0]);
                if(sql_row[1]!= NULL)
                    feature_info.product_guid = sql_row[1];
                if(sql_row[2]!= NULL)
                    feature_info.index = atoi(sql_row[2]);
                if(sql_row[3]!= NULL)
                    feature_info.description = sql_row[3];
                if(sql_row[4]!= NULL)
                    feature_info.parameter = sql_row[4];
                if(sql_row[5]!= NULL)
                    feature_info.status = sql_row[5];
//                if(sql_row[6]!= NULL)
//                    feature_info.start_time = sql_row[6];
//                if(sql_row[7]!= NULL)
//                    feature_info.end_time = sql_row[7];

                deque_feature_info.push_back(feature_info);
            }

        }
        g_flag_IsMySQLConnected = true;
    }
    else
    {
        g_flag_IsMySQLConnected = false;
    }

    mysql_free_result(result);//释放结果资源
    result = nullptr;

    if (result != nullptr)
    {
        mysql_free_result(result);//释放结果资源

        if (result != nullptr)
        {
            result = nullptr;
        }
    }

    m_db.unlock();



}

void CDB::getOperationInfofrommysql(string product_guid)
{
    MYSQL_RES *result = nullptr;
    MYSQL_ROW sql_row = NULL;

    m_db.lock();

    string query_operation= string("SELECT * FROM cpps_db.operations WHERE equipment_guid = '") +
            equip_info.guid + "' AND product_guid = '"+product_guid+"' ORDER BY id ASC;";

    if (!mysql_query(&mySQL, query_operation.c_str()))
    {
        result = mysql_store_result(&mySQL);//保存查询到的数据到result

        if (result)
        {
            int totalrows = (int)mysql_num_rows(result);

            oper_row = totalrows;

            deque_oper_info.clear();

            for(int i = 0; i < totalrows; i++)
            {
                sql_row = mysql_fetch_row(result);

                SOperationInfo oper_info;

                if(sql_row[0]!= NULL)
                    oper_info.id = atoi(sql_row[0]);
                if(sql_row[1]!= NULL)
                    oper_info.equipment_guid = sql_row[1];
                if(sql_row[2]!= NULL)
                    oper_info.product_guid = sql_row[2];
                if(sql_row[3]!= NULL)
                    oper_info.name = sql_row[3];
                if(sql_row[4]!= NULL)
                    oper_info.param = sql_row[4];
                if(sql_row[5]!= NULL)
                    oper_info.status = sql_row[5];
//                if(sql_row[6]!= NULL)
//                    oper_info.start_time = sql_row[6];
//                if(sql_row[7]!= NULL)
//                    oper_info.end_time = sql_row[7];

                deque_oper_info.push_back(oper_info);

            }
        }
        g_flag_IsMySQLConnected =  true;
    }
    else
    {
        g_flag_IsMySQLConnected =  false;
    }

    mysql_free_result(result);//释放结果资源
    result = nullptr;

    if (result != nullptr)
    {
        mysql_free_result(result);//释放结果资源

        if (result != nullptr)
        {
            result = nullptr;
        }
    }

    m_db.unlock();

}


void CDB::getOperationInfoAllfrommysql(string product_guid)
{
    MYSQL_RES *result = nullptr;
    MYSQL_ROW sql_row = NULL;

    m_db.lock();

    string query_operation_all= string("SELECT * FROM cpps_db.operations WHERE product_guid = '") +
            product_guid +"';";

    if (!mysql_query(&mySQL, query_operation_all.c_str()))
    {
        result = mysql_store_result(&mySQL);//保存查询到的数据到result
        if (result)
        {
            int totalrows = (int)mysql_num_rows(result);

            oper_all_row = totalrows;

            deque_oper_all_info.clear();

            for(int i = 0; i < oper_all_row; i++)
            {
                sql_row = mysql_fetch_row(result);

                SOperationInfo oper_all_info;

                if(sql_row[0]!= NULL)
                    oper_all_info.id = atoi(sql_row[0]);
                if(sql_row[1]!= NULL)
                    oper_all_info.equipment_guid = sql_row[1];
                if(sql_row[2]!= NULL)
                    oper_all_info.product_guid = sql_row[2];
                if(sql_row[3]!= NULL)
                    oper_all_info.name = sql_row[3];
                if(sql_row[4]!= NULL)
                    oper_all_info.param = sql_row[4];
                if(sql_row[5]!= NULL)
                    oper_all_info.status = sql_row[5];

                deque_oper_all_info.push_back(oper_all_info);
            }

        }
        g_flag_IsMySQLConnected = true;
    }
    else
    {
        g_flag_IsMySQLConnected =  false;
    }

    mysql_free_result(result);//释放结果资源
    result = nullptr;

    if (result != nullptr)
    {
        mysql_free_result(result);//释放结果资源

        if (result != nullptr)
        {
            result = nullptr;
        }
    }

    m_db.unlock();

}

void CDB::getCNCInfofrommysql()
{
    MYSQL_RES *result = nullptr;
    MYSQL_ROW sql_row = NULL;

    m_db.lock();

    string query_cnc= string("SELECT * FROM cpps_db.operations WHERE name= 'Operation_pick2CNC' or name= 'Operation_machine' or name= 'Operation_pick2pallet' order by id ASC;");

    if (!mysql_query(&mySQL, query_cnc.c_str()))
    {
        result = mysql_store_result(&mySQL);//保存查询到的数据到result
        if (result)
        {
            int totalrows = (int)mysql_num_rows(result);

            cnc_row = totalrows;

            deque_cnc_info.clear();

            for(int i = 0; i < cnc_row; i++)
            {
                sql_row = mysql_fetch_row(result);

                SOperationInfo oper_cnc_info;

                if(sql_row[0]!= NULL)
                    oper_cnc_info.id = atoi(sql_row[0]);
                if(sql_row[1]!= NULL)
                    oper_cnc_info.equipment_guid = sql_row[1];
                if(sql_row[2]!= NULL)
                    oper_cnc_info.product_guid = sql_row[2];
                if(sql_row[3]!= NULL)
                    oper_cnc_info.name = sql_row[3];
                if(sql_row[4]!= NULL)
                    oper_cnc_info.param = sql_row[4];
                if(sql_row[5]!= NULL)
                    oper_cnc_info.status = sql_row[5];

                deque_cnc_info.push_back(oper_cnc_info);
            }

        }
        g_flag_IsMySQLConnected = true;
    }
    else
    {
        g_flag_IsMySQLConnected =  false;
    }

    mysql_free_result(result);//释放结果资源
    result = nullptr;

    if (result != nullptr)
    {
        mysql_free_result(result);//释放结果资源

        if (result != nullptr)
        {
            result = nullptr;
        }
    }

    m_db.unlock();
}




string CDB::get_TYPE_EQUIPMENT_Name(TYPE_EQUIPMENT type)
{
    char* names[] =
    {
        (char *)"CNC",
        (char *)"ROBOT",
        (char *)"MACHINE_UPLOAD_BOX",
        (char *)"MACHINE_UNLOAD_BOX",
        (char *)"MACHINE_UPLOAD_COVER",
        (char *)"LASER",
        (char *)"CONVEYOR",
        (char *)"AGV"
    };

    int idx = static_cast<int>(type);
    return names[idx];
}


TYPE_EQUIPMENT CDB::get_TYPE_EQUIPMENT_Enum(string type)
{
    TYPE_EQUIPMENT name;
    if (!strcmp(type.c_str(), "CNC"))
    {
        name = CNC;
    }
    else if (!strcmp(type.c_str(), "ROBOT"))
    {
        name = ROBOT;
    }
    else if (!strcmp(type.c_str(), "MACHINE_UPLOAD_BOX"))
    {
        name = MACHINE_UPLOAD_BOX;
    }
    else if (!strcmp(type.c_str(), "MACHINE_UNLOAD_BOX"))
    {
        name = MACHINE_UNLOAD_BOX;
    }
    else if (!strcmp(type.c_str(), "MACHINE_UPLOAD_COVER"))
    {
        name = MACHINE_UPLOAD_COVER;
    }
    else if (!strcmp(type.c_str(), "LASER"))
    {
        name = LASER;
    }
    else if (!strcmp(type.c_str(), "CONVEYOR"))
    {
        name = CONVEYOR;
    }
    else if (!strcmp(type.c_str(), "AGV"))
    {
        name = AGV;
    }
    else
    {
        name = NOPE;
    }

    return name;
}

/*由这个函数统一生成query语句
 * QUERYTYPE querytype 对数据库操作类型，插入、更新、删除
 * TABLENAME tablename 操作数据库表的名字，table_equipmentinfo、table_orderinfo、table_operationinfo
 * initializer_list<string>param_list 一系列可变参数数组
 * param_list 对INSERT输入的格式为，{列1名字，列2名字...列n名字，列1的值，列2的值...列n的值}
 * param_list 对UPDATE输入的格式为，{列1名字，列1的值，列2名字，列2的值，列n名字...列n的值，主键名，主键的值}
 * param_list 对DELETE输入的格式为，{该行序号}
 * 使用范例：
 * string query1;
 * string query2;
 * string query3;
 * query2 = set_query(QUERY_UPDATE, TABLE_EQUIPMENTINFO,{ "equipment_id","2","IP_port","192.168.1.103","equipment_type","plc","id","2"});
 *
 */
bool CDB::run_query(QUERYTYPE querytype, TABLENAME tablename, initializer_list<string> param_list)
{
    bool ret_run_query = false;

    string query_part1;
    string query_part2;
    string query_part3;
    string query;

    int size_param_list;
    size_param_list = param_list.size();

    switch (querytype)
    {
    case QUERY_INSERT:
    {
        query_part1 = "INSERT INTO `cpps_db`.`";

        switch (tablename)
        {
        case TABLE_EQUIPMENTINFO:
            query_part2 = "equipments` (`";
            break;
        case TABLE_ORDERINFO:
            query_part2 = "orders` (`";
            break;
        case TABLE_OPERATIONINFO:
            query_part2 = "operations` (`";
            break;
        case TABLE_PRODUCTINFO:
            query_part2 = "products` (`";
            break;
        case TABLE_FEATUREINFO:
            query_part2 = "features` (`";
            break;
        }

        for (int i = 1; i <= size_param_list / 2; i++)
        {
            initializer_list<string>::iterator param_list_iterator = param_list.begin();
            advance(param_list_iterator, i - 1);
            query_part3 += *param_list_iterator;
            if (i != size_param_list / 2)
            {
                query_part3 += "`, `";
            }
        }

        query_part3 += "`) VALUES ('";

        for (int i = (size_param_list / 2) + 1; i <= size_param_list; i++)
        {
            initializer_list<string>::iterator param_list_iterator = param_list.begin();
            advance(param_list_iterator, i - 1);
            query_part3 += *param_list_iterator;
            if (i != size_param_list)
            {
                query_part3 += "', '";
            }
        }
    }
        break;

    case QUERY_UPDATE:
    {
        query_part1 = "UPDATE `cpps_db`.`";
        switch (tablename)
        {
        case TABLE_EQUIPMENTINFO:
            query_part2 = "equipments` SET `";
            break;
        case TABLE_ORDERINFO:
            query_part2 = "orders` SET `";
            break;
        case TABLE_OPERATIONINFO:
            query_part2 = "operations` SET `";
            break;
        case TABLE_PRODUCTINFO:
            query_part2 = "products` SET `";
            break;
        case TABLE_FEATUREINFO:
            query_part2 = "features` SET `";
            break;

        }

        for (int i = 1; i <= (size_param_list - 2); i++)
        {
            initializer_list<string>::iterator param_list_iterator = param_list.begin();
            advance(param_list_iterator, i - 1);
            query_part3 += *param_list_iterator;

            if (i % 2 == 0)
            {
                //i是偶数时
                if (i != (size_param_list - 2))
                {
                    query_part3 += "', `";
                }
            }
            else
            {
                //i是奇数时
                query_part3 += "` = '";
            }
        }

        query_part3 += "' WHERE (`";

        //WHERE (`equipment_id` = '1453');"

        for (int i = (size_param_list - 1); i <= size_param_list; i++)
        {
            initializer_list<string>::iterator param_list_iterator = param_list.begin();
            advance(param_list_iterator, i - 1);
            query_part3 += *param_list_iterator;

            if(i == (size_param_list - 1))
            {
                query_part3 += "` = '";
            }

        }

    }
        break;

    case QUERY_DELETE:
    {
        query_part1 = "DELETE FROM `cpps_db`.`";

        switch (tablename)
        {
        case TABLE_EQUIPMENTINFO:
            query_part2 = "equipments` WHERE (`";
            break;
        case TABLE_ORDERINFO:
            query_part2 = "orders` WHERE (`";
            break;
        case TABLE_OPERATIONINFO:
            query_part2 = "operations` WHERE (`";
            break;
        case TABLE_PRODUCTINFO:
            query_part2 = "products` WHERE (`";
            break;
        case TABLE_FEATUREINFO:
            query_part2 = "features` WHERE (`";
            break;
        }

        query_part3 += *param_list.begin();
        query_part3 += "` = '";
        query_part3 += *param_list.end();

    }
        break;

    }

    query_part3 += "');";

    query = query_part1 + query_part2 + query_part3;

    m_db.lock();
    if (!mysql_query(&mySQL, query.c_str()))
    {
        ret_run_query = true;
    }
    else
    {
        ret_run_query =  false;
    }
    m_db.unlock();

    return ret_run_query;

}


