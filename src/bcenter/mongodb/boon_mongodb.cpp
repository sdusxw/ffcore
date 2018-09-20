/***************************boon_mongodb.cpp***************************************
              功能：mongodb数据库增删改查
              创建时间：2017-02-04
              创建人：孙振行
              单位：山东博昂信息科技有限公司
              修改时间：
***************************************************************************/
#include "rapidjson_lib.h"
#include "boon_log.h"
#include "common_def.h"
#include "../bcenter_def.h"

#include "boon_mongodb.h"
#include <iostream>
#include <string>

using namespace BASE;

/******************************mongodb连接信息************************/
mongoc_uri_t *mongodb_uri;
mongoc_client_pool_t *mongodb_pool;
mongoc_client_t *mongodb_client;
/*****************************end************************************/
/*mongoc_collection_t *mongodb_table_park;	//park表
mongoc_collection_t *mongodb_table_channel;	//channel表
mongoc_collection_t *mongodb_table_device;	//device表
mongoc_collection_t *mongodb_table_car;	//car表
mongoc_collection_t *mongodb_table_carinpark;	//carinpark表
mongoc_collection_t *mongodb_table_plateauth;	//plateauth表
mongoc_collection_t *mongodb_table_typeauth;	//typeauth表
mongoc_collection_t *mongodb_table_chargerule;	//typeauth表
mongoc_collection_t *mongodb_table_carinrec;	//carinrec表
mongoc_collection_t *mongodb_table_ledset;	//ledset表
mongoc_collection_t *mongodb_table_caroutrec;	//caroutrec表
mongoc_collection_t *mongodb_table_people;	//people表
mongoc_collection_t *mongodb_table_tmpcartype;	//tmpcartype表
mongoc_collection_t *mongodb_table_zxcharge;	//中心缴费表
mongoc_collection_t *mongodb_table_wxcharge;	//微信缴费表*/

extern char  host_server_ip[64];//服务器ip
extern char out_plate_last[PLATE_LENGTH];
extern bool in_guoqi_flag; //入口车过期标志
extern bool out_guoqi_flag; //出口车过期标志
extern char in_plate[24]; //最近的车牌号
extern char in_channel[256];//最近的通道id号
extern char in_time[24];//最近的进入时间
extern char out_plate[24]; //最近的车牌号
extern char out_channel[256];//最近的通道id号
extern char out_time[24];//最近的进入时间
extern char out_pcolor[24]; // 出口车牌颜色
extern char in_out_flag[24]; // 入口或出口标记
extern bool in_fleet; //入口车队模式标志
extern bool out_fleet; //出口车队模式标志
extern int fmoney; //收费金额
extern char in_led_ip[24];
extern char  out_led_ip[24];
extern bool in_led_guoqi_flag;
extern bool out_led_guoqi_flag;
extern bool in_led_noauth_flag;
extern pthread_mutex_t mongo_mutex_channel;
extern pthread_mutex_t mongo_mutex_carinpark;
extern pthread_mutex_t mongo_mutex_car;
extern pthread_mutex_t mongo_mutex_device;
char str_con[128];
char remain_car_time[128];
char number[10][10] = {
    {'@','@','@','@','@','@','@','@','@','@'}, //0
    {'T','7','V','L','Y','E','U','@','@','@'}, //1
    {'Z','9','@','@','@','@','@','@','@','@'}, //2
    {'@','@','@','@','@','@','@','@','@','@'}, //3
    {'A','@','@','@','@','@','@','@','@','@'}, //4
    {'3','S','@','@','@','@','@','@','@','@'}, //5
    {'@','@','@','@','@','@','@','@','@','@'}, //6
    {'Z','1','T','@','@','@','@','@','@','@'}, //7
    {'B','@','@','@','@','@','@','@','@','@'}, //8
    {'@','@','@','@','@','@','@','@','@','@'}, //9
};

char alpha[26][10] = {
    {'@','@','@','@','@','@','@','@','@','@'},	//A
    {'8','@','@','@','@','@','@','@','@','@'},     //B
    {'8','0','@','@','@','@','@','@','@','@'},     //C
    {'@','@','@','@','@','@','@','@','@','@'},     //D
    {'F','@','@','@','@','@','@','@','@','@'},     //E
    {'E','@','@','@','@','@','@','@','@','@'},     //F
    {'8','@','@','@','@','@','@','@','@','@'},     //G
    {'@','@','@','@','@','@','@','@','@','@'},	//H
    {'@','@','@','@','@','@','@','@','@','@'},	//I
    {'U','A','1','@','@','@','@','@','@','@'},	//J
    {'@','@','@','@','@','@','@','@','@','@'},	//K
    {'Z','G','0','A','Z','T','@','@','@','@'},	//L
    {'@','@','@','@','@','@','@','@','@','@'},	//M
    {'@','@','@','@','@','@','@','@','@','@'},	//N
    {'@','@','@','@','@','@','@','@','@','@'},	//O
    {'@','@','@','@','@','@','@','@','@','@'},	//P
    {'0','A','L','@','@','@','@','@','@','@'},	//Q
    {'@','@','@','@','@','@','@','@','@','@'},	//R
    {'A','3','@','@','@','@','@','@','@','@'},	//S
    {'7','1','@','@','@','@','@','@','@','@'},	//T
    {'0','@','@','@','@','@','@','@','@','@'},	//U
    {'M','@','@','@','@','@','@','@','@','@'},	//V
    {'@','@','@','@','@','@','@','@','@','@'},	//W
    {'@','@','@','@','@','@','@','@','@','@'},	//X
    {'M','@','@','@','@','@','@','@','@','@'},	//Y
    {'3','2','@','@','@','@','@','@','@','@'}	//Z
};

// 日志缓冲区
static char log_buf_[LOG_BUF_SIZE] = {0};
// 写日志
extern void writelog(const char* _buf);
// 是否加入万能语音功能， 不在此赋值, 在 main.cpp中定义
extern bool g_has_wanwangyuyin_;
// 是否加入车队模式，不在此赋值, 在 main.cpp中定义
extern bool g_has_cheduimoshi_;

void my_logger (mongoc_log_level_t log_level,
                const char *log_domain,
                const char *message,
                void *user_data)
{

    if (log_level < MONGOC_LOG_LEVEL_INFO) {
        mongoc_log_default_handler (log_level, log_domain, message, user_data);
    }
}


/**************************************************mongodb数据库连接******************************/
int mongodb_connect()
{
    char str[128] = {0};
    sprintf(str,"nc -z  -w 1 %s 27017",host_server_ip);
    if(system(str) != 0)
    {
        return -1;
    }

    // mongodb://127.0.0.1:27017
    //sprintf(str_con,"mongodb://boon:boon123456@%s:27017/?authSource=boondb&socketTimeoutMS=30&connectTimeoutMS=30",host_server_ip); //数据库连接地址
    sprintf(str_con,"mongodb://boon:boon123456@%s:27017/?authSource=boondb",host_server_ip); //数据库连接地址

    //mongodb_uri = mongoc_uri_new(str);
    //mongodb_pool = mongoc_client_pool_new(mongodb_uri);
    mongodb_client = mongoc_client_new(str_con);

  /*  mongodb_table_park = mongoc_client_get_collection(mongodb_client,"boondb","park");   //park表
    mongodb_table_channel = mongoc_client_get_collection(mongodb_client,"boondb","channel");   //channel表
    mongodb_table_device = mongoc_client_get_collection(mongodb_client,"boondb","device");   //device表
    mongodb_table_car = mongoc_client_get_collection(mongodb_client,"boondb","car");   //car表
    mongodb_table_carinpark = mongoc_client_get_collection(mongodb_client,"boondb","carinpark");   //carinpark表
    mongodb_table_plateauth = mongoc_client_get_collection(mongodb_client,"boondb","plateauth");   //plateauth表
    mongodb_table_typeauth = mongoc_client_get_collection(mongodb_client,"boondb","typeauth");   //typeauth表
    mongodb_table_chargerule = mongoc_client_get_collection(mongodb_client,"boondb","chargerule");   //chargerule表
    mongodb_table_carinrec = mongoc_client_get_collection(mongodb_client,"boondb","carinrec");   //carinrec表
    mongodb_table_ledset = mongoc_client_get_collection(mongodb_client,"boondb","ledset");   //ledset表
    mongodb_table_caroutrec = mongoc_client_get_collection(mongodb_client,"boondb","caroutrec");   //caroutrec表
    mongodb_table_people = mongoc_client_get_collection(mongodb_client,"boondb","people");   //people表
    mongodb_table_tmpcartype = mongoc_client_get_collection(mongodb_client,"boondb","tmpcartype");   //tmpcartype表
    mongodb_table_zxcharge = mongoc_client_get_collection(mongodb_client,"boondb","zxcharge");   //中心缴费表
    mongodb_table_wxcharge = mongoc_client_get_collection(mongodb_client,"boondb","wxcharge");   //微信缴费表*/

    return 0;
}


int mongodb_exit()
{


    /*if(mongodb_table_park != NULL)
    {
        mongoc_collection_destroy(mongodb_table_park);
    }
    if(mongodb_table_channel != NULL)
    {
        mongoc_collection_destroy(mongodb_table_channel);
    }
    if(mongodb_table_device != NULL)
    {
        mongoc_collection_destroy(mongodb_table_device);
    }
    if(mongodb_table_car != NULL)
    {
        mongoc_collection_destroy(mongodb_table_car);
    }
    if(mongodb_table_carinpark != NULL)
    {
        mongoc_collection_destroy(mongodb_table_carinpark);
    }
    if(mongodb_table_plateauth != NULL)
    {
        mongoc_collection_destroy(mongodb_table_plateauth);
    }
    if(mongodb_table_typeauth != NULL)
    {
        mongoc_collection_destroy(mongodb_table_typeauth);
    }
    if(mongodb_table_chargerule != NULL)
    {
        mongoc_collection_destroy(mongodb_table_chargerule);
    }
    if(mongodb_table_carinrec != NULL)
    {
        mongoc_collection_destroy(mongodb_table_carinrec);
    }
    if(mongodb_table_ledset != NULL)
    {
        mongoc_collection_destroy(mongodb_table_ledset);
    }
    if(mongodb_table_caroutrec != NULL)
    {
        mongoc_collection_destroy(mongodb_table_caroutrec);
    }
    if(mongodb_table_people != NULL)
    {
        mongoc_collection_destroy(mongodb_table_people);
    }
    if(mongodb_table_tmpcartype != NULL)
    {
        mongoc_collection_destroy(mongodb_table_tmpcartype);
    }
    if(mongodb_table_zxcharge != NULL)
    {
        mongoc_collection_destroy(mongodb_table_zxcharge);
    }
    if(mongodb_table_wxcharge != NULL)
    {
        mongoc_collection_destroy(mongodb_table_wxcharge);
    }
    if(mongodb_client != NULL)
    {
        mongoc_client_destroy(mongodb_client);
    }
    //mongoc_client_pool_destroy (mongodb_pool);
    //mongoc_uri_destroy (mongodb_uri);*/
    if(mongodb_client != NULL)
    {
        mongoc_client_destroy(mongodb_client);
    }

}
/**************************************************end******************************************/
/******************************给bipc发送ipc_config信息*******************************************/
int mongodb_get_ipc_config(int _dst_port)
{
   // mongoc_client_t *mongodb_client;
    mongoc_collection_t *mongodb_table_channel;	//channel表
    mongoc_collection_t *mongodb_table_device;	//device表
  //  mongodb_client = mongoc_client_new(str_con);
    mongodb_table_channel = mongoc_client_get_collection(mongodb_client,"boondb","channel");   //channel表
    mongodb_table_device = mongoc_client_get_collection(mongodb_client,"boondb","device");   //device表

    mongoc_cursor_t *cursor_channel;
    mongoc_cursor_t *cursor_device;
    bson_t *query;
    bson_t result;

    bson_iter_t iter;
    const char *tmp;
    const bson_t *tmp1;
    bson_error_t error;
    unsigned int length;

    char channel_id[256];
    char channel_in_out[16];
    char channel_one_way[16];
    char device_type[32];
    char device_ip_id[32];
    char device_username[32];
    char device_password[32];

    Json::Value json_ipc_config; //发送给bipc的json格式的信息

    query = bson_new();

    BSON_APPEND_UTF8 (query, "channel_ip", host_ip); //查询条件

    cursor_channel = mongoc_collection_find (mongodb_table_channel, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查channel表
    if(mongoc_cursor_error(cursor_channel,&error))
    {
        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 查询数据库失败 line[%d]", __FUNCTION__, __LINE__);
        writelog(log_buf_);
        return -1;
    }

    while(!mongoc_cursor_error(cursor_channel,&error)&&mongoc_cursor_more(cursor_channel)) //得到小主机下属的每一个通道信息
    {
        Json::Value json_channel; //通道json格式的信息
        if (mongoc_cursor_next (cursor_channel, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "channel_id")) //得到channel_id
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(channel_id,0,256);
                memcpy(channel_id,tmp,length);
            }
            json_channel["channel_id"] = Json::Value(channel_id);  //json赋值 channel_id
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "channel_in_out")) //得到channel_in_out
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(channel_in_out,0,16);
                memcpy(channel_in_out,tmp,length);
            }
            json_channel["in_out"] = Json::Value(channel_in_out);//json赋值 channel_in_out

            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "channel_one_way")) //得到channel_in_out
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(channel_one_way,0,16);
                memcpy(channel_one_way,tmp,length);
            }
            json_channel["one_way"] = Json::Value(channel_one_way);//json赋值 channel_in_out

            if(strcmp(channel_in_out,"入口") == 0)
            {
                memset(in_channel,0,256);
                memcpy(in_channel,channel_id,strlen(channel_id));
            }
            else
            {
                memset(out_channel,0,256);
                memcpy(out_channel,channel_id,strlen(channel_id));
            }
            bson_t *query_device;
            query_device = bson_new();
            BSON_APPEND_UTF8 (query_device, "device_channel_id", channel_id);  //查询条件

            cursor_device = mongoc_collection_find (mongodb_table_device, MONGOC_QUERY_NONE, 0, 0, 0, query_device, NULL, NULL); //根据通道id号查device表
            while(!mongoc_cursor_error(cursor_device,&error)&&mongoc_cursor_more(cursor_device)) //遍历该通道下每一条设备记录
            {
                if (mongoc_cursor_next (cursor_device, &tmp1))
                {
                    bson_copy_to(tmp1,&result);
                }
                else
                {
                    break;
                }
                if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "device_type")) //得到设备类型
                {
                    tmp = bson_iter_utf8(&iter,&length);
                    memset(device_type,0,32);
                    memcpy(device_type,tmp,length);
                }
                if(strcmp(device_type,"中维抓拍相机") == 0  || strcmp(device_type,"中维智能相机") == 0|| strcmp(device_type,"臻识智能相机") == 0)
                {
                    if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "device_ip_id")) //得到device_ip_id
                    {
                        tmp = bson_iter_utf8(&iter,&length);
                        memset(device_ip_id,0,32);
                        memcpy(device_ip_id,tmp,length);
                    }
                    if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "device_username")) //得到device_username
                    {
                        tmp = bson_iter_utf8(&iter,&length);
                        memset(device_username,0,32);
                        memcpy(device_username,tmp,length);
                    }
                    if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "device_password")) //得到device_password
                    {
                        tmp = bson_iter_utf8(&iter,&length);
                        memset(device_password,0,32);
                        memcpy(device_password,tmp,length);
                    }
                    Json::Value json_tmp;
                    json_tmp["device_type"] = Json::Value(device_type);
                    json_tmp["device_ip_id"] = Json::Value(device_ip_id);
                    json_tmp["device_username"] = Json::Value(device_username);
                    json_tmp["device_password"] = Json::Value(device_password);

                    json_channel["ipc"].append(json_tmp);

                }
                else if(strcmp(device_type,"停车场控制器") == 0 || strcmp(device_type,"LED语音一体机") == 0)
                {
                    if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "device_ip_id"))
                    {
                        tmp = bson_iter_utf8(&iter,&length);
                        memset(device_ip_id,0,32);
                        memcpy(device_ip_id,tmp,length);
                    }
                    Json::Value json_tmp;
                    json_tmp["device_type"] = Json::Value(device_type);
                    json_tmp["device_ip_id"] = Json::Value(device_ip_id);

                    json_channel["ipc"].append(json_tmp);
                }
                bson_destroy(&result);
            }
            bson_destroy(query_device);

            mongoc_cursor_destroy(cursor_device);
            json_ipc_config["channel"].append(json_channel);
        }
    }

    bson_destroy(query);
    mongoc_cursor_destroy(cursor_channel);

    mongoc_collection_destroy(mongodb_table_channel);
    mongoc_collection_destroy(mongodb_table_device);
   // mongoc_client_destroy(mongodb_client);

    json_ipc_config["cmd"] = Json::Value("ipc_config");
    std::string send = json_ipc_config.toStyledString();
    char time_now[64];
    time_t tm;
    time_printf(time(&tm),time_now);

    char ip[] = {"127.0.0.1"};

    struct sockaddr_in addr;
    int sock;
    sock=socket(AF_INET, SOCK_DGRAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(_dst_port);
    addr.sin_addr.s_addr = inet_addr(ip);
    int n = sendto(sock, send.c_str(), send.length(), 0, (struct sockaddr *)&addr, sizeof(addr));
    if(n < 0)
    {
        fprintf(fp_log,"%s##bipc ipc_config发送失败\n",time_now);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 向bled发送消息失败 [%s] ip[%s] port[%d] line[%d]", __FUNCTION__, send.c_str(), ip, _dst_port, __LINE__);
        writelog(log_buf_);
    }
    else
    {
        fprintf(fp_log,"%s##bipc ipc_config发送成功:\n",time_now);
        fprintf(fp_log,"%s\n",send.c_str());

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 向bled成功发送一条消息[%s] ip[%s] port[%d] line[%d]", __FUNCTION__, send.c_str(), ip, _dst_port, __LINE__);
        writelog(log_buf_);
    }
    close(sock);
    return 0;

}
/**************************************************end******************************************/

/******************************给bipc发送whilte_list信息*******************************************/
int mongodb_get_white_list(int _dst_port)
{
    mongoc_collection_t *mongodb_table_car = mongoc_client_get_collection(mongodb_client, "boondb", "car");   //car表

    bson_t *query = bson_new();
    bson_error_t error;

    mongoc_cursor_t *cursor = mongoc_collection_find (mongodb_table_car, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
    if(mongoc_cursor_error(cursor, &error))
    {
        bson_destroy(query);
        mongoc_cursor_destroy(cursor);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 查询数据库异常! line[%d]", __FUNCTION__, __LINE__);
        writelog(log_buf_);

        return -1;
    }

    bool find_value = false;

    unsigned int length;
    bson_iter_t iter;
    bson_t result;

    const char* tmp;
    const bson_t *tmp1;

    char record[64] = {0};
    std::string record_str;

    // 记录数目
    int index = 0;

    rapidjson::Document doc;
    doc.SetObject();

    //rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

    Value value(rapidjson::kObjectType);

    doc.AddMember("cmd", "get_white_list", doc.GetAllocator());

    rapidjson::Value info_array(rapidjson::kArrayType);

    //Json::Value json_car; //通道json格式的信息
    // car_start_time, car_stop_time, car_plate_id
    while(!mongoc_cursor_error(cursor, &error) && mongoc_cursor_more(cursor)) //得到car表的每一条记录
    {
        //Json::Value json_tmp;
        if (mongoc_cursor_next (cursor, &tmp1)) // 下一条记录
        {
            rapidjson::Value info_object(rapidjson::kObjectType);
            info_object.SetObject();

            bson_copy_to(tmp1, &result); //得到一条完整的记录

            if (bson_iter_init(&iter, &result) && bson_iter_find(&iter, "car_plate_id")) //得到 car_plate_id
            {
                tmp = bson_iter_utf8(&iter, &length);
                memset(record, 0, sizeof(record));
                memcpy(record, tmp, length);

                std::string plate = record;
                value.SetString(StringRef(plate.c_str()));
                info_object.AddMember("car_plate_id", value, doc.GetAllocator());

                snprintf(log_buf_, sizeof(log_buf_) - 1, "[%s] plate[%s] line[%d]", __FUNCTION__, plate.c_str(), __LINE__);
                writelog(log_buf_);
            }

            if (bson_iter_init(&iter, &result) && bson_iter_find (&iter, "car_start_time")) //car_start_time
            {
                tmp = bson_iter_utf8(&iter, &length);
                memset(record, 0, sizeof(record));
                memcpy(record, tmp, length);

                std::string start = record;
                value.SetString(StringRef(start.c_str()));
                info_object.AddMember("car_start_time", value, doc.GetAllocator());

                snprintf(log_buf_, sizeof(log_buf_) - 1, "[%s] start[%s] line[%d]", __FUNCTION__, start.c_str(), __LINE__);
                writelog(log_buf_);
            }

            if (bson_iter_init(&iter, &result) && bson_iter_find (&iter, "car_stop_time")) //得到 car_stop_time
            {
                tmp = bson_iter_utf8(&iter, &length);
                memset(record, 0, sizeof(record));
                memcpy(record, tmp, length);

                std::string stop = record;
                value.SetString(StringRef(stop.c_str()));
                info_object.AddMember("car_stop_time", value, doc.GetAllocator());

                snprintf(log_buf_, sizeof(log_buf_) - 1, "[%s] stop[%s] line[%d]", __FUNCTION__, stop.c_str(), __LINE__);
                writelog(log_buf_);
            }

            index++;

            info_array.PushBack(info_object, doc.GetAllocator());

            if(index >= 5)
                break;

        } // end if

    } // end while

    doc.AddMember("car_list", info_array, doc.GetAllocator());

    rapidjson::StringBuffer buffer;//in rapidjson/stringbuffer.h
    rapidjson::Writer<StringBuffer> writer(buffer); //in rapidjson/writer.h
    doc.Accept(writer);

    std::string car_list = "";
    //car_list = json_car.toStyledString();
    car_list = buffer.GetString();

    snprintf(log_buf_, sizeof(log_buf_) - 1, "[%s] 获取car表 记录[%d] msg_len[%ld] 内部车列表[%s] line[%d]",
             __FUNCTION__, index, car_list.length(), car_list.c_str(), __LINE__);
    //writelog(log_buf_);

    char ip[] = {"127.0.0.1"};
    //
    return mogodbb_process_udp_send(car_list, ip, PORT_UDP_BCENTER_TO_BIPC);
}
/**************************************************end******************************************/

/******************************给bop发送ipc_config信息*******************************************/
int mongodb_get_ipc_config_bop()
{
   // mongoc_client_t *mongodb_client;
    mongoc_collection_t *mongodb_table_channel;	//channel表
    mongoc_collection_t *mongodb_table_device;	//device表
   // mongodb_client = mongoc_client_new(str_con);
    mongodb_table_channel = mongoc_client_get_collection(mongodb_client,"boondb","channel");   //channel表
    mongodb_table_device = mongoc_client_get_collection(mongodb_client,"boondb","device");   //device表

    mongoc_cursor_t *cursor_channel;
    mongoc_cursor_t *cursor_device;
    bson_t *query;
    bson_t result;
    bson_iter_t iter;
    const char *tmp;
    const bson_t *tmp1;
    bson_error_t error;
    unsigned int length;

    char channel_id[256];
    char channel_in_out[16];
    char channel_one_way[16];
    char device_type[32];
    char device_ip_id[32];
    char device_username[32];
    char device_password[32];

    Json::Value json_ipc_config; //发送给bop的json格式的信息

    query = bson_new();

    BSON_APPEND_UTF8 (query, "channel_ip", host_ip); //查询条件

    cursor_channel = mongoc_collection_find (mongodb_table_channel, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查channel表
    if(mongoc_cursor_error(cursor_channel,&error))
    {
        return -1;
    }
    while(!mongoc_cursor_error(cursor_channel,&error)&&mongoc_cursor_more(cursor_channel)) //得到小主机下属的每一个通道信息
    {
        Json::Value json_channel; //通道json格式的信息
        if (mongoc_cursor_next (cursor_channel, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "channel_id")) //得到channel_id
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(channel_id,0,256);
                memcpy(channel_id,tmp,length);
            }
            json_channel["channel_id"] = Json::Value(channel_id);  //json赋值 channel_id
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "channel_in_out")) //得到channel_in_out
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(channel_in_out,0,16);
                memcpy(channel_in_out,tmp,length);
            }
            json_channel["in_out"] = Json::Value(channel_in_out);//json赋值 channel_in_out

            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "channel_one_way")) //得到channel_in_out
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(channel_one_way,0,16);
                memcpy(channel_one_way,tmp,length);
            }
            json_channel["one_way"] = Json::Value(channel_one_way);//json赋值 channel_in_out


            bson_t *query_device;
            query_device = bson_new();
            BSON_APPEND_UTF8 (query_device, "device_channel_id", channel_id);  //查询条件

            cursor_device = mongoc_collection_find (mongodb_table_device, MONGOC_QUERY_NONE, 0, 0, 0, query_device, NULL, NULL); //根据通道id号查device表
            while(!mongoc_cursor_error(cursor_device,&error)&&mongoc_cursor_more(cursor_device)) //遍历该通道下每一条设备记录
            {
                if (mongoc_cursor_next (cursor_device, &tmp1))
                {
                    bson_copy_to(tmp1,&result);
                }
                else
                {
                    break;
                }
                if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "device_type")) //得到设备类型
                {
                    tmp = bson_iter_utf8(&iter,&length);
                    memset(device_type,0,32);
                    memcpy(device_type,tmp,length);
                }
                if(strcmp(device_type,"中维抓拍相机") == 0  || strcmp(device_type,"中维智能相机") == 0|| strcmp(device_type,"臻识智能相机") == 0)
                {
                    if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "device_ip_id")) //得到device_ip_id
                    {
                        tmp = bson_iter_utf8(&iter,&length);
                        memset(device_ip_id,0,32);
                        memcpy(device_ip_id,tmp,length);
                    }
                    if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "device_username")) //得到device_username
                    {
                        tmp = bson_iter_utf8(&iter,&length);
                        memset(device_username,0,32);
                        memcpy(device_username,tmp,length);
                    }
                    if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "device_password")) //得到device_password
                    {
                        tmp = bson_iter_utf8(&iter,&length);
                        memset(device_password,0,32);
                        memcpy(device_password,tmp,length);
                    }
                    Json::Value json_tmp;
                    json_tmp["device_type"] = Json::Value(device_type);
                    json_tmp["device_ip_id"] = Json::Value(device_ip_id);
                    json_tmp["device_username"] = Json::Value(device_username);
                    json_tmp["device_password"] = Json::Value(device_password);

                    json_channel["ipc"].append(json_tmp);

                }
                else if(strcmp(device_type,"停车场控制器") == 0 || strcmp(device_type,"LED语音一体机") == 0)
                {
                    if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "device_ip_id"))
                    {
                        tmp = bson_iter_utf8(&iter,&length);
                        memset(device_ip_id,0,32);
                        memcpy(device_ip_id,tmp,length);
                    }
                    Json::Value json_tmp;
                    json_tmp["device_type"] = Json::Value(device_type);
                    json_tmp["device_ip_id"] = Json::Value(device_ip_id);

                    json_channel["ipc"].append(json_tmp);
                }
                bson_destroy(&result);
            }
            bson_destroy(query_device);

            mongoc_cursor_destroy(cursor_device);

            json_ipc_config["channel"].append(json_channel);


        }
    }

    bson_destroy(query);

    mongoc_cursor_destroy(cursor_channel);
    mongoc_collection_destroy(mongodb_table_channel);
    mongoc_collection_destroy(mongodb_table_device);
   // mongoc_client_destroy(mongodb_client);

    json_ipc_config["cmd"] = Json::Value("ipc_config");
    json_ipc_config["server_ip"] = Json::Value(host_server_ip);
    json_ipc_config["local_ip"] = Json::Value(host_ip);
    std::string send = json_ipc_config.toStyledString();
    char time_now[64];
    time_t tm;
    time_printf(time(&tm),time_now);

    char ip[] = {"127.0.0.1"};

    struct sockaddr_in addr;
    int sock;
    sock=socket(AF_INET, SOCK_DGRAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT_UDP_BCENTER_TO_BOP);
    addr.sin_addr.s_addr = inet_addr(ip);
    int n = sendto(sock,send.c_str(),send.length(), 0, (struct sockaddr *)&addr, sizeof(addr));
    if(n < 0)
    {
        fprintf(fp_log,"%s##bop ipc_config发送失败\n",time_now);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 向bop发包失败 msg[%s] len[%ld] ip[%s] port[%d] line[%d]",
                 __FUNCTION__, send.c_str(), send.length(), ip, PORT_UDP_BCENTER_TO_BOP, __LINE__);
        writelog(log_buf_);
    }
    else
    {
        fprintf(fp_log,"%s##bop ipc_config发送成功:\n",time_now);
        fprintf(fp_log,"%s\n",send.c_str());

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 向bop发出去一个消息 msg[%s] len[%ld] ip[%s] port[%d] line[%d]",
                 __FUNCTION__, send.c_str(), send.length(), ip, PORT_UDP_BCENTER_TO_BOP, __LINE__);
        writelog(log_buf_);
    }
    close(sock);
    return 0;

}
/**************************************************end******************************************/
/******************************给bvs发送ipc_config信息*******************************************/
int mongodb_get_ipc_config_bvs()
{
  //  mongoc_client_t *mongodb_client;
    mongoc_collection_t *mongodb_table_channel;	//channel表
    mongoc_collection_t *mongodb_table_device;	//device表
   // mongodb_client = mongoc_client_new(str_con);
    mongodb_table_channel = mongoc_client_get_collection(mongodb_client,"boondb","channel");   //channel表
    mongodb_table_device = mongoc_client_get_collection(mongodb_client,"boondb","device");   //device表

    mongoc_cursor_t *cursor_channel;
    mongoc_cursor_t *cursor_device;
    bson_t *query;
    bson_t result;
    bson_iter_t iter;
    const char *tmp;
    const bson_t *tmp1;
    bson_error_t error;
    unsigned int length;


    query = bson_new();

    BSON_APPEND_UTF8 (query, "channel_ip", host_ip); //查询条件
    char channel_id[256];
    char channel_in_out[16];
    char channel_one_way[16];
    char device_type[32];
    char device_ip_id[32];
    char device_username[32];
    char device_password[32];

    Json::Value json_ipc_config; //发送给bop的json格式的信息

    cursor_channel = mongoc_collection_find (mongodb_table_channel, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查channel表
    if(mongoc_cursor_error(cursor_channel,&error))
    {

        return -1;
    }
    while(!mongoc_cursor_error(cursor_channel,&error)&&mongoc_cursor_more(cursor_channel)) //得到小主机下属的每一个通道信息
    {
        Json::Value json_channel; //通道json格式的信息
        if (mongoc_cursor_next (cursor_channel, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "channel_id")) //得到channel_id
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(channel_id,0,256);
                memcpy(channel_id,tmp,length);
            }
            json_channel["channel_id"] = Json::Value(channel_id);  //json赋值 channel_id
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "channel_in_out")) //得到channel_in_out
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(channel_in_out,0,16);
                memcpy(channel_in_out,tmp,length);
            }
            json_channel["in_out"] = Json::Value(channel_in_out);//json赋值 channel_in_out

            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "channel_one_way")) //得到channel_in_out
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(channel_one_way,0,16);
                memcpy(channel_one_way,tmp,length);
            }
            json_channel["one_way"] = Json::Value(channel_one_way);//json赋值 channel_in_out

            bson_t *query_device;
            query_device = bson_new();
            BSON_APPEND_UTF8 (query_device, "device_channel_id", channel_id);  //查询条件

            cursor_device = mongoc_collection_find (mongodb_table_device, MONGOC_QUERY_NONE, 0, 0, 0, query_device, NULL, NULL); //根据通道id号查device表
            while(!mongoc_cursor_error(cursor_device,&error)&&mongoc_cursor_more(cursor_device)) //遍历该通道下每一条设备记录
            {
                if (mongoc_cursor_next (cursor_device, &tmp1))
                {
                    bson_copy_to(tmp1,&result);
                }
                else
                {
                    break;
                }
                if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "device_type")) //得到设备类型
                {
                    tmp = bson_iter_utf8(&iter,&length);
                    memset(device_type,0,32);
                    memcpy(device_type,tmp,length);
                }
                if(strcmp(device_type,"中维抓拍相机") == 0  || strcmp(device_type,"中维智能相机") == 0|| strcmp(device_type,"臻识智能相机") == 0)
                {
                    if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "device_ip_id")) //得到device_ip_id
                    {
                        tmp = bson_iter_utf8(&iter,&length);
                        memset(device_ip_id,0,32);
                        memcpy(device_ip_id,tmp,length);
                    }
                    if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "device_username")) //得到device_username
                    {
                        tmp = bson_iter_utf8(&iter,&length);
                        memset(device_username,0,32);
                        memcpy(device_username,tmp,length);
                    }
                    if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "device_password")) //得到device_password
                    {
                        tmp = bson_iter_utf8(&iter,&length);
                        memset(device_password,0,32);
                        memcpy(device_password,tmp,length);
                    }
                    Json::Value json_tmp;
                    json_tmp["device_type"] = Json::Value(device_type);
                    json_tmp["device_ip_id"] = Json::Value(device_ip_id);
                    json_tmp["device_username"] = Json::Value(device_username);
                    json_tmp["device_password"] = Json::Value(device_password);

                    json_channel["ipc"].append(json_tmp);

                }
                else if(strcmp(device_type,"停车场控制器") == 0 || strcmp(device_type,"LED语音一体机") == 0)
                {
                    if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "device_ip_id"))
                    {
                        tmp = bson_iter_utf8(&iter,&length);
                        memset(device_ip_id,0,32);
                        memcpy(device_ip_id,tmp,length);
                    }
                    Json::Value json_tmp;
                    json_tmp["device_type"] = Json::Value(device_type);
                    json_tmp["device_ip_id"] = Json::Value(device_ip_id);

                    json_channel["ipc"].append(json_tmp);
                }
                bson_destroy(&result);
            }
            bson_destroy(query_device);
            mongoc_cursor_destroy(cursor_device);

            json_ipc_config["channel"].append(json_channel);


        }
    }

    bson_destroy(query);

    mongoc_cursor_destroy(cursor_channel);
    mongoc_collection_destroy(mongodb_table_channel);
    mongoc_collection_destroy(mongodb_table_device);
  //  mongoc_client_destroy(mongodb_client);

    json_ipc_config["cmd"] = Json::Value("ipc_config");
    std::string send = json_ipc_config.toStyledString();
    char time_now[64];
    time_t tm;
    time_printf(time(&tm),time_now);

    char ip[] = {"127.0.0.1"};

    struct sockaddr_in addr;
    int sock;
    sock=socket(AF_INET, SOCK_DGRAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT_UDP_BCENTER_TO_BVS);
    addr.sin_addr.s_addr = inet_addr(ip);
    int n = sendto(sock,send.c_str(),send.length(), 0, (struct sockaddr *)&addr, sizeof(addr));
    if(n < 0)
    {
        fprintf(fp_log,"%s##bvs ipc_config发送失败\n",time_now);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 向bvs发包失败[%s] ip[%s] port[%d] line[%d]", __FUNCTION__, send.c_str(), ip, PORT_UDP_BCENTER_TO_BVS, __LINE__);
        writelog(log_buf_);
    }
    else
    {
        fprintf(fp_log,"%s##bvs ipc_config发送成功:\n",time_now);
        fprintf(fp_log,"%s\n",send.c_str());

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 向bvs成功发送一条消息[%s] ip[%s] port[%d] line[%d]", __FUNCTION__, send.c_str(), ip, PORT_UDP_BCENTER_TO_BVS, __LINE__);
        writelog(log_buf_);
    }
    close(sock);
    return 0;

}
/**************************************************end******************************************/
/******************************查询这辆车在car表里的个数*******************************************/
int mongodb_query_car(char * plate)
{
   // mongoc_client_t *mongodb_client;
    mongoc_collection_t *mongodb_table_car;	//channel表

  //  mongodb_client = mongoc_client_new(str_con);
    mongodb_table_car = mongoc_client_get_collection(mongodb_client,"boondb","car");   //channel表

    int count = 0;
    mongoc_cursor_t *cursor;
    bson_t *query;
    bson_error_t error;

    query = bson_new();
    BSON_APPEND_UTF8(query, "car_plate_id", plate); //查询条件

    count = mongoc_collection_count (mongodb_table_car, MONGOC_QUERY_NONE, query, 0, 0, NULL, &error);

    bson_destroy(query);
    mongoc_collection_destroy(mongodb_table_car);
   // mongoc_client_destroy(mongodb_client);
    return count;
}
/**************************************************end******************************************/
/******************************根据车牌号在car表查询这辆车的车辆类型*******************************************/
int mongodb_query_cartype(char * plate,char *type)
{
    snprintf(log_buf_, sizeof(log_buf_), "[%s] 准备获取plate[%s]车辆类型 line[%d]", __FUNCTION__, plate, __LINE__);
    writelog(log_buf_);

    // mongoc_client_t *mongodb_client;
    mongoc_collection_t *mongodb_table_car;	//channel表

  //  mongodb_client = mongoc_client_new(str_con);
    mongodb_table_car = mongoc_client_get_collection(mongodb_client,"boondb","car");   //channel表

    int count = 0;
    mongoc_cursor_t *cursor;
    bson_t *query;
    bson_t result;
    const char *tmp;
    const bson_t *tmp1;
    bson_iter_t iter;
    bson_error_t error;
    unsigned int length;

    query = bson_new();

    // 车牌号
    BSON_APPEND_UTF8(query, "car_plate_id", plate); //查询条件

    memset(type,0,24);
    memcpy(type,"临时车",strlen("临时车"));
    cursor = mongoc_collection_find (mongodb_table_car, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);
        mongoc_cursor_destroy(cursor);

        char time_now[64];
        time_t tm;
        time_printf(time(&tm),time_now);
        fprintf(fp_log,"%s##mongodb_query_cartype 失败\n",time_now);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 连接数据库失败! 默认为临时车, return -1. line[%d]", __FUNCTION__, __LINE__);
        writelog(log_buf_);

        return -1;
    }

    bool find_value = false;
    if(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor))
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "car_type")) //得到车辆类型
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(type,0,24);
                memcpy(type,tmp,length);

                find_value = true;

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 找到车牌[%s]对应的车辆类型[%s] 查找car表 line[%d]", __FUNCTION__, plate, type, __LINE__);
                writelog(log_buf_);
            }
            bson_destroy(&result);
        }

    }
    bson_destroy(query);

    mongoc_cursor_destroy(cursor);
    mongoc_collection_destroy(mongodb_table_car);

    if(!find_value){
        snprintf(log_buf_, sizeof(log_buf_), "[%s] warn! 查找car表, 没有找到车牌[%s]对应的车辆类型, 修订为[%s] line[%d]", __FUNCTION__, plate, type, __LINE__);
        writelog(log_buf_);
    }

  //  mongoc_client_destroy(mongodb_client);
    return 0;
}
/**************************************************end******************************************/
int mongodb_query_carinpark(char * channel_id,char *plate,char *in_time,char *in_pic_path,char *in_channel_id,char *people_name,char *in_channel_name)
{
    snprintf(log_buf_, sizeof(log_buf_), "[%s] line[%d]", __FUNCTION__, __LINE__);
    writelog(log_buf_);

    //mongoc_client_t *mongodb_client;
    mongoc_collection_t *mongodb_table_channel;	//channel表
    mongoc_collection_t *mongodb_table_carinpark;	//device表
    //mongodb_client = mongoc_client_new(str_con);
    mongodb_table_channel = mongoc_client_get_collection(mongodb_client,"boondb","channel");   //channel表
    mongodb_table_carinpark = mongoc_client_get_collection(mongodb_client,"boondb","carinpark");   //device表

    int count = 0;
    mongoc_cursor_t *cursor;
    bson_t *query;
    bson_t result;
    const char *tmp;
    const bson_t *tmp1;
    bson_iter_t iter;
    bson_error_t error;
    unsigned int length;
    char park_id[256];

    char time_now[64];
    time_t tm;
    time_printf(time(&tm),time_now);

    query = bson_new();

    BSON_APPEND_UTF8(query, "channel_id", channel_id); //查询条件

    cursor = mongoc_collection_find (mongodb_table_channel, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);
        mongoc_cursor_destroy(cursor);

        fprintf(fp_log,"%s##mongodb_count_carinpark 失败\n",time_now);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 查找channel表 查询数据库失败 line[%d]", __FUNCTION__, __LINE__);
        writelog(log_buf_);

        return -1;
    }

    bool find_value = false;

    if(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor))
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "channel_park_id")) //得到车辆类型
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(park_id,0,256);
                memcpy(park_id,tmp,length);

                find_value = true;

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 查找channel表 找到channel_park_id[%s] line[%d]", __FUNCTION__, park_id, __LINE__);
                writelog(log_buf_);
            }
            bson_destroy(&result);
        }

    }
    bson_destroy(query);
    mongoc_cursor_destroy(cursor);

    if(!find_value){
        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 查找channel表 没有找到channel_park_id 查找条件channel_id[%s] line[%d]", __FUNCTION__, channel_id, __LINE__);
        writelog(log_buf_);
    }

    find_value = false;

    query = bson_new();

    BSON_APPEND_UTF8(query, "carinpark_park_id", park_id); //查询条件
    BSON_APPEND_UTF8(query, "carinpark_plate_id", plate); //查询条件

    memset(in_time,0,24);
    memset(in_pic_path,0,128);
    memset(in_channel_id,0,256);
    memset(people_name,0,24);
    cursor = mongoc_collection_find (mongodb_table_carinpark, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);
        char time_now[64];
        time_t tm;
        time_printf(time(&tm),time_now);

        fprintf(fp_log,"%s##mongodb_count_carinpark 失败\n",time_now);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 查找carinpark表 查询数据库失败 line[%d]", __FUNCTION__, __LINE__);
        writelog(log_buf_);
        return -1;
    }

    while(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor))
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "carinpark_in_time")) //得到车辆类型
            {
                tmp = bson_iter_utf8(&iter,&length);

                memcpy(in_time,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "carinpark_pic_path")) //得到车辆类型
            {
                tmp = bson_iter_utf8(&iter,&length);

                memcpy(in_pic_path,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "carinpark_channel_id")) //得到车辆类型
            {
                tmp = bson_iter_utf8(&iter,&length);

                memcpy(in_channel_id,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "carinpark_people_name")) //得到车辆类型
            {
                tmp = bson_iter_utf8(&iter,&length);

                memcpy(people_name,tmp,length);
            }
            bson_destroy(&result);
        }

    }
    bson_destroy(query);
    mongoc_cursor_destroy(cursor);

    query = bson_new();

    BSON_APPEND_UTF8(query, "channel_id", in_channel_id); //查询条件

    cursor = mongoc_collection_find (mongodb_table_channel, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);
        char time_now[64];
        time_t tm;
        time_printf(time(&tm),time_now);

        fprintf(fp_log,"%s##mongodb_count_carinpark 失败\n",time_now);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 查找channel表 查询数据库失败 line[%d]", __FUNCTION__, __LINE__);
        writelog(log_buf_);

        return -1;
    }

    find_value = false;

    while(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor))
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "channel_name")) //得到车辆类型
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(in_channel_name,0,256);
                memcpy(in_channel_name,tmp,length);

                find_value = true;

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 找到channel_name[%s] 查找channel表 line[%d]", __FUNCTION__, in_channel_name, __LINE__);
                writelog(log_buf_);
            }
            bson_destroy(&result);
        }

    }
    bson_destroy(query);
    mongoc_cursor_destroy(cursor);

    if(!find_value){
        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 查找channel表 没有找到channel_name 查找条件channel_id[%s] line[%d]", __FUNCTION__, in_channel_id, __LINE__);
        writelog(log_buf_);
    }

    query = bson_new();
    BSON_APPEND_UTF8(query, "carinpark_park_id", park_id); //查询条件
    BSON_APPEND_UTF8(query, "carinpark_plate_id", plate); //查询条件

    count = mongoc_collection_count (mongodb_table_carinpark, MONGOC_QUERY_NONE, query, 0, 0, NULL, &error);
    fprintf(fp_log,"%s##count is %d intime is %s\n",time_now,count,in_time);
    bson_destroy(query);
    mongoc_collection_destroy(mongodb_table_channel);
    mongoc_collection_destroy(mongodb_table_carinpark);
  //  mongoc_client_destroy(mongodb_client);

    if(count <= 0){
        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 查找carinpark表 没有找到车牌plate[%s]的入场时间! 查询条件:车牌carinpark_plate_id[%s] 停车场carinpark_park_id[%s] line[%d]",
                 __FUNCTION__, plate, plate, park_id, __LINE__);
        writelog(log_buf_);
    }else{
        snprintf(log_buf_, sizeof(log_buf_), "[%s] 查找carinpark表 车牌plate[%s] count[%d] line[%d]", __FUNCTION__, plate, count, __LINE__);
        writelog(log_buf_);
    }

    return count;

}
/******************************根据通道id号查询通道属性*******************************************/
int mongodb_query_channel(char * id,char *auth_type,char * open_door,char * one_way,char * mohu_match,char * park_id)
{
    //mongoc_client_t *mongodb_client;
    mongoc_collection_t *mongodb_table_channel;	//channel表

   // mongodb_client = mongoc_client_new(str_con);
    mongodb_table_channel = mongoc_client_get_collection(mongodb_client,"boondb","channel");   //channel表

    char res[24];
    mongoc_cursor_t *cursor;
    bson_t *query;
    bson_t result;

    const char *tmp;
    const bson_t *tmp1;
    bson_iter_t iter;
    bson_error_t error;
    unsigned int length;

    query = bson_new();

    BSON_APPEND_UTF8(query, "channel_id", id); //查询条件

    cursor = mongoc_collection_find (mongodb_table_channel, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查channel表
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);
        char time_now[64];
        time_t tm;
        time_printf(time(&tm),time_now);
        fprintf(fp_log,"%s##mongodb_query_channel 失败\n",time_now);

        return -1;
    }
    if(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor))
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "channel_fuzzy_match")) //得到模糊匹配的属性
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(mohu_match,0,24);
                memcpy(mohu_match,tmp,length);
            }
            if(bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "channel_auth_type")) //得到授权类型
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(auth_type,0,24);
                memcpy(auth_type,tmp,length);
            }
            if(bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "channel_open_door"))  //得到开闸类型
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(open_door,0,24);
                memcpy(open_door,tmp,length);
            }
            if(bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "channel_one_way")) //得到同口进出的属性
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(one_way,0,24);
                memcpy(one_way,tmp,length);
            }
            if(bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "channel_park_id")) //得到停车场id
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(park_id,0,256);
                memcpy(park_id,tmp,length);
            }
            bson_destroy(&result);
        }
    }
    bson_destroy(query);
    //bson_destroy(result);
    mongoc_cursor_destroy(cursor);
    mongoc_collection_destroy(mongodb_table_channel);

  //  mongoc_client_destroy(mongodb_client);
    return 0;
}
/**************************************************end******************************************/

/******************************mongodb的模糊匹配*******************************************/
int mongodb_mohu_match(char *pcolor,char *inplate,char *outplate)
{
    snprintf(log_buf_, sizeof(log_buf_), "[%s] 模糊匹配 待匹配的车牌 inplate[%s] line[%d]", __FUNCTION__, inplate, __LINE__);
    writelog(log_buf_);

   // mongoc_client_t *mongodb_client;
    mongoc_collection_t *mongodb_table_car;	//channel表

   // mongodb_client = mongoc_client_new(str_con);
    mongodb_table_car = mongoc_client_get_collection(mongodb_client,"boondb","car");   //channel表

    char res[24];

    mongoc_cursor_t *cursor;
    bson_t *query;
    bson_t *query_tmp;
    bson_t result;
    const char *tmp;
    const bson_t *tmp1;
    bson_iter_t iter;
    bson_error_t error;
    unsigned int length;

    char compare[24];

    std::string plate(inplate);

    char time_now[64];
    time_t tm;
    time_printf(time(&tm),time_now);

    fprintf(fp_log,"%s##模糊匹配收到的pcolor为%s,车牌号为%s\n",time_now,pcolor,inplate);
    if(strcmp("blue",pcolor) == 0 || strcmp("yellow",pcolor) == 0) //车牌为蓝牌或黄牌
    {
        query = bson_new();
	    query_tmp = bson_new();
	
        memset(compare,0,24);
        sprintf(compare,"*%s",plate.substr(3,6).c_str()); //匹配条件为*AB925E
        fprintf(fp_log,"%s##模糊匹配 汉字匹配为%s\n",time_now,compare);
	    BSON_APPEND_UTF8(query_tmp, "$regex",plate.substr(3,6).c_str());
	    BSON_APPEND_DOCUMENT(query, "car_plate_id",query_tmp);
	
        cursor = mongoc_collection_find (mongodb_table_car, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
        if(mongoc_cursor_error(cursor,&error))
        {
            bson_destroy(query);

            mongoc_cursor_destroy(cursor);

            fprintf(fp_log,"%s##mongodb_mohu_match 失败\n",time_now);

            snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 查询mongodb失败 在car表未找到 car_plate_id[%s] color[%s] line[%d]", __FUNCTION__, inplate, pcolor, __LINE__);
            writelog(log_buf_);

            return -1;
        }
        while(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor)) //得到car表的每一条记录
        {
            if (mongoc_cursor_next (cursor, &tmp1))
            {
                bson_copy_to(tmp1,&result); //得到一条完整的记录
                if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "car_plate_id")) //得到车牌号
                {
                    tmp = bson_iter_utf8(&iter,&length);
                    memset(res,0,24);
                    memcpy(res,tmp,length);
                }
                
                {
                    fprintf(fp_log,"%s##模糊匹配 汉字匹配成功 车牌号为%s\n",time_now,res);
                    memcpy(outplate,res,PLATE_LENGTH);
                    bson_destroy(&result);
                    bson_destroy(query);
                    mongoc_collection_destroy(mongodb_table_car);
                  //  mongoc_client_destroy(mongodb_client);
                    return 0;
                }
                bson_destroy(&result);
            }
        }
        bson_destroy(query);
	    bson_destroy(query_tmp);

        mongoc_cursor_destroy(cursor);

        for(int i = 0;i<6;i++)
        {
            int seg = i + 3;
            std::string tmp1=plate.substr(seg,1);
            int val_asc = (int) tmp1.c_str()[0];
            if(strlen(tmp1.c_str()) < 1)
                break;
            fprintf(fp_log,"%s##模糊匹配 匹配的字符为%s\n",time_now,tmp1.c_str());
            if(val_asc < 60)  //数字
            {
                val_asc = val_asc - 48;

                for(int j = 0;j<10;j++)
                {
                    if(number[val_asc][j] == '@') //遇到数组里的'@'，则跳出循环
                        break;

                    query = bson_new();
                    char plate_query[PLATE_LENGTH];
                    memset(plate_query,0,PLATE_LENGTH);

                    sprintf(plate_query,"%s%c%s",plate.substr(0,seg).c_str(),number[val_asc][j],plate.substr(seg + 1,plate.length() - seg - 1).c_str()); //组成查询的车牌号
                    fprintf(fp_log,"%s##模糊匹配 把%s变为%s到数据库查询\n",time_now,inplate,plate_query);
                    BSON_APPEND_UTF8(query, "car_plate_id", plate_query); //查询条件

                    int count = mongoc_collection_count (mongodb_table_car, MONGOC_QUERY_NONE, query, 0, 0, NULL, &error); //获取个数
                    bson_destroy(query);

                    if(count > 0) //匹配到则返回
                    {
                        fprintf(fp_log,"%s##模糊匹配成功 车牌号为%s\n",time_now,plate_query);
                        memcpy(outplate,plate_query,PLATE_LENGTH);
                        mongoc_collection_destroy(mongodb_table_car);
                     //   mongoc_client_destroy(mongodb_client);
                        //bson_destroy(query);
                        return 0;
                    }
                }
            }
            else //字母
            {
                val_asc = val_asc - 65;
                for(int j = 0;j<10;j++)
                {
                    if(alpha[val_asc][j] == '@') //遇到数组里的'@'，则跳出循环
                        break;

                    query = bson_new();
                    char plate_query[PLATE_LENGTH];
                    memset(plate_query,0,PLATE_LENGTH);
                    sprintf(plate_query,"%s%c%s",plate.substr(0,seg).c_str(),alpha[val_asc][j],plate.substr(seg + 1,plate.length() - seg - 1).c_str()); //组成查询的车牌号
                    fprintf(fp_log,"%s##模糊匹配 把%s变为%s到数据库查询\n",time_now,inplate,plate_query);
                    BSON_APPEND_UTF8(query, "car_plate_id", plate_query); //查询条件

                    int count = mongoc_collection_count (mongodb_table_car, MONGOC_QUERY_NONE, query, 0, 0, NULL, &error); //获取个数
                    bson_destroy(query);

                    if(count > 0) //匹配到则返回
                    {
                        fprintf(fp_log,"%s##模糊匹配成功 车牌号为%s\n",time_now,plate_query);
                        memcpy(outplate,plate_query,PLATE_LENGTH);
                        mongoc_collection_destroy(mongodb_table_car);
                     //   mongoc_client_destroy(mongodb_client);
                        //bson_destroy(query);
                        return 0;
                    }
                }
            }


        }

    }

    memcpy(outplate,inplate,PLATE_LENGTH); //未匹配成功，则两者车牌一致
    mongoc_collection_destroy(mongodb_table_car);
  //  mongoc_client_destroy(mongodb_client);
    return 0;


}
int mongodb_mohu_match_out(char *pcolor,char *inplate,char *outplate)
{
  //  mongoc_client_t *mongodb_client;
    mongoc_collection_t *mongodb_table_car;	//channel表
    mongoc_collection_t *mongodb_table_carinpark;	//channel表

  //  mongodb_client = mongoc_client_new(str_con);
    mongodb_table_car = mongoc_client_get_collection(mongodb_client,"boondb","car");   //channel表
    mongodb_table_carinpark = mongoc_client_get_collection(mongodb_client,"boondb","carinpark");   //channel表
    char res[24];

    mongoc_cursor_t *cursor;
    bson_t *query;
    bson_t *query_tmp;
    bson_t result;
    const char *tmp;
    const bson_t *tmp1;
    bson_iter_t iter;
    bson_error_t error;
    unsigned int length;

    char compare[24];

    std::string plate(inplate);

    char time_now[64];
    time_t tm;
    time_printf(time(&tm),time_now);

    fprintf(fp_log,"%s##模糊匹配收到的pcolor为%s,车牌号为%s\n",time_now,pcolor,inplate);
    if(strcmp("blue",pcolor) == 0 || strcmp("yellow",pcolor) == 0) //车牌为蓝牌或黄牌
    {
        query = bson_new();
	    query_tmp = bson_new();
        memset(compare,0,24);
        sprintf(compare,"*%s",plate.substr(3,6).c_str()); //匹配条件为*AB925E
        fprintf(fp_log,"%s##模糊匹配 汉字匹配为%s\n",time_now,compare);
	    BSON_APPEND_UTF8(query_tmp, "$regex",plate.substr(3,6).c_str());
	    BSON_APPEND_DOCUMENT(query, "car_plate_id",query_tmp);
        cursor = mongoc_collection_find (mongodb_table_car, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
        if(mongoc_cursor_error(cursor,&error))
        {
            bson_destroy(query);

            mongoc_cursor_destroy(cursor);

            fprintf(fp_log,"%s##mongodb_mohu_match 失败\n",time_now);

            return -1;
        }
        while(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor)) //得到car表的每一条记录
        {
            if (mongoc_cursor_next (cursor, &tmp1))
            {
                bson_copy_to(tmp1,&result); //得到一条完整的记录
                if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "car_plate_id")) //得到车牌号
                {
                    tmp = bson_iter_utf8(&iter,&length);
                    memset(res,0,24);
                    memcpy(res,tmp,length);
                }
              
                {
                    fprintf(fp_log,"%s##模糊匹配 汉字匹配成功 车牌号为%s\n",time_now,res);
                    memcpy(outplate,res,PLATE_LENGTH);
                    bson_destroy(&result);
                    bson_destroy(query);
                    mongoc_collection_destroy(mongodb_table_car);
                    mongoc_collection_destroy(mongodb_table_carinpark);
                //    mongoc_client_destroy(mongodb_client);
                    return 0;
                }
                bson_destroy(&result);
            }
        }
        bson_destroy(query);
	    bson_destroy(query_tmp);

        mongoc_cursor_destroy(cursor);

        for(int i = 0;i<6;i++)
        {
            int seg = i + 3;
            std::string tmp1=plate.substr(seg,1);
            int val_asc = (int) tmp1.c_str()[0];
            if(strlen(tmp1.c_str()) < 1)
                break;
            fprintf(fp_log,"%s##模糊匹配 匹配的字符为%s\n",time_now,tmp1.c_str());
            if(val_asc < 60)  //数字
            {
                val_asc = val_asc - 48;

                for(int j = 0;j<10;j++)
                {
                    if(number[val_asc][j] == '@') //遇到数组里的'@'，则跳出循环
                        break;

                    query = bson_new();
                    char plate_query[PLATE_LENGTH];
                    memset(plate_query,0,PLATE_LENGTH);

                    sprintf(plate_query,"%s%c%s",plate.substr(0,seg).c_str(),number[val_asc][j],plate.substr(seg + 1,plate.length() - seg - 1).c_str()); //组成查询的车牌号
                    fprintf(fp_log,"%s##模糊匹配 把%s变为%s到数据库查询\n",time_now,inplate,plate_query);
                    BSON_APPEND_UTF8(query, "car_plate_id", plate_query); //查询条件

                    int count = mongoc_collection_count (mongodb_table_car, MONGOC_QUERY_NONE, query, 0, 0, NULL, &error); //获取个数
                    bson_destroy(query);

                    if(count > 0) //匹配到则返回
                    {
                        fprintf(fp_log,"%s##模糊匹配成功 车牌号为%s\n",time_now,plate_query);
                        memcpy(outplate,plate_query,PLATE_LENGTH);
                        //bson_destroy(query);
                        mongoc_collection_destroy(mongodb_table_car);
                        mongoc_collection_destroy(mongodb_table_carinpark);
                    //    mongoc_client_destroy(mongodb_client);
                        return 0;
                    }
                }
            }
            else //字母
            {
                val_asc = val_asc - 65;
                for(int j = 0;j<10;j++)
                {
                    if(alpha[val_asc][j] == '@') //遇到数组里的'@'，则跳出循环
                        break;

                    query = bson_new();
                    char plate_query[PLATE_LENGTH];
                    memset(plate_query,0,PLATE_LENGTH);
                    sprintf(plate_query,"%s%c%s",plate.substr(0,seg).c_str(),alpha[val_asc][j],plate.substr(seg + 1,plate.length() - seg - 1).c_str()); //组成查询的车牌号
                    fprintf(fp_log,"%s##模糊匹配 把%s变为%s到数据库查询\n",time_now,inplate,plate_query);
                    BSON_APPEND_UTF8(query, "car_plate_id", plate_query); //查询条件

                    int count = mongoc_collection_count (mongodb_table_car, MONGOC_QUERY_NONE, query, 0, 0, NULL, &error); //获取个数
                    bson_destroy(query);

                    if(count > 0) //匹配到则返回
                    {
                        fprintf(fp_log,"%s##模糊匹配成功 车牌号为%s\n",time_now,plate_query);
                        memcpy(outplate,plate_query,PLATE_LENGTH);
                        mongoc_collection_destroy(mongodb_table_car);
                        mongoc_collection_destroy(mongodb_table_carinpark);
                      //  mongoc_client_destroy(mongodb_client);
                        //bson_destroy(query);
                        return 0;
                    }
                }
            }


        }

        query = bson_new();
	    query_tmp = bson_new();

        memset(compare,0,24);
        sprintf(compare,"*%s",plate.substr(3,6).c_str()); //匹配条件为*AB925E
        fprintf(fp_log,"%s##模糊匹配 汉字匹配为%s\n",time_now,compare);
	    BSON_APPEND_UTF8(query_tmp, "$regex",plate.substr(3,6).c_str());
	    BSON_APPEND_DOCUMENT(query, "car_plate_id",query_tmp);
        cursor = mongoc_collection_find (mongodb_table_carinpark, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
        if(mongoc_cursor_error(cursor,&error))
        {
            bson_destroy(query);

            mongoc_cursor_destroy(cursor);

            fprintf(fp_log,"%s##mongodb_mohu_match 失败\n",time_now);

            return -1;
        }

        while(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor)) //得到car表的每一条记录
        {
            if (mongoc_cursor_next (cursor, &tmp1))
            {
                bson_copy_to(tmp1,&result); //得到一条完整的记录
                if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "carinpark_plate_id")) //得到车牌号
                {
                    tmp = bson_iter_utf8(&iter,&length);
                    memset(res,0,24);
                    memcpy(res,tmp,length);
                }
               
                {
                    fprintf(fp_log,"%s##模糊匹配 汉字匹配成功 车牌号为%s\n",time_now,res);
                    memcpy(outplate,res,PLATE_LENGTH);
                    bson_destroy(&result);
                    bson_destroy(query);
                    mongoc_collection_destroy(mongodb_table_car);
                    mongoc_collection_destroy(mongodb_table_carinpark);
                  //  mongoc_client_destroy(mongodb_client);
                    return 0;
                }
                bson_destroy(&result);
            }
        }
        bson_destroy(query);
	    bson_destroy(query_tmp);

        mongoc_cursor_destroy(cursor);

        for(int i = 0;i<6;i++)
        {
            int seg = i + 3;
            std::string tmp1=plate.substr(seg,1);
            int val_asc = (int) tmp1.c_str()[0];
            if(strlen(tmp1.c_str()) < 1)
                break;
            fprintf(fp_log,"%s##模糊匹配 匹配的字符为%s\n",time_now,tmp1.c_str());
            if(val_asc < 60)  //数字
            {
                val_asc = val_asc - 48;

                for(int j = 0;j<10;j++)
                {
                    if(number[val_asc][j] == '@') //遇到数组里的'@'，则跳出循环
                        break;

                    query = bson_new();
                    char plate_query[PLATE_LENGTH];
                    memset(plate_query,0,PLATE_LENGTH);

                    sprintf(plate_query,"%s%c%s",plate.substr(0,seg).c_str(),number[val_asc][j],plate.substr(seg + 1,plate.length() - seg - 1).c_str()); //组成查询的车牌号
                    fprintf(fp_log,"%s##模糊匹配 把%s变为%s到数据库查询\n",time_now,inplate,plate_query);
                    BSON_APPEND_UTF8(query, "carinpark_plate_id", plate_query); //查询条件

                    int count = mongoc_collection_count (mongodb_table_carinpark, MONGOC_QUERY_NONE, query, 0, 0, NULL, &error); //获取个数
                    bson_destroy(query);

                    if(count > 0) //匹配到则返回
                    {
                        fprintf(fp_log,"%s##模糊匹配成功 车牌号为%s\n",time_now,plate_query);
                        memcpy(outplate,plate_query,PLATE_LENGTH);
                        mongoc_collection_destroy(mongodb_table_car);
                        mongoc_collection_destroy(mongodb_table_carinpark);
                      //  mongoc_client_destroy(mongodb_client);
                        //bson_destroy(query);
                        return 0;
                    }
                }
            }
            else //字母
            {
                val_asc = val_asc - 65;
                for(int j = 0;j<10;j++)
                {
                    if(alpha[val_asc][j] == '@') //遇到数组里的'@'，则跳出循环
                        break;

                    query = bson_new();
                    char plate_query[PLATE_LENGTH];
                    memset(plate_query,0,PLATE_LENGTH);
                    sprintf(plate_query,"%s%c%s",plate.substr(0,seg).c_str(),alpha[val_asc][j],plate.substr(seg + 1,plate.length() - seg - 1).c_str()); //组成查询的车牌号
                    fprintf(fp_log,"%s##模糊匹配 把%s变为%s到数据库查询\n",time_now,inplate,plate_query);
                    BSON_APPEND_UTF8(query, "carinpark_plate_id", plate_query); //查询条件

                    int count = mongoc_collection_count (mongodb_table_carinpark, MONGOC_QUERY_NONE, query, 0, 0, NULL, &error); //获取个数
                    bson_destroy(query);

                    if(count > 0) //匹配到则返回
                    {
                        fprintf(fp_log,"%s##模糊匹配成功 车牌号为%s\n",time_now,plate_query);
                        memcpy(outplate,plate_query,PLATE_LENGTH);
                        //bson_destroy(query);
                        mongoc_collection_destroy(mongodb_table_car);
                        mongoc_collection_destroy(mongodb_table_carinpark);
                      //  mongoc_client_destroy(mongodb_client);
                        return 0;
                    }
                }
            }
        }
    }

    memcpy(outplate,inplate,PLATE_LENGTH); //未匹配成功，则两者车牌一致
    mongoc_collection_destroy(mongodb_table_car);
    mongoc_collection_destroy(mongodb_table_carinpark);
  //  mongoc_client_destroy(mongodb_client);
    return 0;
}
/**************************************************end******************************************/
/******************************删除该车辆在在场表里的记录*******************************************/
int mongodb_delete_car_inpark(char *plate,char *id)
{
   // mongoc_client_t *mongodb_client;
    mongoc_collection_t *mongodb_table_carinpark;	//channel表

  //  mongodb_client = mongoc_client_new(str_con);
    mongodb_table_carinpark = mongoc_client_get_collection(mongodb_client,"boondb","carinpark");   //channel表

    char res[24];
    mongoc_cursor_t *cursor;
    bson_t *query;

    const char *tmp;
    const bson_t *tmp1;
    bson_iter_t iter;
    bson_error_t error;
    unsigned int length;

    query = bson_new();

    BSON_APPEND_UTF8(query, "carinpark_plate_id", plate); //查询条件
    BSON_APPEND_UTF8(query, "carinpark_park_id", id); //查询条件

    if(mongoc_collection_count (mongodb_table_carinpark, MONGOC_QUERY_NONE, query, 0, 0, NULL, &error) > 0 )
    {
        mongoc_collection_remove (mongodb_table_carinpark, MONGOC_REMOVE_NONE,query, NULL, &error);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 出场时, 删除carinpark表中的车牌号plate[%s]记录! line[%d]", __FUNCTION__, plate, __LINE__);
        writelog(log_buf_);
    }
    bson_destroy(query);
    mongoc_collection_destroy(mongodb_table_carinpark);
  //  mongoc_client_destroy(mongodb_client);
    return 0;
}
/******************************end*************************************************************/
/******************************按车牌号授权******************************************************/
int mongodb_auth_plate(char *id,char *plate,bool *auth)
{
  //  mongoc_client_t *mongodb_client;
    mongoc_collection_t *mongodb_table_plateauth;	//channel表

  //  mongodb_client = mongoc_client_new(str_con);
    mongodb_table_plateauth = mongoc_client_get_collection(mongodb_client,"boondb","plateauth");   //channel表
    char res[24];
    mongoc_cursor_t *cursor;
    bson_t *query;

    const char *tmp;
    const bson_t *tmp1;
    bson_iter_t iter;
    bson_error_t error;
    unsigned int length;
    //*auth = false;
    int count = 0;

    query = bson_new();

    BSON_APPEND_UTF8(query, "plateauth_channel_id", id); //查询条件
    BSON_APPEND_UTF8(query, "plateauth_plate_id", plate); //查询条件
    count = mongoc_collection_count (mongodb_table_plateauth, MONGOC_QUERY_NONE, query, 0, 0, NULL, &error); //根据通道id号和车牌号查询plateauth表，如果能查到，则该车已授权，查不到，则该车未授权

    if(count > 0)
    {
        *auth = true;

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 查找plateauth表 获取车牌plate[%s]的权限[%d] line[%d]", __FUNCTION__, plate, *auth, __LINE__);
        writelog(log_buf_);
    }

    bson_destroy(query);
    mongoc_collection_destroy(mongodb_table_plateauth);
//    mongoc_client_destroy(mongodb_client);
    return 0;
}
/******************************end***********************************************************/
/******************************按车辆类型授权******************************************************/
int mongodb_auth_type(char *id,char *plate,bool *auth)
{
    snprintf(log_buf_, sizeof(log_buf_), "[%s] 按车辆类型授权 line[%d]", __FUNCTION__, __LINE__);
    writelog(log_buf_);

  //  mongoc_client_t *mongodb_client;
    mongoc_collection_t *mongodb_table_typeauth;	//channel表

//    mongodb_client = mongoc_client_new(str_con);
    mongodb_table_typeauth = mongoc_client_get_collection(mongodb_client,"boondb","typeauth");   //channel表

    char res[24];
    char type[24];
    mongoc_cursor_t *cursor;
    bson_t *query;

    const char *tmp;
    const bson_t *tmp1;
    bson_iter_t iter;
    bson_error_t error;
    unsigned int length;
    //*auth = false;
    int count = 0;

    char time_now[64];
    time_t tm;
    time_printf(time(&tm),time_now);

    if(mongodb_query_cartype(plate,type) < 0) //获取车辆类型
    {
        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 获取车牌plate[%s]的车辆类型失败, return -1! line[%d]", __FUNCTION__, plate, __LINE__);
        writelog(log_buf_);

        return -1;
    }

    fprintf(fp_log,"%s##车辆类型为%s\n",time_now,type);

    query = bson_new();

    BSON_APPEND_UTF8(query, "typeauth_channel_id", id); //查询条件
    BSON_APPEND_UTF8(query, "typeauth_car_type", type); //查询条件
    //char* str = bson_as_json(query, NULL);
    //fprintf(fp_log,"%s##车辆类型为%s\n",time_now,str);
    count = mongoc_collection_count (mongodb_table_typeauth, MONGOC_QUERY_NONE, query, 0, 0, NULL, &error); //根据通道id号和车辆类型查询typeauth表，如果能查到，则该车已授权，查不到，则该车未授权

    if(count > 0)
    {
        *auth = true;

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 查找typeauth表, 获取通道chanel_id[%s]和车辆类型[%s]的记录, 已授权 count[%d] line[%d]", __FUNCTION__, id, type, count, __LINE__);
        writelog(log_buf_);
    }else{
        snprintf(log_buf_, sizeof(log_buf_), "[%s] warn! 车辆类型[%s]的通道授权记录,说明未授权, 查找typeauth表 count[%d] line[%d]", __FUNCTION__, type, count, __LINE__);
        writelog(log_buf_);
    }

    bson_destroy(query);
    mongoc_collection_destroy(mongodb_table_typeauth);
 //   mongoc_client_destroy(mongodb_client);
    return 0;
}
/******************************end***********************************************************/
/******************************车位数验证是否开启***********************************************/
int mongodb_validate_chewei(bool *flag,char *channel)//车位数验证是否开启
{
 //   mongoc_client_t *mongodb_client;
    mongoc_collection_t *mongodb_table_channel;	//channel表
    mongoc_collection_t *mongodb_table_park;	//device表
 //   mongodb_client = mongoc_client_new(str_con);
    mongodb_table_channel = mongoc_client_get_collection(mongodb_client,"boondb","channel");   //channel表
    mongodb_table_park = mongoc_client_get_collection(mongodb_client,"boondb","park");   //device表

    char time_now[64];
    time_t tm;
    time_printf(time(&tm),time_now);

    unsigned int length = 0;
    char park_id[1024] = {0};
    char park_limit[64] = {0};

    {
        bson_t *query = bson_new();
        BSON_APPEND_UTF8(query, "channel_id", channel); //查询条件
        mongoc_cursor_t *cursor = mongoc_collection_find (mongodb_table_channel, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
        bson_error_t error;
        if(mongoc_cursor_error(cursor,&error))
        {
            bson_destroy(query);
            mongoc_cursor_destroy(cursor);
            fprintf(fp_log,"%s##mongodb_validate_chewei 失败\n",time_now);
            return -1;
        }
        while(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor))
        {
            const bson_t *tmp = NULL;
            if (mongoc_cursor_next (cursor, &tmp))
            {
                bson_t result;
                bson_copy_to(tmp,&result);
                bson_iter_t iter;
                if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "channel_park_id"))
                {
                    const char *src = bson_iter_utf8(&iter,&length);
                    memcpy(park_id,src,length);
                }
                bson_destroy(&result);
            }
        }
        bson_destroy(query);
        mongoc_cursor_destroy(cursor);
    }

    {
        bson_t *query = bson_new();
        BSON_APPEND_UTF8(query, "park_id", park_id); //查询条件
        mongoc_cursor_t *cursor = mongoc_collection_find (mongodb_table_park, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
        bson_error_t error;
        if(mongoc_cursor_error(cursor,&error))
        {
            bson_destroy(query);
            mongoc_cursor_destroy(cursor);
            fprintf(fp_log,"%s##mongodb_validate_chewei 失败\n",time_now);
            return -1;
        }
        while(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor))
        {
            const bson_t *tmp = NULL;
            if (mongoc_cursor_next (cursor, &tmp))
            {
                bson_t result;
                bson_copy_to(tmp,&result);
                bson_iter_t iter;
                if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "park_limit"))
                {
                    const char *src = bson_iter_utf8(&iter,&length);
                    memcpy(park_limit,src,length);
                }
                bson_destroy(&result);
            }
        }
        bson_destroy(query);
        mongoc_cursor_destroy(cursor);
    }
    if(strcmp(park_limit,"是") == 0)
    {
        *flag = true;
        fprintf(fp_log,"%s开启车位数限制%s\n",time_now,park_limit);

    }
    else
    {
        *flag = false;
        fprintf(fp_log,"%s关闭车位数限制%s\n",time_now,park_limit);
    }
    mongoc_collection_destroy(mongodb_table_channel);
    mongoc_collection_destroy(mongodb_table_park);
  //  mongoc_client_destroy(mongodb_client);
}
/******************************end***********************************************************/
/******************************车位数验证******************************************************/
int mongodb_validate_chewei_num(char *plate,bool *auth)
{
   // mongoc_client_t *mongodb_client;
    mongoc_collection_t *mongodb_table_channel;	//channel表
    mongoc_collection_t *mongodb_table_car;	//device表
    mongoc_collection_t *mongodb_table_carinpark;	//device表
 //   mongodb_client = mongoc_client_new(str_con);
    mongodb_table_channel = mongoc_client_get_collection(mongodb_client,"boondb","channel");   //channel表
    mongodb_table_car = mongoc_client_get_collection(mongodb_client,"boondb","car");   //device表
    mongodb_table_carinpark = mongoc_client_get_collection(mongodb_client,"boondb","carinpark");   //device表

    char res[24];
    char name[128];
    char car_num[24];
    char park_full[24] = {0};
    mongoc_cursor_t *cursor;
    bson_t *query;
    bson_t result;
    const char *tmp;
    const bson_t *tmp1;
    bson_iter_t iter;
    bson_error_t error;
    unsigned int length;
    int chewei_count = 0;
    int carinpark_count = 0;

    char park_id[256];
    char time_now[64];
    time_t tm;
    time_printf(time(&tm),time_now);


    memset(park_id,0,256);
    /*************************根据车牌号查询车位数和车主姓名*******************/
    query = bson_new();


    BSON_APPEND_UTF8(query, "car_plate_id", plate); //查询条件

    cursor = mongoc_collection_find (mongodb_table_car, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);

        fprintf(fp_log,"%s##mongodb_validate_chewei_num 失败\n",time_now);
        return -1;
    }
    while(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor)) //得到car表的每一条记录
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "car_place_num")) //得到车位数量
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(car_num,0,24);
                memcpy(car_num,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "car_name")) //得到车主姓名
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(name,0,128);
                memcpy(name,tmp,length);
            }
            bson_destroy(&result);
        }
    }
    bson_destroy(query);

    mongoc_cursor_destroy(cursor);

    /******************************end*****************************/

    chewei_count = atoi(car_num);


    query = bson_new();

    BSON_APPEND_UTF8(query, "channel_ip",host_ip);

    cursor = mongoc_collection_find(mongodb_table_channel, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);

        fprintf(fp_log,"%s##mongodb_send_bgui_start 失败\n",time_now);
        return -1;
    }
    while(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor))
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "channel_park_id"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memcpy(park_id,tmp,length);
            }
            bson_destroy(&result);
        }
    }
    bson_destroy(query);

    mongoc_cursor_destroy(cursor);



    query = bson_new();


    BSON_APPEND_UTF8(query, "carinpark_people_name", name); //查询条件
    BSON_APPEND_UTF8(query, "carinpark_park_id", park_id); //查询条件

    carinpark_count = mongoc_collection_count (mongodb_table_carinpark, MONGOC_QUERY_NONE, query, 0, 0, NULL, &error); //根据车主姓名到carinpark里查询，该车主有几辆在场车
    bson_destroy(query);


    if(chewei_count <= carinpark_count) //车位数量小于等于在场车数量
    {
        *auth = false; //把已授权改为未授权
        fprintf(fp_log,"%s##车位数验证 %s车位数为%d 在场车数量为%d,车位数超限\n",time_now,name,chewei_count,carinpark_count);
    }
    else
    {
        fprintf(fp_log,"%s##车位数验证 %s车位数为%d 在场车数量为%d,车位数正常\n",time_now,name,chewei_count,carinpark_count);
    }
    mongoc_collection_destroy(mongodb_table_channel);
    mongoc_collection_destroy(mongodb_table_car);
    mongoc_collection_destroy(mongodb_table_carinpark);
//    mongoc_client_destroy(mongodb_client);
    return 0;
}
/******************************end***********************************************************/

/******************************车位数验证******************************************************/
int mongodb_validate_chewei_num_out(char *plate,bool *auth)
{
    snprintf(log_buf_, sizeof(log_buf_)-1, "[%s] 出口车位数验证 line[%d]", __FUNCTION__, __LINE__);
    writelog(log_buf_);

    // mongoc_client_t *mongodb_client;
    mongoc_collection_t *mongodb_table_channel;	//channel表
    mongoc_collection_t *mongodb_table_car;	//device表
    mongoc_collection_t *mongodb_table_carinpark;	//device表
    //   mongodb_client = mongoc_client_new(str_con);
    mongodb_table_channel = mongoc_client_get_collection(mongodb_client,"boondb","channel");   //channel表
    mongodb_table_car = mongoc_client_get_collection(mongodb_client,"boondb","car");   //device表
    mongodb_table_carinpark = mongoc_client_get_collection(mongodb_client,"boondb","carinpark");   //device表

    char res[24];
    char name[128];
    char car_num[24];
    char park_full[24] = {0};
    mongoc_cursor_t *cursor;
    bson_t *query;
    bson_t result;
    const char *tmp;
    const bson_t *tmp1;
    bson_iter_t iter;
    bson_error_t error;
    unsigned int length;
    int chewei_count = 0;
    int carinpark_count = 0;

    char park_id[256];
    char time_now[64];
    time_t tm;
    time_printf(time(&tm),time_now);


    memset(park_id,0,256);
    /*************************根据车牌号查询车位数和车主姓名*******************/
    query = bson_new();


    BSON_APPEND_UTF8(query, "car_plate_id", plate); //查询条件

    cursor = mongoc_collection_find (mongodb_table_car, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);

        fprintf(fp_log,"%s##mongodb_validate_chewei_num 失败\n",time_now);
        return -1;
    }
    while(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor)) //得到car表的每一条记录
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "car_place_num")) //得到车位数量
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(car_num,0,24);
                memcpy(car_num,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "car_name")) //得到车主姓名
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(name,0,128);
                memcpy(name,tmp,length);
            }
            bson_destroy(&result);
        }
    }
    bson_destroy(query);

    mongoc_cursor_destroy(cursor);

    /******************************end*****************************/

    chewei_count = atoi(car_num);


    query = bson_new();

    BSON_APPEND_UTF8(query, "channel_ip",host_ip);

    cursor = mongoc_collection_find(mongodb_table_channel, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);

        fprintf(fp_log,"%s##mongodb_send_bgui_start 失败\n",time_now);
        return -1;
    }
    while(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor))
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "channel_park_id"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memcpy(park_id,tmp,length);
            }
            bson_destroy(&result);
        }
    }
    bson_destroy(query);

    mongoc_cursor_destroy(cursor);



    query = bson_new();


    BSON_APPEND_UTF8(query, "carinpark_people_name", name); //查询条件
    BSON_APPEND_UTF8(query, "carinpark_park_id", park_id); //查询条件

    carinpark_count = mongoc_collection_count (mongodb_table_carinpark, MONGOC_QUERY_NONE, query, 0, 0, NULL, &error); //根据车主姓名到carinpark里查询，该车主有几辆在场车
    bson_destroy(query);


    if(chewei_count < carinpark_count) //车位数量小于等于在场车数量
    {
        *auth = false; //把已授权改为未授权
        fprintf(fp_log,"%s##车位数验证 %s车位数为%d 在场车数量为%d,车位数超限\n",time_now,name,chewei_count,carinpark_count);
    }
    else
    {
        fprintf(fp_log,"%s##车位数验证 %s车位数为%d 在场车数量为%d,车位数正常\n",time_now,name,chewei_count,carinpark_count);
    }
    mongoc_collection_destroy(mongodb_table_channel);
    mongoc_collection_destroy(mongodb_table_car);
    mongoc_collection_destroy(mongodb_table_carinpark);
//    mongoc_client_destroy(mongodb_client);
    return 0;
}
/******************************end***********************************************************/


/******************************月租车，储值车，储时车过期验证******************************************************/
/**
 * @brief:  月租车，储值车，储时车, 计次车　过期验证
 * @param flag
 * @param id
 * @param plate
 * @param type
 * @param auth
 * @param remain_day
 * @param remain_hour
 * @param remain_money
 * @param _out_remain_jici , 输出剩余计数次数
 * @param charge_rule
 * @return
 * @attention：　
 *          关于洗车，返回的权限是否过期，和计次次数
 */
int mongodb_validate_guoqi(char *flag,char *id,char *plate,char *type,bool *auth,int *remain_day,int *remain_hour,int * remain_money,char * charge_rule)
{
    //   mongoc_client_t *mongodb_client;
    mongoc_collection_t *mongodb_table_car;	//channel表
    mongoc_collection_t *mongodb_table_chargerule;	//device表
//    mongodb_client = mongoc_client_new(str_con);
    mongodb_table_car = mongoc_client_get_collection(mongodb_client,"boondb","car");   //channel表
    mongodb_table_chargerule = mongoc_client_get_collection(mongodb_client,"boondb","chargerule");   //device表

    char res[24];
    char charge_name[256];
    //char yuezuguoqi[24];

    mongoc_cursor_t *cursor;
    bson_t *query;
    bson_t result;
    const char *tmp;
    const bson_t *tmp1;
    bson_iter_t iter;
    bson_error_t error;
    unsigned int length;
    int charge_id = 0;
    int carinpark_count = 0;
      char yuezuguoqi[24];

    char time_now[64];
    char time_stop[64];
    long mil_now = 0;
    long mil_stop = 0;
    time_t tm;
    time_printf(time(&tm),time_now);
    memset(remain_car_time,0,128);
    memset(time_stop,0,64);
    memset(yuezuguoqi,0,24);

    //根据车牌号得到车辆类型
    if(mongodb_query_cartype(plate,type) < 0)
    {
        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 获取车牌[%s]的车辆类型[%s]失败, 验证授权失败, 直接返回! line[%d]", __FUNCTION__, plate, type, __LINE__);
        writelog(log_buf_);
        return -1;
    }

    /*********************根据通道id号和车辆类型到chargerule表查询收费方案***************/
    query = bson_new();

    BSON_APPEND_UTF8(query, "chargerule_park_id", id); //查询条件
    BSON_APPEND_UTF8(query, "chargerule_car_type", type); //查询条件

    cursor = mongoc_collection_find (mongodb_table_chargerule, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
    if(mongoc_cursor_error(cursor,&error))
    {
        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 查询chargerule表 chargerule_park_id[%s] chargerule_car_type[%s]失败 line[%d]", __FUNCTION__, id, type, __LINE__);
        writelog(log_buf_);

        bson_destroy(query);
        mongoc_cursor_destroy(cursor);

        fprintf(fp_log,"%s##mongodb_validate_guoqi 失败\n",time_now);
        return -1;
    }

    if(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor)) //得到car表的每一条记录
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "chargerule_name_id")) //得到车牌号
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(charge_name,0,256);
                memcpy(charge_name,tmp,length);

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 查询chargerule表 收费规则charge_name[%s] line[%d]", __FUNCTION__, charge_name, __LINE__);
                writelog(log_buf_);
            }

            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "chargerule_yuezuche_time")) //得到车牌号
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(yuezuguoqi,0,24);
                memcpy(yuezuguoqi,tmp,length);

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 查询chargerule表 月租过期 chargerule_yuezuche_time[%s] line[%d]", __FUNCTION__, yuezuguoqi, __LINE__);
                writelog(log_buf_);
            }
            bson_destroy(&result);
        }
        else
        {
            snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 查询chargerule表 没有找到 通道[%s] 车辆类型[%s]对应的收费规则 line[%d]", __FUNCTION__, id, type, __LINE__);
            writelog(log_buf_);

            bson_destroy(query);

            mongoc_cursor_destroy(cursor);
            fprintf(fp_log,"%s##通道%s 车辆类型%s 未找到对应的收费规则\n",time_now,id,type);
            return -1;
        }
    }
    bson_destroy(query);

    mongoc_cursor_destroy(cursor);
    /******************************end******************************************/
	if(strlen(yuezuguoqi) <= 0)
	{
		strcpy(yuezuguoqi,"10");
	}
	
    memset(charge_rule,0,256);
    memcpy(charge_rule,charge_name,strlen(charge_name));
    // 吉化大厦加入员工车辆 定制 20180411
    if(strcmp(type,"临时车") == 0 || strcmp(type,"员工车辆") == 0) //临时车不验证
    {
        snprintf(log_buf_, sizeof(log_buf_), "[%s] 车辆类型为[%s] 收费规则[%s] 直接返回0 reuturn 0! line[%d]", __FUNCTION__, type, charge_name, __LINE__);
        writelog(log_buf_);

        mongoc_collection_destroy(mongodb_table_car);
        mongoc_collection_destroy(mongodb_table_chargerule);
     //   mongoc_client_destroy(mongodb_client);
        return 0;
    }
    fprintf(fp_log,"%s##开始验证授权，车辆类型为%s,收费规则为:%s\n",time_now,type,charge_name);

    snprintf(log_buf_, sizeof(log_buf_), "[%s] 开始验证授权 车辆类型为[%s] 收费规则为[%s] line[%d]", __FUNCTION__, type, charge_name, __LINE__);
    writelog(log_buf_);

    if(strcmp(charge_rule,"指定时间免费") == 0) //租期车
    {
        char stop_time[24];
        query = bson_new();

        /**********根据车牌号到car表查询租期结束时间***********/
        BSON_APPEND_UTF8(query, "car_plate_id", plate); //查询条件

        cursor = mongoc_collection_find (mongodb_table_car, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
        if(mongoc_cursor_error(cursor,&error))
        {
            snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 查询car表失败! plate[%s] return -1. line[%d]", __FUNCTION__, plate, __LINE__);
            writelog(log_buf_);

            bson_destroy(query);
            mongoc_cursor_destroy(cursor);

            fprintf(fp_log,"%s##mongodb_validate_guoqi 失败\n",time_now);
            return -1;
        }
        if(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor)) //得到car表的每一条记录
        {
            if (mongoc_cursor_next (cursor, &tmp1))
            {
                bson_copy_to(tmp1,&result); //得到一条完整的记录
                if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "car_stop_time")) //得到车牌号
                {
                    tmp = bson_iter_utf8(&iter,&length);
                    memset(stop_time,0,24);
                    memcpy(stop_time,tmp,length);

                    snprintf(log_buf_, sizeof(log_buf_), "[%s] 查找car表 找到车牌plate[%s]的过期时间stop_time[%s] line[%d]", __FUNCTION__, plate, stop_time, __LINE__);
                    writelog(log_buf_);
                }
                bson_destroy(&result);
            }
        }
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);

        if(strlen(stop_time) < 6)
        {
            snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 没有找到车牌plate[%s]的租期结束时间 stop_time[%s] return -1. line[%d]", __FUNCTION__, plate, stop_time, __LINE__);
            writelog(log_buf_);

            mongoc_collection_destroy(mongodb_table_car);
            mongoc_collection_destroy(mongodb_table_chargerule);
        //    mongoc_client_destroy(mongodb_client);
            fprintf(fp_log,"%s##验证租期，未找到租期结束时间\n",time_now);
            return -1;
        }

        fprintf(fp_log,"%s##租期结束时间为%s\n",time_now,stop_time);
        /***********************end**********************/
        mil_now = get_tick(time_now); //得到当前时间的秒数
        mil_stop = get_tick(stop_time);	 //得到结束时间的秒数

        if(mil_now < mil_stop) //秒数比较
        {
            *remain_day = ((mil_stop - mil_now)/3600)/24;
            *remain_hour = ((mil_stop - mil_now)/3600)%24;
            fprintf(fp_log,"%s##验证租期，结束时间为%s 当前时间为%s,未过期。还有%d天%d小时过期\n",time_now,stop_time,time_now,*remain_day,*remain_hour);
            if(*remain_day <= atoi(yuezuguoqi))
            {
                sprintf(remain_car_time,"剩余时间%d天",*remain_day);
            }
            if(strcmp(flag,"入口") == 0)
            {
                in_guoqi_flag = true; //入口未过期
            }
            else
            {
                out_guoqi_flag = true; //入口未过期
            }

            snprintf(log_buf_, sizeof(log_buf_), "[%s] 验证租期 车牌[%s]的 当前时间为[%s] 结束时间为[%s] 未过期. 还有[%d]天[%d]小时过期 line[%d]",
                     __FUNCTION__, plate, time_now, stop_time, *remain_day, *remain_hour, __LINE__);
            writelog(log_buf_);
        }
        else
        {
            fprintf(fp_log,"%s##验证租期，结束时间为%s 当前时间为%s,已过期。\n",time_now,stop_time,time_now);
            strcpy(remain_car_time,"已过期");
            *auth = false; //把已授权改为未授权
            if(strcmp(flag,"入口") == 0)
            {
                in_guoqi_flag = false; //入口过期
            }
            else
            {
                out_guoqi_flag = false; //出口过期
            }

            snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 验证租期 已过期 车牌[%s]的 当前时间为[%s] 结束时间为[%s] line[%d]",
                     __FUNCTION__, plate, time_now, stop_time, __LINE__);
            writelog(log_buf_);
        }
    }
    else if(strcmp(charge_rule,"储时，剩余时间不足，按小时收费") == 0) //储时车
    {
        char remain_time[24];
        query = bson_new();

        /**********根据车牌号到car表查询剩余时间***********/
        BSON_APPEND_UTF8(query, "car_plate_id", plate); //查询条件

        cursor = mongoc_collection_find (mongodb_table_car, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
        if(mongoc_cursor_error(cursor,&error))
        {
            bson_destroy(query);

            mongoc_cursor_destroy(cursor);

            fprintf(fp_log,"%s##mongodb_validate_guoqi 失败\n",time_now);
            return -1;
        }
        while(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor)) //得到car表的每一条记录
        {
            if (mongoc_cursor_next (cursor, &tmp1))
            {
                bson_copy_to(tmp1,&result); //得到一条完整的记录
                if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "car_remain_time")) //得到车牌号
                {
                    tmp = bson_iter_utf8(&iter,&length);
                    memset(remain_time,0,24);
                    memcpy(remain_time,tmp,length);
                }
                bson_destroy(&result);
            }
        }
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);

        /***********************end**********************/

        if(atoi(remain_time) <= 0) //剩余时间为0
        {
            fprintf(fp_log,"%s##验证储时，剩余时间为0小时,时间不足。\n",time_now);
            *auth = false;
            if(strcmp(flag,"入口") == 0)
            {
                in_guoqi_flag = false; //入口过期
            }
            else
            {
                out_guoqi_flag = false; //出口过期
            }
        }
        else
        {
            if(strcmp(flag,"入口") == 0)
            {
                in_guoqi_flag = true; //入口未过期
            }
            else
            {
                out_guoqi_flag = true; //入口未过期
            }
            fprintf(fp_log,"%s##验证储时，剩余时间为%d小时。\n",time_now,atoi(remain_time));
        }
    }
    else if(strcmp(charge_rule,"储值，剩余金额不足，按小时收费") == 0) //储值车
    {
        char remain_money[24];
        query = bson_new();

        /**********根据车牌号到car表查询剩余金额***********/
        BSON_APPEND_UTF8(query, "car_plate_id", plate); //查询条件

        cursor = mongoc_collection_find (mongodb_table_car, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
        if(mongoc_cursor_error(cursor,&error))
        {
            bson_destroy(query);

            mongoc_cursor_destroy(cursor);

            fprintf(fp_log,"%s##mongodb_validate_guoqi 失败\n",time_now);
            return -1;
        }
        while(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor)) //得到car表的每一条记录
        {
            if (mongoc_cursor_next (cursor, &tmp1))
            {
                bson_copy_to(tmp1,&result); //得到一条完整的记录
                if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "car_remain_money")) //得到车牌号
                {
                    tmp = bson_iter_utf8(&iter,&length);
                    memset(remain_money,0,24);
                    memcpy(remain_money,tmp,length);
                }
                bson_destroy(&result);
            }
        }
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);

        /***********************end**********************/

        if(atoi(remain_money) <= 0) //剩余金额不足
        {
            if(strcmp(flag,"入口") == 0)
            {
                in_guoqi_flag = false; //入口过期
            }
            else
            {
                out_guoqi_flag = false; //出口过期
            }
            fprintf(fp_log,"%s##验证储值，剩余金额为0元,余额不足。\n",time_now);
            *auth = false;
        }
        else
        {
            if(strcmp(flag,"入口") == 0)
            {
                in_guoqi_flag = true; //入口未过期
            }
            else
            {
                out_guoqi_flag = true; //入口未过期
            }
            fprintf(fp_log,"%s##验证储值，剩余金额为%d元。\n",time_now,atoi(remain_money));
        }
    }else {
        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 严重错误! 没有找到收费方案charge_rule[%s] line[%d]", __FUNCTION__, charge_rule, __LINE__);
        writelog(log_buf_);
    }

    mongoc_collection_destroy(mongodb_table_car);
    mongoc_collection_destroy(mongodb_table_chargerule);
  //  mongoc_client_destroy(mongodb_client);

    return 0;
}
/******************************end***********************************************************/
/******************************白天按小时收费，晚上按次收费***********************************************/
int mongodb_day_hour_night_ci(char *id,char *car_type,char *in_time,char *out_time)
{
  //  mongoc_client_t *mongodb_client;

    mongoc_collection_t *mongodb_table_chargerule;	//device表
  //  mongodb_client = mongoc_client_new(str_con);

    mongodb_table_chargerule = mongoc_client_get_collection(mongodb_client,"boondb","chargerule");   //device表

    char rate[24];

    char uninttime[24];
    char ci[24];
    char day_start_time[24];
    char night_start_time[24];
    char day_free_time[24];
    char night_free_time[24];
    char append_free_time[24] = {0};

    mongoc_cursor_t *cursor;
    bson_t *query;
    bson_t result;
    const char *tmp;
    const bson_t *tmp1;
    bson_iter_t iter;
    bson_error_t error;
    unsigned int length;
    char tmp_in[24];
    char tmp_out[24];

    char time_now[64];
    time_t tm;
    time_printf(time(&tm),time_now);

    /******************************查询白天开始时间，夜间开始时间，白天收费费率和单位时间，白天免费时间，夜间免费时间***********************************************/
    query = bson_new();

    BSON_APPEND_UTF8(query, "chargerule_park_id",id); //查询条件
    BSON_APPEND_UTF8(query, "chargerule_car_type",car_type); //查询条件
    cursor = mongoc_collection_find (mongodb_table_chargerule, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);

        fprintf(fp_log,"%s##mongodb_day_hour_night_ci 失败\n",time_now);
        return -1;
    }
    if(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor))
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "chargerule_day_start_time"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(day_start_time,0,24);
                memcpy(day_start_time,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "chargerule_night_start_time"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(night_start_time,0,24);
                memcpy(night_start_time,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "chargerule_day_first_period_rate"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(rate,0,24);
                memcpy(rate,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "chargerule_night_first_period_rate"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(ci,0,24);
                memcpy(ci,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "chargerule_day_first_period_uninttime"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(uninttime,0,24);
                memcpy(uninttime,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "chargerule_day_free_period_duration"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(day_free_time,0,24);
                memcpy(day_free_time,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "chargerule_night_free_period_duration"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(night_free_time,0,24);
                memcpy(night_free_time,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "chargerule_append_free_time"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(append_free_time,0,24);
                memcpy(append_free_time,tmp,length);
            }
            bson_destroy(&result);
        }
    }
    bson_destroy(query);

    mongoc_cursor_destroy(cursor);
    mongoc_collection_destroy(mongodb_table_chargerule);
 //   mongoc_client_destroy(mongodb_client);

    fprintf(fp_log,"%s##白天开始时间为%s,夜间开始时间为%s,白天费率为%s元/%s分钟,夜晚费率为%s元/次，白天免费时间为%s,夜间免费时间为%s\n",time_now,day_start_time,night_start_time,rate,uninttime,ci,day_free_time,night_free_time);

    /**************************************************end******************************************/
    int mil_now = get_tick(out_time); //得到当前时间的秒数

    int mil_in = get_tick(in_time);	 //得到进入时间的秒数

    std::string in(in_time);
    std::string out(out_time);
    mil_now = mil_now - 60;
    if(mil_in >= mil_now) mil_in = mil_now - 10;
    memset(tmp_in,0,24);
    sprintf(tmp_in,"%s 00:00:00",in.substr(0,10).c_str()); //获取进入时间的凌晨时间
    memset(tmp_out,0,24);
    sprintf(tmp_out,"%s 00:00:00",out.substr(0,10).c_str()); //获取离开时间的凌晨时间

    int mil_tmp_in = mil_in -  get_tick(tmp_in); //化为24小时以内 得到进入时间
    int mil_tmp_out =  mil_now - get_tick(tmp_out); //化为24小时以内 得到离开时间

    if(mil_tmp_in/60 >= atoi(day_start_time) && mil_tmp_in/60 <= atoi(night_start_time)) //白天
    {
        if(mil_now/60 - mil_in/60 < (atoi(day_free_time) + 1)) //停留时长小于免费时间
        {
            return 0;
        }
        if(strcmp(append_free_time,"是") == 0)		//判断是否追加免费时间
        {
            mil_in += 60 * atoi(day_free_time);
        }
        else
        {
            fprintf(fp_log,"不追加免费时长");
        }
    }
    else //夜间
    {
        if(mil_now/60 - mil_in/60 < (atoi(night_free_time) + 1)) //停留时长小于免费时间
        {
            return 0;
        }
        if(strcmp(append_free_time,"是") == 0)		//判断是否追加免费时间
        {
            mil_in += 60 * atoi(day_free_time);
        }
        else
        {
            fprintf(fp_log,"不追加免费时长");
        }
    }

    if(mil_now - mil_in >= 86400) //停留时长大于24小时
    {
        int days = floor((mil_now - mil_in)/86400); //停留整天数

        int days_fee = days*((atoi(night_start_time) - atoi(day_start_time))/atoi(uninttime)*atoi(rate) + atoi(ci)); //停留整天数的费用

        if(mil_tmp_in < mil_tmp_out) //化为24小时，进入时间在离开时间之前
        {
            if(mil_tmp_in/60 < atoi(day_start_time) && mil_tmp_out/60  <= atoi(day_start_time)) //0点到8点进  0点到8点出
                return atoi(ci)+days_fee;
            else if(mil_tmp_in/60 < atoi(day_start_time) && mil_tmp_out/60  >atoi(day_start_time)&& mil_tmp_out/60  <= atoi(night_start_time)) //0点到8点进  8点到20点出
                return atoi(ci) + ((int)floor((mil_tmp_out/60 - atoi(day_start_time))/atoi(uninttime)) + 1)*atoi(rate) + days_fee;
            else if(mil_tmp_in/60 < atoi(day_start_time) && mil_tmp_out/60  > atoi(night_start_time))  //0点到8点进  20点到24点出
                return atoi(ci)*2 +  ((int)floor((atoi(night_start_time) - atoi(day_start_time))/atoi(uninttime)) + 1)*atoi(rate) + days_fee;
            else if(mil_tmp_in/60 >= atoi(day_start_time) && mil_tmp_in/60 < atoi(night_start_time) && mil_tmp_out/60  <= atoi(night_start_time)) //8点到20点进  8点到20点出
                return ((int)floor((mil_tmp_out/60 - mil_tmp_in/60)/atoi(uninttime)) + 1)*atoi(rate) + days_fee;
            else if(mil_tmp_in/60 >= atoi(day_start_time) && mil_tmp_in/60 < atoi(night_start_time) && mil_tmp_out/60  > atoi(night_start_time)) //8点到20点进  20点到24点出
                return  atoi(ci) + ((int)floor((atoi(night_start_time) - mil_tmp_in/60)/atoi(uninttime)) + 1)*atoi(rate) + days_fee;
            else  //20点到24点进  20点到24点出
                return  atoi(ci) + days_fee;
        }
        else //化为24小时，进入时间在离开时间之后
        {
            if(mil_tmp_in/60 <= atoi(day_start_time)) //0点到8点进  0点到8点出
                return atoi(ci)*2 + ((int)floor((atoi(night_start_time) - atoi(day_start_time))/atoi(uninttime)) + 1 )*atoi(rate) + days_fee;
            else if(mil_tmp_in/60 > atoi(day_start_time) && mil_tmp_in/60 <= atoi(night_start_time) && mil_tmp_out/60 <= atoi(day_start_time)) //8点到20点进  第二天的0点到8点出
                return atoi(ci) + ((int)floor((atoi(night_start_time) - mil_tmp_in/60)/atoi(uninttime) ) +1)*atoi(rate)+ days_fee;
            else if(mil_tmp_in/60 > atoi(day_start_time) && mil_tmp_in/60 <= atoi(night_start_time) && mil_tmp_out/60 < atoi(night_start_time)) //8点到20点进  第二天的8点到20点出
                return atoi(ci) + ((int)floor((atoi(night_start_time) - mil_tmp_in/60)/atoi(uninttime)) + 1)*atoi(rate) + ((int)floor((mil_tmp_out/60 - atoi(day_start_time))/atoi(uninttime)) + 1)*atoi(rate) + days_fee;
            else if(mil_tmp_in/60 >  atoi(night_start_time) && mil_tmp_out/60 <= atoi(day_start_time)) //20点到24点进  第二天的0点到8点出
                return atoi(ci)  + days_fee;
            else if(mil_tmp_in/60 >  atoi(night_start_time) && mil_tmp_out/60 > atoi(day_start_time) && mil_tmp_out/60 <= atoi(night_start_time)) //20点到24点进  第二天的8点到20点出
                return atoi(ci)  + ((int)floor((mil_tmp_out/60 - atoi(day_start_time))/atoi(uninttime)) + 1)*atoi(rate) + days_fee;
            else if(mil_tmp_in/60 >  atoi(night_start_time) && mil_tmp_out/60 >  atoi(night_start_time)) //20点到24点进  第二天的20点到24点出
                return atoi(ci)*2  + ((int)floor((atoi(night_start_time) - atoi(day_start_time))/atoi(uninttime)) + 1)*atoi(rate) + days_fee;
        }

    }
    else ////停留时长小于24小时
    {
        int days_fee = 0;
        if(mil_tmp_in < mil_tmp_out) //化为24小时，进入时间在离开时间之前
        {
            if(mil_tmp_in/60 < atoi(day_start_time) && mil_tmp_out/60  <= atoi(day_start_time)) //0点到8点进  0点到8点出
                return atoi(ci)+days_fee;
            else if(mil_tmp_in/60 < atoi(day_start_time) && mil_tmp_out/60  >atoi(day_start_time)&& mil_tmp_out/60  <= atoi(night_start_time)) //0点到8点进  8点到20点出
                return atoi(ci) + ((int)floor((mil_tmp_out/60 - atoi(day_start_time))/atoi(uninttime)) + 1)*atoi(rate) + days_fee;
            else if(mil_tmp_in/60 < atoi(day_start_time) && mil_tmp_out/60  > atoi(night_start_time))  //0点到8点进  20点到24点出
                return atoi(ci)*2 +  ((int)floor((atoi(night_start_time) - atoi(day_start_time))/atoi(uninttime)) + 1)*atoi(rate) + days_fee;
            else if(mil_tmp_in/60 >= atoi(day_start_time) && mil_tmp_in/60 < atoi(night_start_time) && mil_tmp_out/60  <= atoi(night_start_time)) //8点到20点进  8点到20点出
                return ((int)floor((mil_tmp_out/60 - mil_tmp_in/60)/atoi(uninttime)) + 1)*atoi(rate) + days_fee;
            else if(mil_tmp_in/60 >= atoi(day_start_time) && mil_tmp_in/60 < atoi(night_start_time) && mil_tmp_out/60  > atoi(night_start_time)) //8点到20点进  20点到24点出
                return  atoi(ci) + ((int)floor((atoi(night_start_time) - mil_tmp_in/60)/atoi(uninttime)) + 1)*atoi(rate) + days_fee;
            else  //20点到24点进  20点到24点出
                return  atoi(ci) + days_fee;
        }
        else //化为24小时，进入时间在离开时间之后
        {
            if(mil_tmp_in/60 <= atoi(day_start_time)) //0点到8点进  0点到8点出
                return atoi(ci)*2 + ((int)floor((atoi(night_start_time) - atoi(day_start_time))/atoi(uninttime)) + 1 )*atoi(rate) + days_fee;
            else if(mil_tmp_in/60 > atoi(day_start_time) && mil_tmp_in/60 <= atoi(night_start_time) && mil_tmp_out/60 <= atoi(day_start_time)) //8点到20点进  第二天的0点到8点出
                return atoi(ci) + ((int)floor((atoi(night_start_time) - mil_tmp_in/60)/atoi(uninttime) ) +1)*atoi(rate)+ days_fee;
            else if(mil_tmp_in/60 > atoi(day_start_time) && mil_tmp_in/60 <= atoi(night_start_time) && mil_tmp_out/60 < atoi(night_start_time)) //8点到20点进  第二天的8点到20点出
                return atoi(ci) + ((int)floor((atoi(night_start_time) - mil_tmp_in/60)/atoi(uninttime)) + 1)*atoi(rate) + ((int)floor((mil_tmp_out/60 - atoi(day_start_time))/atoi(uninttime)) + 1)*atoi(rate) + days_fee;
            else if(mil_tmp_in/60 >  atoi(night_start_time) && mil_tmp_out/60 <= atoi(day_start_time)) //20点到24点进  第二天的0点到8点出
                return atoi(ci)  + days_fee;
            else if(mil_tmp_in/60 >  atoi(night_start_time) && mil_tmp_out/60 > atoi(day_start_time) && mil_tmp_out/60 <= atoi(night_start_time)) //20点到24点进  第二天的8点到20点出
                return atoi(ci)  + ((int)floor((mil_tmp_out/60 - atoi(day_start_time))/atoi(uninttime)) + 1)*atoi(rate) + days_fee;
            else if(mil_tmp_in/60 >  atoi(night_start_time) && mil_tmp_out/60 >  atoi(night_start_time)) //20点到24点进  第二天的20点到24点出
                return atoi(ci)*2  + ((int)floor((atoi(night_start_time) - atoi(day_start_time))/atoi(uninttime)) + 1)*atoi(rate) + days_fee;
        }
    }

}
/******************************济南2017收费规则***********************************************/
int mongodb_jinan_2017(char *id,char *plate,char *car_type,char *in_time,char *out_time)
{
    snprintf(log_buf_, sizeof(log_buf_), "[%s] 济南2017规则 车牌plate[%s] 车辆类型car_type[%s] 入场时间in_time[%s] 出场时间out_time[%s] line[%d]",
             __FUNCTION__, plate, car_type, in_time, out_time, __LINE__);
    writelog(log_buf_);

 //   mongoc_client_t *mongodb_client;

    mongoc_collection_t *mongodb_table_chargerule;	//device表
 //   mongodb_client = mongoc_client_new(str_con);

    mongodb_table_chargerule = mongoc_client_get_collection(mongodb_client,"boondb","chargerule");   //device表

    char rate[24];

    char uninttime[24];
    char uninttime_night[24];
    char rate_night[24];
    char day_start_time[24];
    char night_start_time[24];
    char day_free_time[24];
    char night_free_time[24];
    char night_max_cost[24];
    char cycle_max_cost[24] = {0};
    char append_free_time[24] = {0};
    char tonggang_free_period_duration[24] = {0};

    mongoc_cursor_t *cursor;
    bson_t *query;
    bson_t result;
    const char *tmp;
    const bson_t *tmp1;
    bson_iter_t iter;
    bson_error_t error;
    unsigned int length;
    char tmp_in[24];
    char tmp_out[24];

    char time_now[64];
    time_t tm;
    time_printf(time(&tm),time_now);

    /******************************查询白天开始时间，夜间开始时间，白天收费费率和单位时间，白天免费时间，夜间免费时间***********************************************/
    query = bson_new();

    /*
    snprintf(log_buf_, sizeof(log_buf_), "[%s] 准备查询chargerule表 查询条件 停车场chargerule_park_id[%s] 车辆类型chargerule_car_type[%s] line[%d]",
             __FUNCTION__, id, car_type, __LINE__);
    writelog(log_buf_);
    */

    BSON_APPEND_UTF8(query, "chargerule_park_id",id); //查询条件
    BSON_APPEND_UTF8(query, "chargerule_car_type",car_type); //查询条件
    cursor = mongoc_collection_find (mongodb_table_chargerule, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);

        fprintf(fp_log,"%s##mongodb_jinan_2017 失败\n",time_now);
        return -1;
    }
    if(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor))
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "chargerule_day_start_time"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(day_start_time,0,24);
                memcpy(day_start_time,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "chargerule_night_start_time"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(night_start_time,0,24);
                memcpy(night_start_time,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "chargerule_day_first_period_rate"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(rate,0,24);
                memcpy(rate,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "chargerule_night_first_period_rate"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(rate_night,0,24);
                memcpy(rate_night,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "chargerule_day_first_period_uninttime"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(uninttime,0,24);
                memcpy(uninttime,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "chargerule_night_first_period_uninttime"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(uninttime_night,0,24);
                memcpy(uninttime_night,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "chargerule_day_free_period_duration"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(day_free_time,0,24);
                memcpy(day_free_time,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "chargerule_night_free_period_duration"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(night_free_time,0,24);
                memcpy(night_free_time,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "chargerule_night_max_cost"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(night_max_cost,0,24);
                memcpy(night_max_cost,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "chargerule_cycle_max_cost"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(cycle_max_cost,0,24);
                memcpy(cycle_max_cost,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "chargerule_append_free_time"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(append_free_time,0,24);
                memcpy(append_free_time,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "chargerule_tonggang_free_period_duration"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(tonggang_free_period_duration,0,24);
                memcpy(tonggang_free_period_duration,tmp,length);
            }
            bson_destroy(&result);
        }
    }
    bson_destroy(query);

    mongoc_cursor_destroy(cursor);
    mongoc_collection_destroy(mongodb_table_chargerule);
  //  mongoc_client_destroy(mongodb_client);

    fprintf(fp_log,"%s##白天开始时间为%s,夜间开始时间为%s,白天费率为%s元/%s分钟,夜晚费率为%s元/%s分钟，白天免费时间为%s,夜间免费时间为%s，周期封顶金额为%s元，夜间封顶金额为%s元\n",time_now,
            day_start_time, night_start_time, rate, uninttime, rate_night, uninttime_night, day_free_time, night_free_time, cycle_max_cost, night_max_cost);
    fflush(fp_log);

    snprintf(log_buf_, sizeof(log_buf_), "[%s] 白天开始时间为[%s] 夜间开始时间为[%s] 白天费率为[%s]元/[%s]分钟 夜晚费率为[%s]元/[%s]分钟 白天免费时间为[%s] 夜间免费时间为[%s] 周期封顶金额为[%s]元 夜间封顶金额为[%s]元 同岗免费时间[%s] line[%d]", __FUNCTION__,
             day_start_time, night_start_time, rate, uninttime, rate_night, uninttime_night, day_free_time, night_free_time, cycle_max_cost, night_max_cost, tonggang_free_period_duration, __LINE__);
    writelog(log_buf_);

    int n_day_start_time = atoi(day_start_time);
    int n_night_start_time = atoi(night_start_time);

    int n_day_rate = atoi(rate);
    int n_night_rate = atoi(rate_night);

    int n_day_unittime = atoi(uninttime);
    int n_night_unittime = atoi(uninttime_night);

    int dayfeeCount = (n_night_start_time - n_day_start_time) / n_day_unittime;

    /*
    snprintf(log_buf_, sizeof(log_buf_), "[%s] dayfeeCount[%d] = (n_night_start_time[%d] - n_day_start_time[%d]) / n_day_unittime[%d] line[%d]", __FUNCTION__,
             dayfeeCount, n_night_start_time, n_day_start_time, n_day_unittime, __LINE__);
    writelog(log_buf_);
    */

    int nightfeeCount = ((24 * 60) - n_night_start_time + n_day_start_time) / n_night_unittime;

    int nightMaxCost = nightfeeCount * n_night_rate;

    /*
    snprintf(log_buf_, sizeof(log_buf_), "[%s] dayfeeCount[%d] = nightfeeCount[%d] * nightMaxCost[%d] line[%d]", __FUNCTION__, nightMaxCost, nightfeeCount, n_night_rate, __LINE__);
    writelog(log_buf_);
    */
    if(atoi(night_max_cost) == 0)
    {
        nightMaxCost = nightfeeCount * n_night_rate;
        /*
        snprintf(log_buf_, sizeof(log_buf_), "[%s] night_max_cost[%s] nightMaxCost[%d] line[%d]", __FUNCTION__, night_max_cost, nightMaxCost, __LINE__);
        writelog(log_buf_);
         */
    }
    else if(atoi(night_max_cost) <= nightMaxCost)
    {
        /*
        snprintf(log_buf_, sizeof(log_buf_), "[%s] night_max_cost[%s] <= nightMaxCost[%d] line[%d]", __FUNCTION__, night_max_cost, nightMaxCost, __LINE__);
        writelog(log_buf_);
        */
        nightMaxCost = atoi(night_max_cost);
    }
    else
    {
        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 未处理的逻辑 night_max_cost[%s] nightMaxCost[%d] line[%d]", __FUNCTION__, night_max_cost, nightMaxCost, __LINE__);
        writelog(log_buf_);
    }

    int alldayfee = dayfeeCount * n_day_rate + nightMaxCost;

    /*
    snprintf(log_buf_, sizeof(log_buf_), "[%s] alldayfee[%d] = dayfeeCount[%d] * n_day_rate[%d] + nightMaxCost[%d] line[%d]", __FUNCTION__,
             alldayfee, dayfeeCount, n_day_rate, nightMaxCost, __LINE__);
    writelog(log_buf_);
    */

    /*
    int nightMaxCost = nightfeeCount * n_night_rate;

    if(atoi(night_max_cost) == 0)
    {
        nightMaxCost = nightfeeCount * n_night_rate;
    }
    else if(atoi(night_max_cost) <= nightMaxCost)
    {
        nightMaxCost = atoi(night_max_cost);
    }
    else
    {

    }
     int alldayfee = dayfeeCount * n_day_rate + nightMaxCost;
    */

    if((atoi(night_max_cost) == 0) || (atoi(night_max_cost) > nightfeeCount * n_night_rate))
    {
        int tmp = nightMaxCost;
        sprintf(night_max_cost,"%d",tmp);

        /*
        snprintf(log_buf_, sizeof(log_buf_), "[%s] night_max_cost[%s] nightfeeCount[%d] n_night_rate[%d] line[%d]", __FUNCTION__, night_max_cost, nightfeeCount, n_night_rate, __LINE__);
        writelog(log_buf_);
         */
    }

    if((atoi(cycle_max_cost) == 0) || (atoi(cycle_max_cost) > alldayfee))
    {
        int tmp = alldayfee;
        sprintf(cycle_max_cost,"%d",tmp);

        /*
        snprintf(log_buf_, sizeof(log_buf_), "[%s] night_max_cost[%s] alldayfee[%d] line[%d]", __FUNCTION__, cycle_max_cost, alldayfee, __LINE__);
        writelog(log_buf_);
         */
    }

    /**************************************************end******************************************/
    int mil_now = get_tick(out_time); //得到当前时间的秒数

    int mil_in = get_tick(in_time);	 //得到进入时间的秒数
    mil_now = mil_now - 60;
    if(mil_in >= mil_now) mil_in = mil_now - 10;
    std::string in(in_time);
    std::string out(out_time);

    memset(tmp_in,0,24);
    sprintf(tmp_in,"%s 00:00:00",in.substr(0,10).c_str()); //获取进入时间的凌晨时间
    memset(tmp_out,0,24);
    sprintf(tmp_out,"%s 00:00:00",out.substr(0,10).c_str()); //获取离开时间的凌晨时间

    int mil_tmp_in = mil_in -  get_tick(tmp_in); //化为24小时以内 得到进入时间
    int mil_tmp_out =  mil_now - get_tick(tmp_out); //化为24小时以内 得到离开时间

    printf("---------tong gang jing chu %d,%s\n", atoi(tonggang_free_period_duration), plate);
    if((atoi(tonggang_free_period_duration) > 0) && (strcmp(plate,"yes") == 0))
    {
        memset(day_free_time,0,24);
        memcpy(day_free_time,tonggang_free_period_duration,24);

        memset(night_free_time,0,24);
        memcpy(night_free_time,tonggang_free_period_duration,24);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 同岗进出{该逻辑只有建大有} 其他停车场需要把白天免费时间配置和同岗进出时间一致!!! tonggang_free_period_duration[%s] plate[%s] 白天免费时间day_free_time[%s] 夜间免费时间night_free_time[%s] line[%d]", __FUNCTION__,
                 tonggang_free_period_duration, plate, day_free_time, night_free_time, __LINE__);
        writelog(log_buf_);

    }

    if(mil_tmp_in/60 >= atoi(day_start_time) && mil_tmp_in/60 <= atoi(night_start_time)) //白天
    {
        if(mil_now/60 - mil_in/60 < (atoi(day_free_time) + 1)) //停留时长小于免费时间
        {
            snprintf(log_buf_, sizeof(log_buf_), "[%s] 白天 停留时长小于免费时间 return 0! line[%d]", __FUNCTION__, __LINE__);
            writelog(log_buf_);

            return 0;
        }

        if(strcmp(append_free_time,"是") == 0)		//判断是否追加免费时间
        {
            snprintf(log_buf_, sizeof(log_buf_), "[%s] 白天 追加免费时间 line[%d]", __FUNCTION__, __LINE__);
            writelog(log_buf_);
        }
        else
        {
            mil_in += 60 * atoi(day_free_time);
            mil_tmp_in = mil_in -  get_tick(tmp_in);

            if(mil_tmp_in < 0)
                mil_tmp_in = 0;
            fprintf(fp_log,"不追加免费时长");

            snprintf(log_buf_, sizeof(log_buf_), "[%s] 白天 不追加免费时长 line[%d]", __FUNCTION__, __LINE__);
            writelog(log_buf_);
        }
    }
    else //夜间
    {
        if(mil_now/60 - mil_in/60 < (atoi(night_free_time) + 1)) //停留时长小于免费时间
        {
            snprintf(log_buf_, sizeof(log_buf_), "[%s] 夜间 停留时长小于免费时间 return 0! line[%d]", __FUNCTION__, __LINE__);
            writelog(log_buf_);

            return 0;
        }

        if(strcmp(append_free_time,"是") == 0)		//判断是否追加免费时间
        {
            snprintf(log_buf_, sizeof(log_buf_), "[%s] 夜间 未处理逻辑 否追加免费时间 line[%d]", __FUNCTION__, __LINE__);
            writelog(log_buf_);
        }
        else
        {
            mil_in += 60 * atoi(day_free_time);
            mil_tmp_in = mil_in -  get_tick(tmp_in);
            if(mil_tmp_in < 0) mil_tmp_in = 0;
            fprintf(fp_log,"不追加免费时长");

            snprintf(log_buf_, sizeof(log_buf_), "[%s] 夜间 不追加免费时长 line[%d]", __FUNCTION__, __LINE__);
            writelog(log_buf_);
        }
    }

    if(mil_now - mil_in >= 86400) //停留时长大于24小时
    {
        snprintf(log_buf_, sizeof(log_buf_), "[%s] 停留时长大于24小时 line[%d]", __FUNCTION__, __LINE__);
        writelog(log_buf_);

        int days = floor((mil_now - mil_in)/86400); //停留整天数
        //printf("day = %d\n",days);

        int days_fee = days*atoi(cycle_max_cost); //停留整天数的费用

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 停留时长大于24小时 days[%d] cycle_max_cost[%s] line[%d]", __FUNCTION__, days, cycle_max_cost, __LINE__);
        writelog(log_buf_);

        if(mil_tmp_in < mil_tmp_out) //化为24小时，进入时间在离开时间之前
        {
            if(mil_tmp_in/60 < atoi(day_start_time) && mil_tmp_out/60  <= atoi(day_start_time)) //0点到8点进  0点到8点出
            {
                int fee = ((int)floor((mil_tmp_out/60 - mil_tmp_in/60)/atoi(uninttime_night)) + 1)*atoi(rate_night);
                if(fee > atoi(night_max_cost))   fee = atoi(night_max_cost);

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 0点到8点进 0点到8点出! return fee[%d]+days_fee[%d] 进入时间在离开时间之前 line[%d]",
                         __FUNCTION__, fee, days_fee, __LINE__);
                writelog(log_buf_);

                return fee+days_fee;
            }
            else if(mil_tmp_in/60 < atoi(day_start_time) && mil_tmp_out/60  >atoi(day_start_time)&& mil_tmp_out/60  <= atoi(night_start_time)) //0点到8点进  8点到20点出
            {
                int fee = ((int)floor((atoi(day_start_time) - mil_tmp_in/60)/atoi(uninttime_night)) + 1)*atoi(rate_night);
                if(fee > atoi(night_max_cost))   fee = atoi(night_max_cost);
                fee = fee + ((int)floor((mil_tmp_out/60 - atoi(day_start_time))/atoi(uninttime)) + 1)*atoi(rate);
                if(fee > atoi(cycle_max_cost)) fee = atoi(cycle_max_cost);

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 0点到8点进 8点到20点出! return fee[%d]+days_fee[%d] 进入时间在离开时间之前 line[%d]",
                         __FUNCTION__, fee, days_fee, __LINE__);
                writelog(log_buf_);

                return fee+days_fee;
            }
            else if(mil_tmp_in/60 < atoi(day_start_time) && mil_tmp_out/60  > atoi(night_start_time))  //0点到8点进  20点到24点出
            {
                int fee = ((int)floor((atoi(day_start_time) - mil_tmp_in/60)/atoi(uninttime_night)) + 1)*atoi(rate_night);
                if(fee > atoi(night_max_cost))   fee = atoi(night_max_cost);
                fee = fee + ((int)floor((atoi(night_start_time) - atoi(day_start_time))/atoi(uninttime)) + 1)*atoi(rate);
                if(fee > atoi(cycle_max_cost)) fee = atoi(cycle_max_cost);
                int fee1 = ((int)floor((mil_tmp_out/60 - atoi(night_start_time))/atoi(uninttime_night)) + 1)*atoi(rate_night);
                if(fee1 > atoi(night_max_cost)) fee1 = atoi(night_max_cost);
                fee = fee + fee1;

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 0点到8点进 20点到24点出! return fee[%d]+days_fee[%d] 进入时间在离开时间之前 line[%d]",
                         __FUNCTION__, fee, days_fee, __LINE__);
                writelog(log_buf_);

                return fee+days_fee;
            }
            else if(mil_tmp_in/60 >= atoi(day_start_time) && mil_tmp_in/60 < atoi(night_start_time) && mil_tmp_out/60  <= atoi(night_start_time)) //8点到20点进  8点到20点出
            {
                int fee = ((int)floor((mil_tmp_out/60 - mil_tmp_in/60 )/atoi(uninttime)) + 1)*atoi(rate);
                if(fee > atoi(cycle_max_cost)) fee = atoi(cycle_max_cost);

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 8点到20点进 8点到20点出! return fee[%d]+days_fee[%d] 进入时间在离开时间之前 line[%d]",
                         __FUNCTION__, fee, days_fee, __LINE__);
                writelog(log_buf_);

                return fee+days_fee;
            }
            else if(mil_tmp_in/60 >= atoi(day_start_time) && mil_tmp_in/60 < atoi(night_start_time) && mil_tmp_out/60  > atoi(night_start_time)) //8点到20点进  20点到24点出
            {
                int fee = ((int)floor((atoi(night_start_time) - mil_tmp_in/60)/atoi(uninttime)) + 1)*atoi(rate);
                int fee1 =  ((int)floor((mil_tmp_out/60 - atoi(night_start_time))/atoi(uninttime_night)) + 1)*atoi(rate_night);
                if(fee1 > atoi(night_max_cost)) fee1 = atoi(night_max_cost);
                fee = fee + fee1;
                if(fee > atoi(cycle_max_cost)) fee = atoi(cycle_max_cost);

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 8点到20点进 20点到24点出! return fee[%d]+days_fee[%d] 进入时间在离开时间之前 line[%d]",
                         __FUNCTION__, fee, days_fee, __LINE__);
                writelog(log_buf_);

                return fee+days_fee;
            }
            else  //20点到24点进  20点到24点出
            {
                int fee = ((int)floor((mil_tmp_out/60- atoi(night_start_time))/atoi(uninttime_night)) + 1)*atoi(rate_night);
                if(fee > atoi(night_max_cost))   fee = atoi(night_max_cost);

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 20点到24点进 20点到24点出! return fee[%d]+days_fee[%d] 进入时间在离开时间之前 line[%d]",
                         __FUNCTION__, fee, days_fee, __LINE__);
                writelog(log_buf_);

                return  fee+days_fee;
            }
        }
        else //化为24小时，进入时间在离开时间之后
        {
            if(mil_tmp_in/60 <= atoi(day_start_time)) //0点到8点进  0点到8点出
            {
                int fee = ((int)floor((atoi(day_start_time)- mil_tmp_in/60)/atoi(uninttime_night)) + 1)*atoi(rate_night);
                if(fee > atoi(night_max_cost))   fee = atoi(night_max_cost);
                fee = fee + ((int)floor((atoi(night_start_time)- atoi(day_start_time))/atoi(uninttime)) + 1)*atoi(rate);
                if(fee > atoi(cycle_max_cost))   fee = atoi(cycle_max_cost);
                int fee1 = ((int)floor((24*60 - atoi(night_start_time) + mil_tmp_out/60 )/atoi(uninttime_night)) + 1)*atoi(rate_night);
                if(fee1 > atoi(night_max_cost)) fee1 = atoi(night_max_cost);
                fee = fee + fee1;

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 0点到8点进 0点到8点出! return fee[%d]+days_fee[%d] 进入时间在离开时间之后 line[%d]",
                         __FUNCTION__, fee, days_fee, __LINE__);
                writelog(log_buf_);

                return fee+days_fee;
            }
            else if(mil_tmp_in/60 > atoi(day_start_time) && mil_tmp_in/60 <= atoi(night_start_time) && mil_tmp_out/60 <= atoi(day_start_time)) //8点到20点进  第二天的0点到8点出
            {
                int fee = ((int)floor((atoi(night_start_time)- mil_tmp_in/60)/atoi(uninttime)) + 1)*atoi(rate);
		        int fee1 = ((int)floor((24*60 - atoi(night_start_time) + mil_tmp_out/60 )/atoi(uninttime_night)) + 1)*atoi(rate_night);
                if(fee1 > atoi(night_max_cost)) fee1 = atoi(night_max_cost);
                fee = fee + fee1;
                if(fee > atoi(cycle_max_cost))   fee = atoi(cycle_max_cost);

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 8点到20点进 第二天的0点到8点出! return fee[%d]+days_fee[%d] 进入时间在离开时间之后 line[%d]",
                         __FUNCTION__, fee, days_fee, __LINE__);
                writelog(log_buf_);

                return fee+days_fee;
            }
            else if(mil_tmp_in/60 > atoi(day_start_time) && mil_tmp_in/60 <= atoi(night_start_time) && mil_tmp_out/60 < atoi(night_start_time)) //8点到20点进  第二天的8点到20点出
            {
                int fee = ((int)floor((atoi(night_start_time)- mil_tmp_in/60)/atoi(uninttime)) + 1)*atoi(rate);
                fee = fee + atoi(night_max_cost);
                if(fee > atoi(cycle_max_cost))   fee = atoi(cycle_max_cost);
                int fee1 = ((int)floor((mil_tmp_out/60- atoi(day_start_time))/atoi(uninttime)) + 1)*atoi(rate);
                if(fee1 > atoi(cycle_max_cost))   fee1 = atoi(cycle_max_cost);
                fee = fee + fee1;

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 8点到20点进 第二天的8点到20点出! return fee[%d]+days_fee[%d] 进入时间在离开时间之后 line[%d]",
                         __FUNCTION__, fee, days_fee, __LINE__);
                writelog(log_buf_);

                return fee+days_fee;
            }
            else if(mil_tmp_in/60 >  atoi(night_start_time) && mil_tmp_out/60 <= atoi(day_start_time)) //20点到24点进  第二天的0点到8点出
            {
                int fee = ((int)floor((24*60 - mil_tmp_in/60 + mil_tmp_out/60)/atoi(uninttime_night)) + 1)*atoi(rate_night);
                if(fee > atoi(night_max_cost) ) fee = atoi(night_max_cost);

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 20点到24点进 第二天的0点到8点出! return fee[%d]+days_fee[%d] 进入时间在离开时间之后 line[%d]",
                         __FUNCTION__, fee, days_fee, __LINE__);
                writelog(log_buf_);

                return fee+days_fee;
            }
            else if(mil_tmp_in/60 >  atoi(night_start_time) && mil_tmp_out/60 > atoi(day_start_time) && mil_tmp_out/60 <= atoi(night_start_time)) //20点到24点进  第二天的8点到20点出
            {
                int fee = ((int)floor((mil_tmp_out/60 - atoi(day_start_time))/atoi(uninttime)) + 1)*atoi(rate) + atoi(night_max_cost);
                if(fee > atoi(cycle_max_cost))   fee = atoi(cycle_max_cost);

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 20点到24点进 第二天的8点到20点出! return fee[%d]+days_fee[%d] 进入时间在离开时间之后 line[%d]",
                         __FUNCTION__, fee, days_fee, __LINE__);
                writelog(log_buf_);

                return fee+days_fee;
            }
            else if(mil_tmp_in/60 >  atoi(night_start_time) && mil_tmp_out/60 >  atoi(night_start_time)) //20点到24点进  第二天的20点到24点出
            {
                int fee = ((int)floor((mil_tmp_out/60 - atoi(night_start_time))/atoi(uninttime_night)) + 1)*atoi(rate_night);
                fee = fee + atoi(cycle_max_cost);

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 20点到24点进 第二天的20点到24点出! return fee[%d]+days_fee[%d] 进入时间在离开时间之后 line[%d]",
                         __FUNCTION__, fee, days_fee, __LINE__);
                writelog(log_buf_);

                return fee+days_fee;
            }else{
                snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 未处理的逻辑 line[%d]", __FUNCTION__, __LINE__);
                writelog(log_buf_);
            }
        }

    }
    else //停留时长小于24小时
    {
        snprintf(log_buf_, sizeof(log_buf_), "[%s] 停留时长小于24小时 line[%d]", __FUNCTION__, __LINE__);
        writelog(log_buf_);

        int days_fee = 0;
        if(mil_tmp_in < mil_tmp_out) //化为24小时，进入时间在离开时间之前
        {
            if(mil_tmp_in/60 < atoi(day_start_time) && mil_tmp_out/60  <= atoi(day_start_time)) //0点到8点进  0点到8点出
            {
                int fee = ((int)floor((mil_tmp_out/60 - mil_tmp_in/60)/atoi(uninttime_night)) + 1)*atoi(rate_night);
                if(fee > atoi(night_max_cost))   fee = atoi(night_max_cost);

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 0点到8点进 0点到8点出! return fee[%d]+days_fee[%d] 进入时间在离开时间之前 line[%d]",
                         __FUNCTION__, fee, days_fee, __LINE__);
                writelog(log_buf_);

                return fee+days_fee;
            }
            else if(mil_tmp_in/60 < atoi(day_start_time) && mil_tmp_out/60  >atoi(day_start_time) && mil_tmp_out/60  <= atoi(night_start_time)) //0点到8点进  8点到20点出
            {
                int fee = ((int)floor((atoi(day_start_time) - mil_tmp_in/60)/atoi(uninttime_night)) + 1)*atoi(rate_night);
                if(fee > atoi(night_max_cost)) fee = atoi(night_max_cost);
                fee = fee + ((int)floor((mil_tmp_out/60 - atoi(day_start_time))/atoi(uninttime)) + 1)*atoi(rate);
                if(fee > atoi(cycle_max_cost)) fee = atoi(cycle_max_cost);

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 0点到8点进 8点到20点出! return fee[%d]+days_fee[%d] 进入时间在离开时间之前 line[%d]",
                         __FUNCTION__, fee, days_fee, __LINE__);
                writelog(log_buf_);

                return fee + days_fee;
            }
            else if(mil_tmp_in/60 < atoi(day_start_time) && mil_tmp_out/60  > atoi(night_start_time))  //0点到8点进  20点到24点出
            {
                int fee = ((int)floor((atoi(day_start_time) - mil_tmp_in/60)/atoi(uninttime_night)) + 1)*atoi(rate_night);
                if(fee > atoi(night_max_cost))   fee = atoi(night_max_cost);
                fee = fee + ((int)floor((atoi(night_start_time) - atoi(day_start_time))/atoi(uninttime)) + 1)*atoi(rate);
                if(fee > atoi(cycle_max_cost)) fee = atoi(cycle_max_cost);
                int fee1 = ((int)floor((mil_tmp_out/60 - atoi(night_start_time))/atoi(uninttime_night)) + 1)*atoi(rate_night);
                if(fee1 > atoi(night_max_cost)) fee1 = atoi(night_max_cost);
                fee = fee + fee1;

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 0点到8点进 20点到24点出! return fee[%d]+days_fee[%d] 进入时间在离开时间之前 line[%d]",
                         __FUNCTION__, fee, days_fee, __LINE__);
                writelog(log_buf_);

                return fee+days_fee;
            }
            else if(mil_tmp_in/60 >= atoi(day_start_time) && mil_tmp_in/60 < atoi(night_start_time) && mil_tmp_out/60  <= atoi(night_start_time)) //8点到20点进  8点到20点出
            {
                int fee = ((int)floor((mil_tmp_out/60 -mil_tmp_in/60  )/atoi(uninttime)) + 1)*atoi(rate);
                if(fee > atoi(cycle_max_cost)) fee = atoi(cycle_max_cost);

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 8点到20点进 8点到20点出! return fee[%d]+days_fee[%d] 进入时间在离开时间之前 line[%d]",
                         __FUNCTION__, fee, days_fee, __LINE__);
                writelog(log_buf_);

                return fee+days_fee;
            }
            else if(mil_tmp_in/60 >= atoi(day_start_time) && mil_tmp_in/60 < atoi(night_start_time) && mil_tmp_out/60  > atoi(night_start_time)) //8点到20点进  20点到24点出
            {
                int fee = ((int)floor((atoi(night_start_time) - mil_tmp_in/60)/atoi(uninttime)) + 1)*atoi(rate);
                int fee1 =  ((int)floor((mil_tmp_out/60 - atoi(night_start_time))/atoi(uninttime_night)) + 1)*atoi(rate_night);
                if(fee1 > atoi(night_max_cost)) fee1 = atoi(night_max_cost);
                fee = fee + fee1;
                if(fee > atoi(cycle_max_cost)) fee = atoi(cycle_max_cost);

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 8点到20点进 20点到24点出! return fee[%d]+days_fee[%d] 进入时间在离开时间之前 line[%d]",
                         __FUNCTION__, fee, days_fee, __LINE__);
                writelog(log_buf_);

                return fee+days_fee;
            }
            else  //20点到24点进  20点到24点出
            {
                //int fee = ((int)floor((mil_tmp_out/60- atoi(night_start_time))/atoi(uninttime_night)) + 1)*atoi(rate_night);
                int fee = ((int)floor((mil_tmp_out/60- mil_tmp_in/60)/atoi(uninttime_night)) + 1)*atoi(rate_night);
                if(fee > atoi(night_max_cost))   fee = atoi(night_max_cost);

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 20点到24点进 20点到24点出!! return fee[%d]+days_fee[%d] 进入时间在离开时间之前 line[%d]",
                         __FUNCTION__, fee, days_fee, __LINE__);
                writelog(log_buf_);

                return  fee+days_fee;
            }
        }
        else //化为24小时，进入时间在离开时间之后
        {
            if(mil_tmp_in/60 <= atoi(day_start_time)) //0点到8点进  0点到8点出
            {
                int fee = ((int)floor((atoi(day_start_time)- mil_tmp_in/60)/atoi(uninttime_night)) + 1)*atoi(rate_night);
                if(fee > atoi(night_max_cost))   fee = atoi(night_max_cost);
                fee = fee + ((int)floor((atoi(night_start_time)- atoi(day_start_time))/atoi(uninttime)) + 1)*atoi(rate);
                if(fee > atoi(cycle_max_cost))   fee = atoi(cycle_max_cost);
                int fee1 = ((int)floor((24*60 - atoi(night_start_time) + mil_tmp_out/60 )/atoi(uninttime_night)) + 1)*atoi(rate_night);
		        if(fee1 > atoi(night_max_cost)) fee1 = atoi(night_max_cost);
                fee = fee + fee1;

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 0点到8点进 0点到8点出! return fee[%d]+days_fee[%d] 进入时间在离开时间之后 line[%d]",
                         __FUNCTION__, fee, days_fee, __LINE__);
                writelog(log_buf_);

                return fee+days_fee;
            }
            else if(mil_tmp_in/60 > atoi(day_start_time) && mil_tmp_in/60 <= atoi(night_start_time) && mil_tmp_out/60 <= atoi(day_start_time)) //8点到20点进  第二天的0点到8点出
            {
                int fee = ((int)floor((atoi(night_start_time)- mil_tmp_in/60)/atoi(uninttime)) + 1)*atoi(rate);
		        int fee1 = ((int)floor((24*60 - atoi(night_start_time) + mil_tmp_out/60 )/atoi(uninttime_night)) + 1)*atoi(rate_night);
                if(fee1 > atoi(night_max_cost)) fee1 = atoi(night_max_cost);
                fee = fee + fee1;
                if(fee > atoi(cycle_max_cost))   fee = atoi(cycle_max_cost);

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 8点到20点进 第二天的0点到8点出! return fee[%d]+days_fee[%d] 进入时间在离开时间之后 line[%d]",
                         __FUNCTION__, fee, days_fee, __LINE__);
                writelog(log_buf_);

                return fee+days_fee;
            }
            else if(mil_tmp_in/60 > atoi(day_start_time) && mil_tmp_in/60 <= atoi(night_start_time) && mil_tmp_out/60 < atoi(night_start_time)) //8点到20点进  第二天的8点到20点出
            {
                int fee = ((int)floor((atoi(night_start_time)- mil_tmp_in/60)/atoi(uninttime)) + 1)*atoi(rate);
                fee = fee + atoi(night_max_cost);
                if(fee > atoi(cycle_max_cost))   fee = atoi(cycle_max_cost);
                int fee1 = ((int)floor((mil_tmp_out/60- atoi(day_start_time))/atoi(uninttime)) + 1)*atoi(rate);
                if(fee1 > atoi(cycle_max_cost))   fee1 = atoi(cycle_max_cost);
                fee = fee + fee1;

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 8点到20点进 第二天的8点到20点出! return fee[%d]+days_fee[%d] 进入时间在离开时间之后 line[%d]",
                         __FUNCTION__, fee, days_fee, __LINE__);
                writelog(log_buf_);

                return fee+days_fee;
            }
            else if(mil_tmp_in/60 >  atoi(night_start_time) && mil_tmp_out/60 <= atoi(day_start_time)) //20点到24点进  第二天的0点到8点出
            {
                int fee = ((int)floor((24*60 - mil_tmp_in/60 + mil_tmp_out/60)/atoi(uninttime_night)) + 1)*atoi(rate_night);
                if(fee > atoi(night_max_cost) ) fee = atoi(night_max_cost);

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 20点到24点进 第二天的0点到8点出! return fee[%d]+days_fee[%d] 进入时间在离开时间之后 line[%d]",
                         __FUNCTION__, fee, days_fee, __LINE__);
                writelog(log_buf_);

                return fee+days_fee;
            }
            else if(mil_tmp_in/60 >  atoi(night_start_time) && mil_tmp_out/60 > atoi(day_start_time) && mil_tmp_out/60 <= atoi(night_start_time)) //20点到24点进  第二天的8点到20点出
            {
                int fee = ((int)floor((mil_tmp_out/60 - atoi(day_start_time))/atoi(uninttime)) + 1)*atoi(rate) + atoi(night_max_cost);
                if(fee > atoi(cycle_max_cost))   fee = atoi(cycle_max_cost);

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 20点到24点进 第二天的8点到20点出! return fee[%d]+days_fee[%d] 进入时间在离开时间之后 line[%d]",
                         __FUNCTION__, fee, days_fee, __LINE__);
                writelog(log_buf_);

                return fee+days_fee;
            }
            else if(mil_tmp_in/60 >  atoi(night_start_time) && mil_tmp_out/60 >  atoi(night_start_time)) //20点到24点进  第二天的20点到24点出
            {
                int fee = ((int)floor((mil_tmp_out/60 - atoi(night_start_time))/atoi(uninttime_night)) + 1)*atoi(rate_night);
                fee = fee + atoi(cycle_max_cost);

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 20点到24点进 第二天的20点到24点出! return fee[%d]+days_fee[%d] 进入时间在离开时间之后 line[%d]",
                         __FUNCTION__, fee, days_fee, __LINE__);
                writelog(log_buf_);

                return fee+days_fee;
            }else{
                snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 没有处理的逻辑 line[%d]", __FUNCTION__, __LINE__);
                writelog(log_buf_);
            }
        }
    }
}
/******************************白天按小时收费，晚上按小时收费***********************************************/
int mongodb_day_hour_night_hour(char *id,char *car_type,char *in_time,char *out_time)
{
  //  mongoc_client_t *mongodb_client;

    mongoc_collection_t *mongodb_table_chargerule;	//device表
 //   mongodb_client = mongoc_client_new(str_con);

    mongodb_table_chargerule = mongoc_client_get_collection(mongodb_client,"boondb","chargerule");   //device表

    char rate[24];
    char rate_night[24];

    char uninttime[24];
    char uninttime_night[24];
    char ci[24];
    char day_start_time[24];
    char night_start_time[24];
    char day_free_time[24];
    char night_free_time[24];
    char append_free_time[24] = {0};

    mongoc_cursor_t *cursor;
    bson_t *query;
    bson_t result;
    const char *tmp;
    const bson_t *tmp1;
    bson_iter_t iter;
    bson_error_t error;
    unsigned int length;
    char tmp_in[24];
    char tmp_out[24];

    char time_now[64];
    time_t tm;
    time_printf(time(&tm),time_now);

    /******************************查询白天开始时间，夜间开始时间，白天收费费率和单位时间，白天免费时间，夜间免费时间***********************************************/
    query = bson_new();

    BSON_APPEND_UTF8(query, "chargerule_park_id",id); //查询条件
    BSON_APPEND_UTF8(query, "chargerule_car_type",car_type); //查询条件
    cursor = mongoc_collection_find (mongodb_table_chargerule, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);

        fprintf(fp_log,"%s##mongodb_day_hour_night_ci 失败\n",time_now);
        return -1;
    }
    if(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor))
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "chargerule_day_start_time"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(day_start_time,0,24);
                memcpy(day_start_time,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "chargerule_night_start_time"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(night_start_time,0,24);
                memcpy(night_start_time,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "chargerule_day_first_period_rate"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(rate,0,24);
                memcpy(rate,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "chargerule_night_first_period_rate"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(rate_night,0,24);
                memcpy(rate_night,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "chargerule_day_first_period_uninttime"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(uninttime,0,24);
                memcpy(uninttime,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "chargerule_night_first_period_uninttime"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(uninttime_night,0,24);
                memcpy(uninttime_night,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "chargerule_day_free_period_duration"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(day_free_time,0,24);
                memcpy(day_free_time,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "chargerule_night_free_period_duration"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(night_free_time,0,24);
                memcpy(night_free_time,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "chargerule_append_free_time"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(append_free_time,0,24);
                memcpy(append_free_time,tmp,length);
            }
            bson_destroy(&result);
        }
    }
    bson_destroy(query);

    mongoc_cursor_destroy(cursor);
    mongoc_collection_destroy(mongodb_table_chargerule);
  //  mongoc_client_destroy(mongodb_client);

    fprintf(fp_log,"%s##白天开始时间为%s,夜间开始时间为%s,白天费率为%s元/%s分钟,夜晚费率为%s元/%s分钟，白天免费时间为%s,夜间免费时间为%s\n",
            time_now,day_start_time,night_start_time,rate,uninttime,rate_night,uninttime_night,day_free_time,night_free_time);

    //snprintf(log_buf_, sizeof(log_buf_), "[%s] 白天开始时间为[%s], 夜间开始时间为[%s], 白天费率为[%s]元/[%s]分钟, 夜晚费率为[%s]元/[%s]分钟, 白天免费时间为[%s], 夜间免费时间为[%s] line[%d]",
    //         __FUNCTION__, day_start_time,night_start_time,rate,uninttime,rate_night,uninttime_night,day_free_time,night_free_time, __LINE__);
    //writelog(log_buf_);

    /**************************************************end******************************************/
    int mil_now = get_tick(out_time); //得到当前时间的秒数

    int mil_in = get_tick(in_time);	 //得到进入时间的秒数

    std::string in(in_time);
    std::string out(out_time);

    mil_now = mil_now - 60;
    if(mil_in >= mil_now) mil_in = mil_now - 10;
    memset(tmp_in,0,24);
    sprintf(tmp_in,"%s 00:00:00",in.substr(0,10).c_str()); //获取进入时间的凌晨时间
    memset(tmp_out,0,24);
    sprintf(tmp_out,"%s 00:00:00",out.substr(0,10).c_str()); //获取离开时间的凌晨时间

    int mil_tmp_in = mil_in -  get_tick(tmp_in); //化为24小时以内 得到进入时间
    int mil_tmp_out =  mil_now - get_tick(tmp_out); //化为24小时以内 得到离开时间

    if(mil_tmp_in/60 >= atoi(day_start_time) && mil_tmp_in/60 <= atoi(night_start_time)) //白天
    {
        snprintf(log_buf_, sizeof(log_buf_), "[%s] 白天 line[%d]", __FUNCTION__, __LINE__);
        writelog(log_buf_);

        if(mil_now/60 - mil_in/60 < (atoi(day_free_time) + 1)) //停留时长小于免费时间
        {
            return 0;
        }
        if(strcmp(append_free_time,"是") == 0)		//判断是否追加免费时间
        {

        }
        else
        {
            mil_in += 60 * atoi(day_free_time);
            mil_tmp_in = mil_in -  get_tick(tmp_in);
            if(mil_tmp_in < 0) mil_tmp_in = 0;
            fprintf(fp_log,"不追加免费时长");
        }
    }
    else //夜间
    {
        snprintf(log_buf_, sizeof(log_buf_), "[%s] 夜间 line[%d]", __FUNCTION__, __LINE__);
        writelog(log_buf_);

        if(mil_now/60 - mil_in/60 < (atoi(night_free_time) + 1)) //停留时长小于免费时间
        {
            return 0;
        }
        if(strcmp(append_free_time,"是") == 0)		//判断是否追加免费时间
        {

        }
        else
        {
            mil_in += 60 * atoi(day_free_time);
            mil_tmp_in = mil_in -  get_tick(tmp_in);
            if(mil_tmp_in < 0) mil_tmp_in = 0;
            fprintf(fp_log,"不追加免费时长");
        }
    }

    if(mil_now - mil_in >= 86400) //停留时长大于24小时
    {
        snprintf(log_buf_, sizeof(log_buf_), "[%s] 停留时长大于24小时 line[%d]", __FUNCTION__, __LINE__);
        writelog(log_buf_);

        int days = floor((mil_now - mil_in)/86400); //停留整天数

        int days_fee = days*((atoi(night_start_time) - atoi(day_start_time))/atoi(uninttime)*atoi(rate) + (24*60 - atoi(night_start_time) + atoi(day_start_time))/atoi(uninttime_night)*atoi(rate_night)); //停留整天数的费用

        if(mil_tmp_in < mil_tmp_out) //化为24小时，进入时间在离开时间之前
        {
            if(mil_tmp_in/60 < atoi(day_start_time) && mil_tmp_out/60  <= atoi(day_start_time)) //0点到8点进  0点到8点出
                return ((int)floor((mil_tmp_out/60 - mil_tmp_in/60)/atoi(uninttime_night)) + 1)*atoi(rate_night)+days_fee;
            else if(mil_tmp_in/60 < atoi(day_start_time) && mil_tmp_out/60  >atoi(day_start_time)&& mil_tmp_out/60  <= atoi(night_start_time)) //0点到8点进  8点到20点出
                return ((int)floor((atoi(day_start_time) - mil_tmp_in/60)/atoi(uninttime_night)) + 1)*atoi(rate_night) + ((int)floor((mil_tmp_out/60 - atoi(day_start_time))/atoi(uninttime)) + 1)*atoi(rate) + days_fee;
            else if(mil_tmp_in/60 < atoi(day_start_time) && mil_tmp_out/60  > atoi(night_start_time))  //0点到8点进  20点到24点出
                return ((int)floor((atoi(day_start_time) - mil_tmp_in/60)/atoi(uninttime_night)) + 1)*atoi(rate_night) +  ((int)floor((atoi(night_start_time) - atoi(day_start_time))/atoi(uninttime)) + 1)*atoi(rate) + days_fee;
            else if(mil_tmp_in/60 >= atoi(day_start_time) && mil_tmp_in/60 < atoi(night_start_time) && mil_tmp_out/60  <= atoi(night_start_time)) //8点到20点进  8点到20点出
                return ((int)floor((mil_tmp_out/60 - mil_tmp_in/60)/atoi(uninttime)) + 1)*atoi(rate) + days_fee;
            else if(mil_tmp_in/60 >= atoi(day_start_time) && mil_tmp_in/60 < atoi(night_start_time) && mil_tmp_out/60  > atoi(night_start_time)) //8点到20点进  20点到24点出
                return  ((int)floor((atoi(night_start_time) - mil_tmp_in/60)/atoi(uninttime)) + 1)*atoi(rate) + ((int)floor((mil_tmp_out/60 - atoi(night_start_time))/atoi(uninttime_night)) + 1)*atoi(rate_night) + days_fee;
            else  //20点到24点进  20点到24点出
                return  ((int)floor((mil_tmp_out/60 - mil_tmp_in/60)/atoi(uninttime_night)) + 1)*atoi(rate_night)+days_fee;
        }
        else //化为24小时，进入时间在离开时间之后
        {
            if(mil_tmp_in/60 <= atoi(day_start_time)) //0点到8点进  0点到8点出
                return ((int)floor((atoi(night_start_time) - atoi(day_start_time))/atoi(uninttime))+1)*atoi(rate) + ((int)floor((atoi(day_start_time) - mil_tmp_in/60 + 24*60 - atoi(night_start_time) + mil_tmp_out/60 )/atoi(uninttime_night)) +1)*atoi(rate_night) + days_fee;
            else if(mil_tmp_in/60 > atoi(day_start_time) && mil_tmp_in/60 <= atoi(night_start_time) && mil_tmp_out/60 <= atoi(day_start_time)) //8点到20点进  第二天的0点到8点出
                return ((int)floor((atoi(night_start_time) - mil_tmp_in/60)/atoi(uninttime)) +1 )*atoi(rate) + ((int)floor((24*60 - atoi(night_start_time) + mil_tmp_out/60 )/atoi(uninttime_night)) + 1)*atoi(rate_night) + days_fee;
            else if(mil_tmp_in/60 > atoi(day_start_time) && mil_tmp_in/60 <= atoi(night_start_time) && mil_tmp_out/60 < atoi(night_start_time)) //8点到20点进  第二天的8点到20点出
                return ((int)floor((atoi(night_start_time) - mil_tmp_in/60 + mil_tmp_out/60 - atoi(day_start_time) )/atoi(uninttime)) + 1)*atoi(rate) + ((int)floor((24*60 - atoi(night_start_time) + atoi(day_start_time) )/atoi(uninttime_night)) + 1)*atoi(rate_night) + days_fee;
            else if(mil_tmp_in/60 >  atoi(night_start_time) && mil_tmp_out/60 <= atoi(day_start_time)) //20点到24点进  第二天的0点到8点出
                return ((int)floor((24*60 - mil_tmp_in/60 + mil_tmp_out/60 )/atoi(uninttime_night)) + 1)*atoi(rate_night)  + days_fee;
            else if(mil_tmp_in/60 >  atoi(night_start_time) && mil_tmp_out/60 > atoi(day_start_time) && mil_tmp_out/60 <= atoi(night_start_time)) //20点到24点进  第二天的8点到20点出
                return ((int)floor((24*60 - mil_tmp_in/60 + atoi(day_start_time) )/atoi(uninttime_night)) + 1)*atoi(rate_night)  + ((int)floor((mil_tmp_out/60 - atoi(day_start_time) )/atoi(uninttime)) + 1)*atoi(rate) + days_fee;
            else if(mil_tmp_in/60 >  atoi(night_start_time) && mil_tmp_out/60 >  atoi(night_start_time)) //20点到24点进  第二天的20点到24点出
                return ((int)floor((24*60 - mil_tmp_in/60 + atoi(day_start_time) + mil_tmp_out/60 - atoi(night_start_time) )/atoi(uninttime_night)) + 1)*atoi(rate_night)  + ((int)floor((atoi(night_start_time) - atoi(day_start_time) )/atoi(uninttime)) + 1)*atoi(rate) + days_fee;
        }

    }
    else ////停留时长小于24小时
    {
        snprintf(log_buf_, sizeof(log_buf_), "[%s] 停留时长小于24小时 line[%d]", __FUNCTION__, __LINE__);
        writelog(log_buf_);

        int days_fee = 0;
        if(mil_tmp_in < mil_tmp_out) //化为24小时，进入时间在离开时间之前
        {
            if(mil_tmp_in/60 < atoi(day_start_time) && mil_tmp_out/60  <= atoi(day_start_time)) //0点到8点进  0点到8点出
                return ((int)floor((mil_tmp_out/60 - mil_tmp_in/60)/atoi(uninttime_night)) + 1)*atoi(rate_night)+days_fee;
            else if(mil_tmp_in/60 < atoi(day_start_time) && mil_tmp_out/60  >atoi(day_start_time)&& mil_tmp_out/60  <= atoi(night_start_time)) //0点到8点进  8点到20点出
                return ((int)floor((atoi(day_start_time) - mil_tmp_in/60)/atoi(uninttime_night)) + 1)*atoi(rate_night) + ((int)floor((mil_tmp_out/60 - atoi(day_start_time))/atoi(uninttime)) + 1)*atoi(rate) + days_fee;
            else if(mil_tmp_in/60 < atoi(day_start_time) && mil_tmp_out/60  > atoi(night_start_time))  //0点到8点进  20点到24点出
                return ((int)floor((atoi(day_start_time) - mil_tmp_in/60)/atoi(uninttime_night)) + 1)*atoi(rate_night) +  ((int)floor((atoi(night_start_time) - atoi(day_start_time))/atoi(uninttime)) + 1)*atoi(rate) + days_fee;
            else if(mil_tmp_in/60 >= atoi(day_start_time) && mil_tmp_in/60 < atoi(night_start_time) && mil_tmp_out/60  <= atoi(night_start_time)) //8点到20点进  8点到20点出
                return ((int)floor((mil_tmp_out/60 - mil_tmp_in/60)/atoi(uninttime)) + 1)*atoi(rate) + days_fee;
            else if(mil_tmp_in/60 >= atoi(day_start_time) && mil_tmp_in/60 < atoi(night_start_time) && mil_tmp_out/60  > atoi(night_start_time)) //8点到20点进  20点到24点出
                return  ((int)floor((atoi(night_start_time) - mil_tmp_in/60)/atoi(uninttime)) + 1)*atoi(rate) + ((int)floor((mil_tmp_out/60 - atoi(night_start_time))/atoi(uninttime_night)) + 1)*atoi(rate_night) + days_fee;
            else  //20点到24点进  20点到24点出
                return  ((int)floor((mil_tmp_out/60 - mil_tmp_in/60)/atoi(uninttime_night)) + 1)*atoi(rate_night)+days_fee;
        }
        else //化为24小时，进入时间在离开时间之后
        {
            if(mil_tmp_in/60 <= atoi(day_start_time)) //0点到8点进  0点到8点出
                return ((int)floor((atoi(night_start_time) - atoi(day_start_time))/atoi(uninttime))+1)*atoi(rate) + ((int)floor((atoi(day_start_time) - mil_tmp_in/60 + 24*60 - atoi(night_start_time) + mil_tmp_out/60 )/atoi(uninttime_night)) +1)*atoi(rate_night) + days_fee;
            else if(mil_tmp_in/60 > atoi(day_start_time) && mil_tmp_in/60 <= atoi(night_start_time) && mil_tmp_out/60 <= atoi(day_start_time)) //8点到20点进  第二天的0点到8点出
                return ((int)floor((atoi(night_start_time) - mil_tmp_in/60)/atoi(uninttime)) +1 )*atoi(rate) + ((int)floor((24*60 - atoi(night_start_time) + mil_tmp_out/60 )/atoi(uninttime_night)) + 1)*atoi(rate_night) + days_fee;
            else if(mil_tmp_in/60 > atoi(day_start_time) && mil_tmp_in/60 <= atoi(night_start_time) && mil_tmp_out/60 < atoi(night_start_time)) //8点到20点进  第二天的8点到20点出
                return ((int)floor((atoi(night_start_time) - mil_tmp_in/60 + mil_tmp_out/60 - atoi(day_start_time) )/atoi(uninttime)) + 1)*atoi(rate) + ((int)floor((24*60 - atoi(night_start_time) + atoi(day_start_time) )/atoi(uninttime_night)) + 1)*atoi(rate_night) + days_fee;
            else if(mil_tmp_in/60 >  atoi(night_start_time) && mil_tmp_out/60 <= atoi(day_start_time)) //20点到24点进  第二天的0点到8点出
                return ((int)floor((24*60 - mil_tmp_in/60 + mil_tmp_out/60 )/atoi(uninttime_night)) + 1)*atoi(rate_night)  + days_fee;
            else if(mil_tmp_in/60 >  atoi(night_start_time) && mil_tmp_out/60 > atoi(day_start_time) && mil_tmp_out/60 <= atoi(night_start_time)) //20点到24点进  第二天的8点到20点出
                return ((int)floor((24*60 - mil_tmp_in/60 + atoi(day_start_time) )/atoi(uninttime_night)) + 1)*atoi(rate_night)  + ((int)floor((mil_tmp_out/60 - atoi(day_start_time) )/atoi(uninttime)) + 1)*atoi(rate) + days_fee;
            else if(mil_tmp_in/60 >  atoi(night_start_time) && mil_tmp_out/60 >  atoi(night_start_time)) //20点到24点进  第二天的20点到24点出
                return ((int)floor((24*60 - mil_tmp_in/60 + atoi(day_start_time) + mil_tmp_out/60 - atoi(night_start_time) )/atoi(uninttime_night)) + 1)*atoi(rate_night)  + ((int)floor((atoi(night_start_time) - atoi(day_start_time) )/atoi(uninttime)) + 1)*atoi(rate) + days_fee;
        }
    }

}
/******************************全天按小时收费***********************************************/
int mongodb_calfee_all_day_by_hour(char *id,char *car_type,char *in_time,char *out_time)
{
  //  mongoc_client_t *mongodb_client;

    mongoc_collection_t *mongodb_table_chargerule;	//device表
  //  mongodb_client = mongoc_client_new(str_con);

    mongodb_table_chargerule = mongoc_client_get_collection(mongodb_client,"boondb","chargerule");   //device表

    char rate[24] = {0};
    char uninttime[24] = {0};
    char free_time[24] = {0};
    char max_cost[24] = {0};
    char fdfs[24] = {0};
    mongoc_cursor_t *cursor;
    bson_t *query;
    bson_t result;
    const char *tmp;
    const bson_t *tmp1;
    bson_iter_t iter;
    bson_error_t error;
    unsigned int length;

    char append_free_time[24] = {0};
    char time_now[64] = {0};
    time_t tm;
    time_printf(time(&tm),time_now);
    char tmp_in[24] = {0};
    char tmp_out[24] = {0};

    snprintf(log_buf_, sizeof(log_buf_), "[%s] 查询收费方案信息 查询chargerule表 查询条件 chargerule_park_id[%s] chargerule_car_type[%s] line[%d]",
             __FUNCTION__, id, car_type, __LINE__);
    writelog(log_buf_);

    /******************************查询费率和单位时间***********************************************/
    query = bson_new();

    BSON_APPEND_UTF8(query, "chargerule_park_id",id); //查询条件
    BSON_APPEND_UTF8(query, "chargerule_car_type",car_type); //查询条件
    cursor = mongoc_collection_find (mongodb_table_chargerule, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);
        mongoc_cursor_destroy(cursor);

        fprintf(fp_log,"%s##mongodb_calfee_all_day_by_hour 失败\n",time_now);
        return -1;
    }
    if(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor))
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "chargerule_day_first_period_rate"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(rate,0,24);
                memcpy(rate,tmp,length);

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 全天按小时费率[%s] line[%d]", __FUNCTION__, rate, __LINE__);
                writelog(log_buf_);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "chargerule_day_first_period_uninttime"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(uninttime,0,24);
                memcpy(uninttime,tmp,length);

                snprintf(log_buf_, sizeof(log_buf_), "[%s] [%s]分钟 line[%d]", __FUNCTION__, uninttime, __LINE__);
                writelog(log_buf_);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "chargerule_day_free_period_duration"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(free_time,0,24);
                memcpy(free_time,tmp,length);

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 免费时间为[%s] line[%d]", __FUNCTION__, free_time, __LINE__);
                writelog(log_buf_);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "chargerule_natural_max_cost"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(max_cost,0,24);
                memcpy(max_cost,tmp,length);

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 封顶金额[%s] line[%d]", __FUNCTION__, max_cost, __LINE__);
                writelog(log_buf_);
            }
            /*
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "chargerule_append_free_time"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(append_free_time,0,24);
                memcpy(append_free_time,tmp,length);
            }
            */
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "chargerule_caps_way_select"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(fdfs,0,256);
                memcpy(fdfs,tmp,length);

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 封顶方式[%s] line[%d]", __FUNCTION__, fdfs, __LINE__);
                writelog(log_buf_);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "chargerule_append_free_time"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(append_free_time,0,24);
                memcpy(append_free_time,tmp,length);

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 追加时长[%s] line[%d]", __FUNCTION__, append_free_time, __LINE__);
                writelog(log_buf_);
            }
            bson_destroy(&result);
        }
    }
    bson_destroy(query);

    mongoc_cursor_destroy(cursor);
    mongoc_collection_destroy(mongodb_table_chargerule);

    snprintf(log_buf_, sizeof(log_buf_), "[%s] 全天按小时费率[%s]/[%s]分钟, 免费时间为[%s], 封顶方式[%s] 封顶金额[%s] 追加时长[%s] line[%d]",
             __FUNCTION__, rate, uninttime, free_time, fdfs, max_cost, append_free_time, __LINE__);
    writelog(log_buf_);

 //   mongoc_client_destroy(mongodb_client);
    /**************************************************end******************************************/
    if(atoi(max_cost) <= 0)
    {
        int tmp = 24*60/atoi(uninttime)*atoi(rate);
        sprintf(max_cost,"%d",tmp);
    }

    fprintf(fp_log,"%s##进入时间%s 离开时间%s  追加:%s\n",time_now,in_time,out_time,append_free_time);

    snprintf(log_buf_, sizeof(log_buf_), "[%s] 进入时间[%s] 离开时间[%s] 追加[%s] line[%d]", __FUNCTION__, in_time, out_time, append_free_time, __LINE__);
    writelog(log_buf_);

    int mil_out = get_tick(out_time); //得到当前时间的秒数
    mil_out = mil_out - 60;
    int mil_in = get_tick(in_time);	 //得到进入时间的秒数
    if((atoi(free_time) + 1) >= (mil_out - mil_in)/60)
        return 0;

    if(strcmp(append_free_time,"是") == 0)		//判断是否追加免费时间
    {
        fprintf(fp_log,"追加免费时长");
    }
    else
    {
        mil_in += 60 * atoi(free_time);

        fprintf(fp_log,"不追加免费时长");
    }
    if(!strcmp(fdfs,"累计时间"))
    {
        int fee = 0;
        int remaindaytime = 0;
        int count =  (int)floor((mil_out-mil_in)/(24*60*60));
        remaindaytime = (mil_out-mil_in)%(24*60*60);
        /*
        if(!(strcmp(append_free_time,"是") == 0))		//判断是否追加免费时间
        {
            if(count == 0)
                remaindaytime -= atoi(free_time)*60;
        }
        */
        if(remaindaytime<0)
        {
            remaindaytime=0;
        }
        else
        {
            fee = ((int)floor((remaindaytime/60 /atoi(uninttime)) + 1))*atoi(rate);
        }
        fprintf(fp_log,"计算累计时间,count: %d,fee: %d\n,max_cost: %d",count,fee,atoi(max_cost));
        if(fee<0)fee = 0;
        int ret = count * atoi(max_cost) + (fee<atoi(max_cost)?fee:atoi(max_cost)) ;

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 收费ret[%d] line[%d]", __FUNCTION__, ret, __LINE__);
        writelog(log_buf_);

        return ret;
    }
    if(mil_in >= mil_out) mil_in = mil_out - 10;
    std::string in(in_time);
    std::string out(out_time);

    memset(tmp_in,0,24);
    sprintf(tmp_in,"%s 00:00:00",in.substr(0,10).c_str()); //获取进入时间的凌晨时间
    memset(tmp_out,0,24);
    sprintf(tmp_out,"%s 00:00:00",out.substr(0,10).c_str()); //获取离开时间的凌晨时间

    int mil_tmp_in = mil_in -  get_tick(tmp_in); //化为24小时以内 得到进入时间
    int mil_tmp_out =  mil_out - get_tick(tmp_out); //化为24小时以内 得到离开时间


    int fee = 0;
    if((mil_out - mil_in)/60 >= (24*60 + 24*60 - mil_tmp_in/60))
    {
        int count =  (int)floor((mil_out-mil_in)/24/60/60);
        mil_in = mil_tmp_in;
        mil_out = mil_tmp_out;
        int fee1 = ((int)floor((24*60 - mil_in/60) /atoi(uninttime)) + 1)*atoi(rate);
        if(fee1 > atoi(max_cost))  fee1 = atoi(max_cost);
        int fee2 = ((int)floor((24*60) /atoi(uninttime)) + 1)*atoi(rate);
        if(fee2 > atoi(max_cost))  fee2 = atoi(max_cost);
        fee2 = fee2 * count;
        int fee3 = ((int)floor((mil_out/60) /atoi(uninttime)) + 1)*atoi(rate);
        if(fee3 > atoi(max_cost))  fee3 = atoi(max_cost);
        fee = fee1 + fee2 + fee3;

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 收费ret[%d]. line[%d]", __FUNCTION__, fee, __LINE__);
        writelog(log_buf_);
    }
    else if(mil_tmp_in < mil_tmp_out)
    {
        mil_in = mil_tmp_in;
        mil_out = mil_tmp_out;
        fee = ((int)floor((mil_out/60 - mil_in/60) /atoi(uninttime)) + 1)*atoi(rate);
        printf("fee %d \n",fee);
        if(fee > atoi(max_cost)) fee = atoi(max_cost);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 收费ret[%d].. line[%d]", __FUNCTION__, fee, __LINE__);
        writelog(log_buf_);
    }
    else
    {
        mil_in = mil_tmp_in;
        mil_out = mil_tmp_out;
        int fee1 = ((int)floor((24*60 - mil_in/60) /atoi(uninttime)) + 1)*atoi(rate);
        if(fee1 > atoi(max_cost))  fee1 = atoi(max_cost);
        int fee3 = ((int)floor((mil_out/60) /atoi(uninttime)) + 1)*atoi(rate);
        if(fee3 > atoi(max_cost))  fee3 = atoi(max_cost);
        fee = fee1  + fee3;

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 收费ret[%d]... line[%d]", __FUNCTION__, fee, __LINE__);
        writelog(log_buf_);
    }

    return fee;
}

/******************************计费规则***********************************************/
int mongodb_cal_fee(char *rule,char *in_time,char *out_time,char *car_type,char *id,char *plate)
{
    snprintf(log_buf_, sizeof(log_buf_), "[%s] 计算收费金额, 入参 rule[%s] in_time[%s] out_time[%s] car_type[%s] plate[%s] line[%d]",
             __FUNCTION__, rule, in_time, out_time, car_type, plate, __LINE__);
    writelog(log_buf_);

  //  mongoc_client_t *mongodb_client;
    mongoc_collection_t *mongodb_table_car;	//channel表
    mongoc_collection_t *mongodb_table_chargerule;	//device表
    mongoc_collection_t *mongodb_table_carinpark;	//carinpark表
    mongoc_collection_t *mongodb_table_channel;	//carinpark表
  //  mongodb_client = mongoc_client_new(str_con);
    mongodb_table_car = mongoc_client_get_collection(mongodb_client,"boondb","car");   //channel表
    mongodb_table_chargerule = mongoc_client_get_collection(mongodb_client,"boondb","chargerule");   //device表
    mongodb_table_carinpark = mongoc_client_get_collection(mongodb_client,"boondb","carinpark");   //carinpark表
    mongodb_table_channel = mongoc_client_get_collection(mongodb_client,"boondb","channel");   //carinpark表

    mongoc_cursor_t *cursor;
    bson_t *query;
    bson_t result;
    const char *tmp;
    const bson_t *tmp1;
    bson_iter_t iter;
    bson_error_t error;
    unsigned int length;

    char time_now[64];

    time_t tm;
    time_printf(time(&tm),time_now);
    char in_channel_id[1024] = {0};
    char in_ip[24] = {0};
    char tonggang[24] = {0};

    if(strlen(in_time) < 2)
    {
        memcpy(in_time,time_now,strlen(time_now));
    }

    query = bson_new();

    BSON_APPEND_UTF8(query, "carinpark_plate_id",plate); //查询条件

    cursor = mongoc_collection_find (mongodb_table_carinpark, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);

        fprintf(fp_log,"%s##mongodb_cal_fee 失败\n",time_now);
        return -1;
    }
    if(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor))
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "carinpark_channel_id"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(in_channel_id,0,1024);
                memcpy(in_channel_id,tmp,length);

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 查carinpark表 查询通道信息 carinpark_channel_id[%s] line[%d]", __FUNCTION__, in_channel_id, __LINE__);
                writelog(log_buf_);
            }
            bson_destroy(&result);
        }
    }
    bson_destroy(query);

    mongoc_cursor_destroy(cursor);
    /********************/
    query = bson_new();

    BSON_APPEND_UTF8(query, "channel_id",in_channel_id); //查询条件

    cursor = mongoc_collection_find (mongodb_table_channel, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);

        fprintf(fp_log,"%s##mongodb_cal_fee 失败\n",time_now);
        return -1;
    }
    if(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor))
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "channel_ip"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(in_ip,0,24);
                memcpy(in_ip,tmp,length);

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 查channel表 查询通道信息 channel_ip[%s] line[%d]", __FUNCTION__, in_ip, __LINE__);
                writelog(log_buf_);
            }
            bson_destroy(&result);
        }
    }
    bson_destroy(query);

    mongoc_cursor_destroy(cursor);

    printf("------------ip-------------------%s,%s\n",getLocalIp().c_str(),in_ip);
    if(!strcmp(getLocalIp().c_str(),in_ip))
    {
        strcpy(tonggang,"yes");

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 查询通道信息 是本机ip local[%s] in_ip[%s] line[%d]", __FUNCTION__, getLocalIp().c_str(), in_ip, __LINE__);
        writelog(log_buf_);
    }
    else
    {
        strcpy(tonggang,"no");

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 查询通道信息 不是本机ip local[%s] in_ip[%s] line[%d]", __FUNCTION__, getLocalIp().c_str(), in_ip, __LINE__);
        writelog(log_buf_);
    }

    int mil_in = get_tick(in_time);	 //得到进入时间的秒数
    if((mil_in == -1)||(strlen(in_time) == 0))
    {
        mongoc_collection_destroy(mongodb_table_car);
        mongoc_collection_destroy(mongodb_table_chargerule);
        //    mongoc_client_destroy(mongodb_client);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] warn! 没有找到入场时间, 返回0元 line[%d]", __FUNCTION__, __LINE__);
        writelog(log_buf_);

        return 0;
    }

    snprintf(log_buf_, sizeof(log_buf_), "[%s] 收费方案 rule[%s] line[%d]", __FUNCTION__, rule, __LINE__);
    writelog(log_buf_);

    if(strcmp(rule,"免费") == 0) //收费规则为免费
    {
        fprintf(fp_log,"%s##收费规则为%s\n",time_now,rule);
        mongoc_collection_destroy(mongodb_table_car);
        mongoc_collection_destroy(mongodb_table_chargerule);
     //   mongoc_client_destroy(mongodb_client);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 收费方案[%s], 返回0元. line[%d]", __FUNCTION__, rule, __LINE__);
        writelog(log_buf_);

        return 0;
    }
    else if(strcmp(rule,"指定时间免费") == 0) //收费规则为指定时间免费
    {
        char stop_time[24] = {0};

        char guanlian_rule[256] = {0};

        fprintf(fp_log,"%s##收费规则为%s\n",time_now,rule);
        /******************************查询车辆到期时间***********************************************/
        query = bson_new();

        BSON_APPEND_UTF8(query, "car_plate_id",plate); //查询条件

        cursor = mongoc_collection_find (mongodb_table_car, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
        if(mongoc_cursor_error(cursor,&error))
        {
            bson_destroy(query);
            mongoc_cursor_destroy(cursor);

            snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 查找数据库失败 line[%d]", __FUNCTION__, __LINE__);
            writelog(log_buf_);

            fprintf(fp_log,"%s##mongodb_cal_fee 失败\n",time_now);
            return -1;
        }

        bool find_value = false;
        if(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor))
        {
            if (mongoc_cursor_next (cursor, &tmp1))
            {
                bson_copy_to(tmp1,&result); //得到一条完整的记录
                if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "car_stop_time"))
                {
                    tmp = bson_iter_utf8(&iter,&length);
                    memset(stop_time,0,24);
                    memcpy(stop_time,tmp,length);

                    find_value = true;

                    snprintf(log_buf_, sizeof(log_buf_), "[%s] 获取车辆plate[%s]到期时间 car_stop_time[%s] 查找car表 line[%d]", __FUNCTION__, plate, stop_time, __LINE__);
                    writelog(log_buf_);
                }
                bson_destroy(&result);
            }
        }
        bson_destroy(query);
        mongoc_cursor_destroy(cursor);

        if(!find_value){
            snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 没有找到plate[%s]过期时间 查找car表 line[%d]", __FUNCTION__, plate, __LINE__);
            writelog(log_buf_);
        }

        /**************************************************end******************************************/
        /******************************查询过期关联收费规则***********************************************/
        query = bson_new();

        BSON_APPEND_UTF8(query, "chargerule_park_id",id); //查询条件
        BSON_APPEND_UTF8(query, "chargerule_car_type",car_type); //查询条件
        cursor = mongoc_collection_find (mongodb_table_chargerule, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
        if(mongoc_cursor_error(cursor,&error))
        {
            bson_destroy(query);
            mongoc_cursor_destroy(cursor);

            snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 查找数据库失败 line[%d]", __FUNCTION__, __LINE__);
            writelog(log_buf_);

            fprintf(fp_log,"%s##mongodb_cal_fee 失败\n",time_now);
            return -1;
        }

        find_value = false;
        if(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor))
        {
            if (mongoc_cursor_next (cursor, &tmp1))
            {
                bson_copy_to(tmp1,&result); //得到一条完整的记录
                if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "chargerule_guanlian_id"))
                {
                    tmp = bson_iter_utf8(&iter,&length);
                    memset(guanlian_rule,0,256);
                    memcpy(guanlian_rule,tmp,length);

                    find_value = true;

                    snprintf(log_buf_, sizeof(log_buf_), "[%s] 找到chargerule_guanlian_id[%s] 查找chargerule表 line[%d]",
                             __FUNCTION__, guanlian_rule, __LINE__);
                    writelog(log_buf_);
                }
                bson_destroy(&result);
            }
        }
        bson_destroy(query);
        mongoc_cursor_destroy(cursor);

        if(!find_value){
            snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 没有找到关联规则 查找chargerule表 查找条件car_type[%s] line[%d]",
                     __FUNCTION__, car_type, __LINE__);
            writelog(log_buf_);
        }
        /**************************************************end******************************************/

        int mil_now = get_tick(time_now); //得到当前时间的秒数
        int mil_stop = get_tick(stop_time);	 //得到到期时间的秒数
        int mil_in = get_tick(in_time);	 //得到进入时间的秒数
        if((mil_in == -1)||(strlen(in_time) == 0))
        {
            mongoc_collection_destroy(mongodb_table_car);
            mongoc_collection_destroy(mongodb_table_chargerule);
         //   mongoc_client_destroy(mongodb_client);

            snprintf(log_buf_, sizeof(log_buf_), "[%s] error! in_time不合法 return 0! line[%d]", __FUNCTION__, __LINE__);
            writelog(log_buf_);

            return 0;
        }

        if(mil_now <= mil_stop) //未过期
        {
            mongoc_collection_destroy(mongodb_table_car);
            mongoc_collection_destroy(mongodb_table_chargerule);
          //  mongoc_client_destroy(mongodb_client);

            snprintf(log_buf_, sizeof(log_buf_), "[%s] 关联规则[%s] 未过期 return 0. line[%d]", __FUNCTION__, guanlian_rule, __LINE__);
            writelog(log_buf_);

            return 0;
        }
        else if(mil_in > mil_stop) //已过期
        {
            mongoc_collection_destroy(mongodb_table_car);
            mongoc_collection_destroy(mongodb_table_chargerule);
          //  mongoc_client_destroy(mongodb_client);
            if(strcmp("免费",guanlian_rule) == 0) //关联免费
            {
                snprintf(log_buf_, sizeof(log_buf_), "[%s] 关联规则[%s] 已过期 return 0. line[%d]", __FUNCTION__, guanlian_rule, __LINE__);
                writelog(log_buf_);

                return 0;
            }
            else if(strcmp("白天按小时收费，晚上按次收费",guanlian_rule) == 0) {//关联 白天按小时收费，晚上按次收费
                int ret = mongodb_day_hour_night_ci(id, car_type, in_time, out_time);

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 过期 进入关联规则 guanlian_rule[%s] ret[%d] line[%d]", __FUNCTION__, guanlian_rule, ret, __LINE__);
                writelog(log_buf_);

                return ret;
            }
            else if(strcmp("白天按小时收费，晚上按小时收费",guanlian_rule) == 0) { //关联 白天按小时收费，晚上按小时收费

                // 吉华大厦定制：过期时，出场费用为 总费用（出场减去入场时间段内费用） 减去 非过期费用时费用（有效期时间）
                //int ret = mongodb_day_hour_night_hour(id, car_type, in_time, out_time);
                // 把入场时间 in_time, 改为stop_time
                int ret = mongodb_day_hour_night_hour(id, car_type, stop_time, out_time);

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 过期 进入关联规则 guanlian_rule[%s] ret[%d] in_time[%s] stop_time[%s] out_time[%s] line[%d]",
                         __FUNCTION__, guanlian_rule, ret, in_time, stop_time, out_time, __LINE__);
                writelog(log_buf_);

                return ret;
            }
            else if(strcmp("全天按小时收费",guanlian_rule) == 0) { //关联 全天按小时收费
                int ret = mongodb_calfee_all_day_by_hour(id, car_type, in_time, out_time);

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 过期 进入关联规则 guanlian_rule[%s] ret[%d] line[%d]", __FUNCTION__, guanlian_rule, ret, __LINE__);
                writelog(log_buf_);

                return ret;
            }
            else if(strcmp("济南2017", guanlian_rule) == 0 ){

                int ret = mongodb_jinan_2017(id,tonggang,car_type,in_time,out_time);

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 过期 进入关联规则 guanlian_rule[%s] ret[%d] line[%d]", __FUNCTION__, guanlian_rule, ret, __LINE__);
                writelog(log_buf_);

                return ret;
            }
            else
            {
                fprintf(fp_log,"%s## error! 没有找到关联收费规则! line[%d]\n", __FUNCTION__, __LINE__);

                snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 没有找到关联收费规则! guanlian_rule[%s] line[%d]", __FUNCTION__, guanlian_rule, __LINE__);
                writelog(log_buf_);

                return 0;
            }
        }
        else
        {
            mongoc_collection_destroy(mongodb_table_car);
            mongoc_collection_destroy(mongodb_table_chargerule);
        //    mongoc_client_destroy(mongodb_client);

            snprintf(log_buf_, sizeof(log_buf_), "[%s] error! mil_in[%d] mil_stop[%d] line[%d]", __FUNCTION__, mil_in, mil_stop, __LINE__);
            writelog(log_buf_);

            return 0;
        }

    }
    else if(strcmp(rule,"白天按小时收费，晚上按次收费") == 0) //收费规则为白天按小时收费，晚上按次收费
    {
        mongoc_collection_destroy(mongodb_table_car);
        mongoc_collection_destroy(mongodb_table_chargerule);
    //    mongoc_client_destroy(mongodb_client);
        fprintf(fp_log,"%s##收费规则为%s,进入时间为%s,离开时间为%s,id为 %s\n",time_now,rule,in_time,out_time,id);
        int ret = mongodb_day_hour_night_ci(id,car_type,in_time,out_time);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 收费 ret[%d] line[%d]", __FUNCTION__, ret, __LINE__);
        writelog(log_buf_);

        return ret;
    }
    else if(strcmp(rule,"济南2017") == 0) //济南2017
    {
        mongoc_collection_destroy(mongodb_table_car);
        mongoc_collection_destroy(mongodb_table_chargerule);
      //  mongoc_client_destroy(mongodb_client);
        fprintf(fp_log,"%s##收费规则为%s,进入时间为%s,离开时间为%s,id为 %s\n",time_now,rule,in_time,out_time,id);

        int ret = mongodb_jinan_2017(id,tonggang,car_type,in_time,out_time);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 收费 ret[%d] line[%d]", __FUNCTION__, ret, __LINE__);
        writelog(log_buf_);

        return ret;
    }
    else if(strcmp(rule,"白天按小时收费，晚上按小时收费") == 0) //收费规则为白天按小时收费，晚上按小时收费
    {
        mongoc_collection_destroy(mongodb_table_car);
        mongoc_collection_destroy(mongodb_table_chargerule);
    //    mongoc_client_destroy(mongodb_client);
        fprintf(fp_log,"%s##收费规则为%s,进入时间为%s,离开时间为%s\n",time_now,rule,in_time,out_time);
        int ret = mongodb_day_hour_night_hour(id,car_type,in_time,out_time);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 收费 ret[%d] line[%d]", __FUNCTION__, ret, __LINE__);
        writelog(log_buf_);

        return ret;
    }
    else if(strcmp(rule,"储时") == 0) //收费规则为储时
    {
        char remain_time[24];
        int mil_now = get_tick(time_now); //得到当前时间的秒数
        int mil_in = get_tick(in_time);	 //得到进入时间的秒数
        char guanlian_rule[256];

        fprintf(fp_log,"%s##收费规则为%s\n",time_now,rule);
        /******************************查询剩余时间***********************************************/
        query = bson_new();

        BSON_APPEND_UTF8(query, "car_plate_id",plate); //查询条件

        cursor = mongoc_collection_find (mongodb_table_car, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
        if(mongoc_cursor_error(cursor,&error))
        {
            bson_destroy(query);

            mongoc_cursor_destroy(cursor);

            fprintf(fp_log,"%s##mongodb_cal_fee 失败\n",time_now);
            return -1;
        }
        if(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor))
        {
            if (mongoc_cursor_next (cursor, &tmp1))
            {
                bson_copy_to(tmp1,&result); //得到一条完整的记录
                if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "car_remain_time"))
                {
                    tmp = bson_iter_utf8(&iter,&length);
                    memset(remain_time,0,24);
                    memcpy(remain_time,tmp,length);
                }

                bson_destroy(&result);
            }
        }
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);

        /**************************************************end******************************************/

        /******************************查询关联收费规则***********************************************/
        query = bson_new();

        BSON_APPEND_UTF8(query, "chargerule_park_id",id); //查询条件
        BSON_APPEND_UTF8(query, "chargerule_car_type",car_type); //查询条件
        cursor = mongoc_collection_find (mongodb_table_chargerule, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
        if(mongoc_cursor_error(cursor,&error))
        {
            bson_destroy(query);

            mongoc_cursor_destroy(cursor);

            fprintf(fp_log,"%s##mongodb_cal_fee 失败\n",time_now);
            return -1;
        }
        if(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor))
        {
            if (mongoc_cursor_next (cursor, &tmp1))
            {
                bson_copy_to(tmp1,&result); //得到一条完整的记录
                if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "chargerule_guanlian_id"))
                {
                    tmp = bson_iter_utf8(&iter,&length);
                    memset(guanlian_rule,0,256);
                    memcpy(guanlian_rule,tmp,length);
                }
                bson_destroy(&result);
            }
        }
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);
        /**************************************************end******************************************/

        int time_final = 0;
        int money_final = 0;

        if((mil_now - mil_in)/60 <= atoi(remain_time)) //剩余时间充足
        {
            time_final = atoi(remain_time) - (mil_now - mil_in)/60;
            money_final = 0;
        }
        else if(atoi(remain_time) < (mil_now - mil_in)/60 && atoi(remain_time) != 0) //剩余时间小于该次停留时长
        {
            time_final = 0;
            money_final = 0;
        }
        else if(atoi(remain_time) == 0) //剩余时间为0
        {
            time_final = 0;
            if(strcmp("免费",guanlian_rule) == 0) //关联 免费
            {
                money_final = 0;
            }
            else if(strcmp("白天按小时收费，晚上按次收费",guanlian_rule) == 0) //关联 白天按小时收费，晚上按次收费
                money_final = mongodb_day_hour_night_ci(id,car_type,in_time,out_time);
            else if(strcmp("全天按小时收费",guanlian_rule) == 0)
                money_final = mongodb_calfee_all_day_by_hour(id,car_type,in_time,out_time); //关联 全天按小时收费
            else
                money_final = 0;
        }else{
            snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 未处理逻辑 rule[%s] guanlian_rule[%s] remain_time[%s] line[%d]", __FUNCTION__, rule, guanlian_rule, remain_time, __LINE__);
            writelog(log_buf_);
        }

        query = bson_new();

        BSON_APPEND_UTF8(query, "car_plate_id",plate); //查询条件
        memset(remain_time,0,24);
        sprintf(remain_time,"%d",time_final);
        bson_t *result1 = bson_new();
        result1 = BCON_NEW ("$set", "{", "car_remain_time", BCON_UTF8 (remain_time),"updated", BCON_BOOL (true) ,"}");

        mongoc_collection_update(mongodb_table_car,MONGOC_UPDATE_NONE,query,result1,NULL,&error); //更新car表里的车辆剩余时间
        bson_destroy(query);
        bson_destroy(result1);
        mongoc_collection_destroy(mongodb_table_car);
        mongoc_collection_destroy(mongodb_table_chargerule);
     //   mongoc_client_destroy(mongodb_client);
        return money_final;

    }
    else if(strcmp(rule,"储值") == 0) //收费规则为储值
    {
        char remain_money[24];
        char guanlian_rule[256];
        fprintf(fp_log,"%s##收费规则为%s\n",time_now,rule);
        /******************************查询剩余金额***********************************************/
        query = bson_new();

        BSON_APPEND_UTF8(query, "car_plate_id",plate); //查询条件

        cursor = mongoc_collection_find (mongodb_table_car, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
        if(mongoc_cursor_error(cursor,&error))
        {
            bson_destroy(query);

            mongoc_cursor_destroy(cursor);

            fprintf(fp_log,"%s##mongodb_cal_fee 失败\n",time_now);
            return -1;
        }
        if(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor))
        {
            if (mongoc_cursor_next (cursor, &tmp1))
            {
                bson_copy_to(tmp1,&result); //得到一条完整的记录
                if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "car_remain_money"))
                {
                    tmp = bson_iter_utf8(&iter,&length);
                    memset(remain_money,0,24);
                    memcpy(remain_money,tmp,length);
                }
                bson_destroy(&result);

            }
        }
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);

        /**************************************************end******************************************/

        /******************************查询关联收费规则***********************************************/
        query = bson_new();

        BSON_APPEND_UTF8(query, "chargerule_park_id",id); //查询条件
        BSON_APPEND_UTF8(query, "chargerule_car_type",car_type); //查询条件
        cursor = mongoc_collection_find (mongodb_table_chargerule, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
        if(mongoc_cursor_error(cursor,&error))
        {
            bson_destroy(query);

            mongoc_cursor_destroy(cursor);

            fprintf(fp_log,"%s##mongodb_cal_fee 失败\n",time_now);
            return -1;
        }
        if(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor))
        {
            if (mongoc_cursor_next (cursor, &tmp1))
            {
                bson_copy_to(tmp1,&result); //得到一条完整的记录
                if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "chargerule_guanlian_id"))
                {
                    tmp = bson_iter_utf8(&iter,&length);
                    memset(guanlian_rule,0,256);
                    memcpy(guanlian_rule,tmp,length);
                }
                bson_destroy(&result);
            }
        }
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);
        /**************************************************end******************************************/
        int charge = 0;
        int money_final = 0;

        if(strcmp("免费",guanlian_rule) == 0) //关联 免费
        {
            money_final = atoi(remain_money);
            charge = 0;
        }
        else if(strcmp("白天按小时收费，晚上按次收费",guanlian_rule) == 0) //关联 白天按小时收费，晚上按次收费
        {
            charge = mongodb_day_hour_night_ci(id,car_type,in_time,out_time);
            if(charge <= atoi(remain_money))
            {
                money_final = atoi(remain_money) - charge;
                charge = 0;
            }
            else if(atoi(remain_money) < charge && atoi(remain_money) != 0)
            {
                charge = 0;
                money_final = 0;
            }
            else
            {
                money_final = 0;
            }
        }
        else if(strcmp("全天按小时收费",guanlian_rule) == 0) //关联 全天按小时收费
        {
            charge = mongodb_calfee_all_day_by_hour(id, car_type, in_time, out_time);
            if(charge <= atoi(remain_money))
            {
                money_final = atoi(remain_money) - charge;
                charge = 0;
            }
            else if(atoi(remain_money) < charge && atoi(remain_money) != 0)
            {
                charge = 0;
                money_final = 0;
            }
            else
            {
                money_final = 0;
            }
        }else{
            fprintf(fp_log,"%s## error! 没有找到关联收费规则! line[%d]\n", __FUNCTION__, __LINE__);

            snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 没有找到关联收费规则! guanlian_rule[%s] line[%d]", __FUNCTION__, guanlian_rule, __LINE__);
            writelog(log_buf_);
        }

        query = bson_new();

        BSON_APPEND_UTF8(query, "car_plate_id",plate); //查询条件
        memset(remain_money,0,24);
        sprintf(remain_money,"%d",money_final);
        bson_t *result1;
        result1 = BCON_NEW ("$set", "{", "car_remain_money", BCON_UTF8 (remain_money),"updated", BCON_BOOL (true) ,"}");

        mongoc_collection_update(mongodb_table_car,MONGOC_UPDATE_NONE,query,result1,NULL,&error); //更新car表里的车辆剩余金额

        bson_destroy(query);
        bson_destroy(result1);
        mongoc_collection_destroy(mongodb_table_car);
        mongoc_collection_destroy(mongodb_table_chargerule);
    //    mongoc_client_destroy(mongodb_client);
        return charge;

    }
    else if(strcmp(rule,"全天按小时收费") == 0) //收费规则为全天按小时收费
    {
        fprintf(fp_log,"%s##收费规则为%s\n",time_now,rule);
        mongoc_collection_destroy(mongodb_table_car);
        mongoc_collection_destroy(mongodb_table_chargerule);

   //     mongoc_client_destroy(mongodb_client);
        int ret = mongodb_calfee_all_day_by_hour(id,car_type,in_time,out_time);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 收费ret[%d] line[%d]", __FUNCTION__, ret, __LINE__);
        writelog(log_buf_);

        return ret;
    }else{
        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 没有找到收费方案! rule[%s] line[%d]", __FUNCTION__, rule, __LINE__);
        writelog(log_buf_);
    }
}

/******************************处理上级停车场***********************************************/
int mongodb_process_top_park(char * id,char *plate,char * charge_rule,char *car_type,car_msg *msg,char *open_door)
{
   // mongoc_client_t *mongodb_client;
    mongoc_collection_t *mongodb_table_channel;	//channel表
    mongoc_collection_t *mongodb_table_park;	//device表
    mongoc_collection_t *mongodb_table_chargerule;	//device表
    mongoc_collection_t *mongodb_table_carinpark;	//device表
    mongoc_collection_t *mongodb_table_caroutrec;	//device表
  //  mongodb_client = mongoc_client_new(str_con);
    mongodb_table_channel = mongoc_client_get_collection(mongodb_client,"boondb","channel");   //channel表
    mongodb_table_park = mongoc_client_get_collection(mongodb_client,"boondb","park");   //device表
    mongodb_table_chargerule = mongoc_client_get_collection(mongodb_client,"boondb","chargerule");   //device表
    mongodb_table_carinpark = mongoc_client_get_collection(mongodb_client,"boondb","carinpark");   //device表
    mongodb_table_caroutrec = mongoc_client_get_collection(mongodb_client,"boondb", "caroutrec");

    char res[24];
    char park_id[256];
    char park_parent_id[256];

    char car_num[24];
    char in_time[24];
    char parent_charge_rule[256];
    mongoc_cursor_t *cursor;
    bson_t *query;
    bson_t result;
    const char *tmp;
    const bson_t *tmp1;
    bson_iter_t iter;
    bson_error_t error;
    unsigned int length;
    int chewei_count = 0;
    int carinpark_count = 0;
    int count = 0;


    char time_now[64];
    time_t tm;
    time_printf(time(&tm),time_now);

    query = bson_new();


    fprintf(fp_log,"%s##处理上级停车场\n",time_now);
    /******************************根据通道id查找停车场id***********************************************/
    BSON_APPEND_UTF8(query, "channel_id", id); //查询条件

    cursor = mongoc_collection_find (mongodb_table_channel, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);

        fprintf(fp_log,"%s##mongodb_process_top_park 失败\n",time_now);
        return -1;
    }
    if(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor))
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "channel_park_id"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(park_id,0,256);
                memcpy(park_id,tmp,length);
            }
            bson_destroy(&result);
        }
    }
    bson_destroy(query);

    mongoc_cursor_destroy(cursor);

    /******************************end***********************************************************/

    /******************************查询本停车场是否有上级停车场***********************************************/
    query = bson_new();


    BSON_APPEND_UTF8(query, "park_id", park_id); //查询条件
    cursor = mongoc_collection_find (mongodb_table_park, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);

        fprintf(fp_log,"%s##mongodb_process_top_park 失败\n",time_now);
        return -1;
    }
    if(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor)) //得到car表的每一条记录
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "park_parent_id"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(park_parent_id,0,256);
                memcpy(park_parent_id,tmp,length);
            }
            bson_destroy(&result);
        }
    }
    bson_destroy(query);

    mongoc_cursor_destroy(cursor);
    /******************************end***********************************************************/
    if(atoi(park_parent_id) == -1) //无上级停车场，则返回
    {
        fprintf(fp_log,"%s##无上级停车场\n",time_now);
        mongoc_collection_destroy(mongodb_table_channel);
        mongoc_collection_destroy(mongodb_table_park);
        mongoc_collection_destroy(mongodb_table_carinpark);
        mongoc_collection_destroy(mongodb_table_caroutrec);
        mongoc_collection_destroy(mongodb_table_chargerule);
     //   mongoc_client_destroy(mongodb_client);
        return 0;
    }
    else
    {
        fprintf(fp_log,"%s##有上级停车场\n",time_now);
    }

    /******************************根据车场id和车辆类型查询收费规则***********************************************/
    query = bson_new();


    BSON_APPEND_UTF8(query, "chargerule_park_id", park_parent_id); //查询条件
    BSON_APPEND_UTF8(query, "chargerule_car_type",car_type); //查询条件
    cursor = mongoc_collection_find (mongodb_table_chargerule, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);

        fprintf(fp_log,"%s##mongodb_process_top_park 失败\n",time_now);
        return -1;
    }
    if(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor)) //得到car表的每一条记录
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "chargerule_name_id"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(parent_charge_rule,0,256);
                memcpy(parent_charge_rule,tmp,length);
            }
            bson_destroy(&result);
        }
    }
    bson_destroy(query);

    mongoc_cursor_destroy(cursor);
    /******************************end***********************************************************/
    /******************************根据车场id和车牌号查询进入时间***********************************************/
    char in_pic_path[256];
    char in_channel_id[256];
    memset(in_pic_path,0,256);
    memset(in_channel_id,0,256);


    query = bson_new();

    BSON_APPEND_UTF8(query, "carinpark_park_id", park_parent_id); //查询条件
    BSON_APPEND_UTF8(query, "carinpark_plate_id", plate); //查询条件

    cursor = mongoc_collection_find (mongodb_table_carinpark, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);

        fprintf(fp_log,"%s##mongodb_process_top_park 失败\n",time_now);
        return -1;
    }
    if(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor)) //得到car表的每一条记录
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "carinpark_in_time"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(in_time,0,24);
                memcpy(in_time,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "carinpark_pic_path"))
            {
                tmp = bson_iter_utf8(&iter,&length);

                memcpy(in_pic_path,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "carinpark_channel_id"))
            {
                tmp = bson_iter_utf8(&iter,&length);

                memcpy(in_channel_id,tmp,length);
            }
            bson_destroy(&result);
        }
    }
    bson_destroy(query);

    mongoc_cursor_destroy(cursor);

    /******************************end***********************************************************/

    int fee = mongodb_cal_fee(parent_charge_rule,in_time,time_now,car_type,park_parent_id,plate); //计费
    char fee_tmp[24];
    memset(fee_tmp,0,24);
    sprintf(fee_tmp,"%d",fee);

    /******************************把计费结果写到caroutrec表***********************************************/
    query = bson_new();
    BSON_APPEND_UTF8(query, "caroutrec_in_time", in_time);
    BSON_APPEND_UTF8(query, "caroutrec_out_time", time_now);
    BSON_APPEND_UTF8(query, "caroutrec_pay_charge",fee_tmp);
    memset(fee_tmp,0,24);
    sprintf(fee_tmp,"%d",0);
    BSON_APPEND_UTF8(query, "caroutrec_real_charge",fee_tmp);
    BSON_APPEND_UTF8(query, "caroutrec_park_id",park_id);
    BSON_APPEND_UTF8(query, "caroutrec_plate_id",plate);
    BSON_APPEND_UTF8(query, "caroutrec_in_pic_path",in_pic_path);
    BSON_APPEND_UTF8(query, "caroutrec_out_pic_path",msg->path);
    BSON_APPEND_UTF8(query, "caroutrec_car_type",car_type);
    BSON_APPEND_UTF8(query, "caroutrec_in_channel_id",in_channel_id);
    BSON_APPEND_UTF8(query, "caroutrec_out_channel_id",msg->channel_id);
    BSON_APPEND_UTF8(query, "caroutrec_open_door_type",open_door);
    char stay_time[24];
    memset(stay_time,0,24);
    sprintf(stay_time,"%d",(get_tick(time_now) - get_tick(in_time))/60);
    BSON_APPEND_UTF8(query, "caroutrec_stay_time",stay_time);

    if(!mongoc_collection_insert (mongodb_table_caroutrec, MONGOC_INSERT_NONE, query, NULL, &error))
    {
        fprintf(fp_log,"%s##mongodb_process_top_park 失败\n",time_now);
        bson_destroy(query);
        return -1;
    }
    bson_destroy(query);
    /******************************end***********************************************************/

    query = bson_new();
    BSON_APPEND_UTF8(query, "carinpark_park_id", park_parent_id);
    BSON_APPEND_UTF8(query, "carinpark_plate_id", plate);

    mongoc_collection_remove (mongodb_table_carinpark, MONGOC_REMOVE_SINGLE_REMOVE,query, NULL, &error); //删除上级停车场该车辆的在场记录
    bson_destroy(query);
    mongoc_collection_destroy(mongodb_table_channel);
    mongoc_collection_destroy(mongodb_table_park);
    mongoc_collection_destroy(mongodb_table_carinpark);
    mongoc_collection_destroy(mongodb_table_caroutrec);
    mongoc_collection_destroy(mongodb_table_chargerule);
   // mongoc_client_destroy(mongodb_client);
    return 0;
}

int mongodb_process_top_park_out(char * id,char *plate,char * charge_rule,char *car_type,char *park_parent_id)
{
  //  mongoc_client_t *mongodb_client;
    mongoc_collection_t *mongodb_table_channel;	//channel表
    mongoc_collection_t *mongodb_table_park;	//device表
    mongoc_collection_t *mongodb_table_chargerule;	//device表
    mongoc_collection_t *mongodb_table_carinpark;	//device表
    mongoc_collection_t *mongodb_table_caroutrec;	//device表
  //  mongodb_client = mongoc_client_new(str_con);
    mongodb_table_channel = mongoc_client_get_collection(mongodb_client,"boondb","channel");   //channel表
    mongodb_table_park = mongoc_client_get_collection(mongodb_client,"boondb","park");   //device表
    mongodb_table_chargerule = mongoc_client_get_collection(mongodb_client,"boondb","chargerule");   //device表
    mongodb_table_carinpark = mongoc_client_get_collection(mongodb_client,"boondb","carinpark");   //device表
    mongodb_table_caroutrec = mongoc_client_get_collection(mongodb_client,"boondb","caroutrec");   //device表

    char res[24];
    char park_id[256];

    char people_name[24];

    char car_num[24];
    char in_time[24];
    char parent_charge_rule[256];
    mongoc_cursor_t *cursor;
    bson_t *query;
    bson_t result;
    const char *tmp;
    const bson_t *tmp1;
    bson_iter_t iter;
    bson_error_t error;
    unsigned int length;
    int chewei_count = 0;
    int carinpark_count = 0;
    int count = 0;

    char time_now[64];
    time_t tm;
    time_printf(time(&tm),time_now);


    memset(park_parent_id,0,256);
    query = bson_new();

    /******************************根据通道id查找停车场id***********************************************/
    BSON_APPEND_UTF8(query, "channel_id", id); //查询条件

    cursor = mongoc_collection_find (mongodb_table_channel, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);

        fprintf(fp_log,"%s##mongodb_process_top_park 失败\n",time_now);
        return -1;
    }
    while(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor))
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "channel_park_id"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(park_id,0,256);
                memcpy(park_id,tmp,length);
            }
            bson_destroy(&result);
        }
    }
    bson_destroy(query);

    mongoc_cursor_destroy(cursor);

    /******************************end***********************************************************/

    /******************************查询本停车场是否有上级停车场***********************************************/
    query = bson_new();


    BSON_APPEND_UTF8(query, "park_id", park_id); //查询条件
    cursor = mongoc_collection_find (mongodb_table_park, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);

        fprintf(fp_log,"%s##mongodb_process_top_park 失败\n",time_now);
        return -1;
    }
    if(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor))
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "park_parent_id"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memcpy(park_parent_id,tmp,length);

            }
            bson_destroy(&result);
        }
    }
    bson_destroy(query);

    mongoc_cursor_destroy(cursor);
    mongoc_collection_destroy(mongodb_table_channel);
    mongoc_collection_destroy(mongodb_table_park);
    mongoc_collection_destroy(mongodb_table_carinpark);
    mongoc_collection_destroy(mongodb_table_caroutrec);
    mongoc_collection_destroy(mongodb_table_chargerule);
  //  mongoc_client_destroy(mongodb_client);
    /******************************end***********************************************************/
    if(atoi(park_parent_id) == -1 || strlen(park_parent_id) < 10) //无上级停车场，则返回
    {
        fprintf(fp_log,"%s##无上级停车场\n",time_now);
        return 0;
    }
    else
    {
        fprintf(fp_log,"%s##有上级停车场,上级停车场id为%s\n",time_now,park_parent_id);
        return 1;
    }
}
/******************************end***********************************************************/

/******************************写数据到入口表和在场表***********************************************/
int mongodb_write_in_park(car_msg *in_msg,char *ori_plate,char *final_plate,char *park_id,char *global_car_type,bool *auth,char *open_door)
{
   // mongoc_client_t *mongodb_client;
    mongoc_collection_t *mongodb_table_car;	//channel表
    mongoc_collection_t *mongodb_table_carinrec;	//device表
    mongoc_collection_t *mongodb_table_carinpark;	//device表
  //  mongodb_client = mongoc_client_new(str_con);
    mongodb_table_car = mongoc_client_get_collection(mongodb_client,"boondb","car");   //channel表
    mongodb_table_carinrec = mongoc_client_get_collection(mongodb_client,"boondb","carinrec");   //device表
    mongodb_table_carinpark = mongoc_client_get_collection(mongodb_client,"boondb","carinpark");   //device表

    mongoc_cursor_t *cursor;
    bson_t *query;
    bson_t result;

    const char *tmp;
    const bson_t *tmp1;
    bson_iter_t iter;
    bson_error_t error;
    unsigned int length;

    char time_stamp[24];

    query = bson_new();

    char people_name[24];
    char time_now[64];
    time_t tm;
    time_printf(time(&tm),time_now);
    /******************************写数据到入口表***********************************************/
    BSON_APPEND_UTF8(query, "carinrec_in_time", in_msg->time);
    memset(time_stamp,0,24);
    sprintf(time_stamp,"%d",get_tick(in_msg->time));
    BSON_APPEND_UTF8(query, "carinrec_time_stamp",time_stamp);
    BSON_APPEND_UTF8(query, "carinrec_alg_plate",ori_plate);
    BSON_APPEND_UTF8(query, "carinrec_plate_id",final_plate);
    BSON_APPEND_UTF8(query, "carinrec_pic_path",in_msg->path);
    BSON_APPEND_UTF8(query, "carinrec_channel_id",in_msg->channel_id);
    BSON_APPEND_UTF8(query, "carinrec_park_id",park_id);
    BSON_APPEND_UTF8(query, "carinrec_car_type",global_car_type);
    BSON_APPEND_UTF8(query, "carinrec_open_door_type",open_door);
    BSON_APPEND_UTF8(query, "carinrec_car_logo",in_msg->brand);
    BSON_APPEND_UTF8(query, "carinrec_car_color",in_msg->color);
    BSON_APPEND_UTF8(query, "carinrec_aux_plate_id",in_msg->plate1);
    BSON_APPEND_UTF8(query, "carinrec_aux_alg_plate",in_msg->plate1);
    BSON_APPEND_UTF8(query, "carinrec_aux_car_logo",in_msg->brand1);
    BSON_APPEND_UTF8(query, "carinrec_aux_car_color",in_msg->color1);

    if(!mongoc_collection_insert (mongodb_table_carinrec, MONGOC_INSERT_NONE, query, NULL, &error))
    {
        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 插入数据失败 line[%d]", __FUNCTION__, __LINE__);
        writelog(log_buf_);

        fprintf(fp_log,"%s##mongodb_write_in_park 失败\n",time_now);
        bson_destroy(query);
        return -1;
    }

    bson_destroy(query);
    fprintf(fp_log,"%s##写数据到入口表\n",time_now);
    /******************************end************************************/

    if(strcmp(final_plate,"无车牌") == 0) //无车牌不往在场表写
    {
        mongoc_collection_destroy(mongodb_table_car);
        mongoc_collection_destroy(mongodb_table_carinrec);
        mongoc_collection_destroy(mongodb_table_carinpark);
    //    mongoc_client_destroy(mongodb_client);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 无车牌不需要写在场表 return 0! line[%d]", __FUNCTION__, __LINE__);
        writelog(log_buf_);

        return 0;
    }
    /******************************查询车主姓名***********************************************/
    query = bson_new();

    BSON_APPEND_UTF8(query, "car_plate_id",final_plate);
    memset(people_name,0,24);
    cursor = mongoc_collection_find (mongodb_table_car, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);

        fprintf(fp_log,"%s##mongodb_write_in_park 失败\n",time_now);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 查找数据库失败 return -1! line[%d]", __FUNCTION__, __LINE__);
        writelog(log_buf_);

        return -1;
    }

    bool find_value = false;
    while(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor)) //得到car表的每一条记录
    {
        if (mongoc_cursor_next(cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "car_name")) //得到车牌号
            {
                tmp = bson_iter_utf8(&iter,&length);
                memcpy(people_name,tmp,length);

                find_value = true;
            }
            bson_destroy(&result);
        }
    }
    bson_destroy(query);
    mongoc_cursor_destroy(cursor);

    if(!find_value){
        snprintf(log_buf_, sizeof(log_buf_), "[%s] warn! 没有在car表中找到car_name记录 查找条件car_plate_id[%s] line[%d]", __FUNCTION__, final_plate, __LINE__);
        writelog(log_buf_);
    }
    /******************************end************************************/

    /******************************写数据到在场表***********************************************/
    query = bson_new();
    BSON_APPEND_UTF8(query, "carinpark_in_time", in_msg->time);
    BSON_APPEND_UTF8(query, "carinpark_time_stamp",time_stamp);
    BSON_APPEND_UTF8(query, "carinpark_alg_plate",ori_plate);
    BSON_APPEND_UTF8(query, "carinpark_plate_id",final_plate);
    BSON_APPEND_UTF8(query, "carinpark_pic_path",in_msg->path);
    BSON_APPEND_UTF8(query, "carinpark_channel_id",in_msg->channel_id);
    BSON_APPEND_UTF8(query, "carinpark_park_id",park_id);
    BSON_APPEND_UTF8(query, "carinpark_car_type",global_car_type);
    BSON_APPEND_UTF8(query, "carinpark_car_logo",in_msg->brand);
    BSON_APPEND_UTF8(query, "carinpark_car_color",in_msg->color);
    BSON_APPEND_UTF8(query, "carinpark_aux_plate_id",in_msg->plate1);
    BSON_APPEND_UTF8(query, "carinpark_aux_alg_plate",in_msg->plate1);
    BSON_APPEND_UTF8(query, "carinpark_aux_car_logo",in_msg->brand1);
    BSON_APPEND_UTF8(query, "carinpark_aux_car_color",in_msg->color1);
    BSON_APPEND_UTF8(query, "carinpark_people_name",people_name);

    if(!mongoc_collection_insert (mongodb_table_carinpark, MONGOC_INSERT_NONE, query, NULL, &error))
    {
        fprintf(fp_log,"%s##mongodb_write_in_park 失败\n",time_now);
        bson_destroy(query);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 写carinpark表失败 return -1! line[%d]", __FUNCTION__, __LINE__);
        writelog(log_buf_);

        return -1;
    }
    fprintf(fp_log,"%s##写数据到在场表\n",time_now);
    bson_destroy(query);
    mongoc_collection_destroy(mongodb_table_car);
    mongoc_collection_destroy(mongodb_table_carinrec);
    mongoc_collection_destroy(mongodb_table_carinpark);
  //  mongoc_client_destroy(mongodb_client);
    /******************************end************************************/

    snprintf(log_buf_, sizeof(log_buf_), "[%s] 写数据至carinpark表 车牌号carinpark_plate_id[%s] line[%d]", __FUNCTION__, final_plate, __LINE__);
    writelog(log_buf_);

    return 0;

}
/******************************end***********************************************************/

/******************************根据数据库配置，拼接LED显示信息***********************************************/
int mongodb_process_led(char *plate,char* _in_show_type,char * show_text,char *msg,char * car_time)
{
    snprintf(log_buf_, sizeof(log_buf_), "[%s] line[%d]", __FUNCTION__, __LINE__);
    writelog(log_buf_);

   // mongoc_client_t *mongodb_client;
    mongoc_collection_t *mongodb_table_car;	//channel表
    mongoc_collection_t *mongodb_table_park;	//device表
    mongoc_collection_t *mongodb_table_carinpark;	//device表
    mongoc_collection_t *mongodb_table_channel;	//device表
  //  mongodb_client = mongoc_client_new(str_con);
    mongodb_table_car = mongoc_client_get_collection(mongodb_client,"boondb","car");   //channel表
    mongodb_table_park = mongoc_client_get_collection(mongodb_client,"boondb","park");   //device表
    mongodb_table_carinpark = mongoc_client_get_collection(mongodb_client,"boondb","carinpark");   //device表
    mongodb_table_channel = mongoc_client_get_collection(mongodb_client,"boondb","channel");   //device表

    mongoc_cursor_t *cursor;
    bson_t *query;
    bson_t result;

    const char *tmp;
    const bson_t *tmp1;
    bson_iter_t iter;
    bson_error_t error;
    unsigned int length;

    char channel_id[256];
    char park_id[256];

    char time_now[64];
    time_t tm;
    time_printf(time(&tm),time_now);

    memset(park_id,0,256);
    memset(channel_id,0,256);

    query = bson_new();

    BSON_APPEND_UTF8(query, "channel_ip",host_ip);

    cursor = mongoc_collection_find (mongodb_table_channel, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);

        fprintf(fp_log,"%s##mongodb_process_led 失败\n",time_now);
        return -1;
    }
    while(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor)) //得到car表的每一条记录
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "channel_park_id")) //得到车牌号
            {
                tmp = bson_iter_utf8(&iter,&length);

                memcpy(park_id,tmp,length);
            }
            bson_destroy(&result);
        }
    }
    bson_destroy(query);

    mongoc_cursor_destroy(cursor);


    if(strcmp(_in_show_type,"固定内容") == 0)
    {
        sprintf(msg,"%s",show_text);
    }
    else if(strcmp(_in_show_type,"剩余车位") == 0)
    {
        char remain[24];

        memset(remain,0,24);
        query = bson_new();

        BSON_APPEND_UTF8(query, "park_id",park_id);
        cursor = mongoc_collection_find (mongodb_table_park, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
        if(mongoc_cursor_error(cursor,&error))
        {
            bson_destroy(query);

            mongoc_cursor_destroy(cursor);

            fprintf(fp_log,"%s##mongodb_process_led 失败\n",time_now);
            return -1;
        }
        while(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor)) //得到car表的每一条记录
        {
            if (mongoc_cursor_next (cursor, &tmp1))
            {
                bson_copy_to(tmp1,&result); //得到一条完整的记录
                if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "park_space_count_remain")) //得到车牌号
                {
                    tmp = bson_iter_utf8(&iter,&length);

                    memcpy(remain,tmp,length);
                }

            }
        }
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);

        sprintf(msg,"剩余车位%s",remain);
    }
    else if(strcmp(_in_show_type,"日期") == 0)
    {
        char tmp[24];
        memset(tmp,0,24);
        memcpy(tmp,time_now,10);
        sprintf(msg,"%s",tmp);
    }
    else if(strcmp(_in_show_type,"时间") == 0)
    {
        char tmp[24];
        memset(tmp,0,24);
        memcpy(tmp,time_now + 10 ,9);
        sprintf(msg,"%s",tmp);
    }
    else if(strcmp(_in_show_type,"入场时间") == 0)
    {
        char time_ru[24];
        memset(time_ru,0,24);
        query = bson_new();

        BSON_APPEND_UTF8(query, "carinpark_park_id",park_id);
        BSON_APPEND_UTF8(query, "carinpark_plate_id",plate);

        cursor = mongoc_collection_find (mongodb_table_carinpark, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
        if(mongoc_cursor_error(cursor,&error))
        {
            bson_destroy(query);

            mongoc_cursor_destroy(cursor);

            fprintf(fp_log,"%s##mongodb_process_led 失败\n",time_now);
            return -1;
        }
        while(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor))
        {
            if (mongoc_cursor_next (cursor, &tmp1))
            {
                bson_copy_to(tmp1,&result); //得到一条完整的记录
                if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "carinpark_in_time")) //得到车牌号
                {
                    tmp = bson_iter_utf8(&iter,&length);

                    memcpy(time_ru,tmp,length);
                }
                bson_destroy(&result);
            }
        }
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);

        sprintf(msg,"%s",time_ru);
        if(strlen(time_ru) < 2)
            sprintf(msg,"%s","未找到入场时间");
    }
    else if(strcmp(_in_show_type,"车类型") == 0)
    {
        char car_type[24];
        memset(car_type,0,24);
        query = bson_new();


        BSON_APPEND_UTF8(query, "car_plate_id",plate);

        cursor = mongoc_collection_find (mongodb_table_car, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
        if(mongoc_cursor_error(cursor,&error))
        {
            bson_destroy(query);

            mongoc_cursor_destroy(cursor);

            fprintf(fp_log,"%s##mongodb_process_led 失败\n",time_now);
            return -1;
        }
        while(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor))
        {
            if (mongoc_cursor_next (cursor, &tmp1))
            {
                bson_copy_to(tmp1,&result); //得到一条完整的记录
                if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "car_type"))
                {
                    tmp = bson_iter_utf8(&iter,&length);

                    memcpy(car_type,tmp,length);
                }
                bson_destroy(&result);
            }
        }
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);
        if(strlen(car_type) < 2)
            strcpy(car_type,"临时车");

        sprintf(msg,"%s",car_type);
    }
    else if(strcmp(_in_show_type,"车牌") == 0)
    {
        sprintf(msg,"%s",plate);
    }
    else if(strcmp(_in_show_type,"收费") == 0)
    {
        sprintf(msg,"收费%d元",fmoney);
    }
    else if(strcmp(_in_show_type,"停车时长") == 0)
    {
        char time_stay[24];
        memset(time_stay,0,24);
        query = bson_new();

        BSON_APPEND_UTF8(query, "carinpark_park_id",park_id);
        BSON_APPEND_UTF8(query, "carinpark_plate_id",plate);

        cursor = mongoc_collection_find (mongodb_table_carinpark, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
        if(mongoc_cursor_error(cursor,&error))
        {
            bson_destroy(query);

            mongoc_cursor_destroy(cursor);

            fprintf(fp_log,"%s##mongodb_process_led 失败\n",time_now);
            return -1;
        }
        while(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor))
        {
            if (mongoc_cursor_next (cursor, &tmp1))
            {
                bson_copy_to(tmp1,&result); //得到一条完整的记录
                if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "carinpark_in_time")) //得到车牌号
                {
                    tmp = bson_iter_utf8(&iter,&length);

                    memcpy(time_stay,tmp,length);
                }
                bson_destroy(&result);
            }
        }
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);

        int hour = 0;
        if(strlen(time_stay) > 6)
        {
            int mil = (get_tick(car_time) - get_tick(time_stay))/60;
            if(mil > 60)
            {
                hour = floor(mil /60) + 1;
                sprintf(msg,"停车%d小时",hour-1);
		        int minu = (mil -  (hour - 1)*60);
                sprintf(msg,"%s%d分钟",msg,minu);
            }
            else
            {
                sprintf(msg,"停车%d分钟",mil);
            }
        }
        else
        {
            sprintf(msg,"未找到入场时间");
        }
    }
    mongoc_collection_destroy(mongodb_table_car);
    mongoc_collection_destroy(mongodb_table_channel);
    mongoc_collection_destroy(mongodb_table_park);
    mongoc_collection_destroy(mongodb_table_carinpark);
  //  mongoc_client_destroy(mongodb_client);
    return 0;
}
/******************************end***********************************************************/

/******************************给bled发送led显示信息和语音信息***********************************************/
int mongodb_send_bled(car_msg *msg, char *flag, char *car_type, char *plate, char *id, char *speak_send, int money, int park_time)
{
    snprintf(log_buf_, sizeof(log_buf_), "[%s] 准备向bled发送消息 speak_send[%s] line[%d]", __FUNCTION__, speak_send, __LINE__);
    writelog(log_buf_);

    printf("mongodb_send_bled===========start========\n");
 //   mongoc_client_t *mongodb_client;
    mongoc_collection_t *mongodb_table_ledset;	//channel表
    mongoc_collection_t *mongodb_table_device;	//device表
//    mongodb_client = mongoc_client_new(str_con);
    mongodb_table_ledset = mongoc_client_get_collection(mongodb_client,"boondb","ledset");   //channel表
    mongodb_table_device = mongoc_client_get_collection(mongodb_client,"boondb","device");   //device表

    mongoc_cursor_t *cursor;
    bson_t *query;
    bson_t result;

    const char *tmp;
    const bson_t *tmp1;
    bson_iter_t iter;
    bson_error_t error;
    unsigned int length;

    char ledset_type[24] = {0};
    char ledset_show_type[24] = {0};
    char ledset_show_text[256] = {0};
    char num[24] = {0};

    char tmp_type[24] = {0};
    char led_ip[24] = {0};

    char num1_text[1024] = {0};
    char num2_text[1024] = {0};
    char num3_text[1024] = {0};
    char num4_text[1024] = {0};

    char time_now[64] = {0};
    time_t tm;
    time_printf(time(&tm),time_now);

    if(strcmp(flag,"入口") == 0)
    {
        strcpy(tmp_type,"入口有车");
    }
    else
    {
        strcpy(tmp_type,"出口有车");
    }

    query = bson_new();

    memset(num,0,24);
    sprintf(num,"%d",1);

    /******************************查询数据库，每一行该显示什么内容***********************************************/

    cursor = mongoc_collection_find (mongodb_table_ledset, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);
        mongoc_cursor_destroy(cursor);

        fprintf(fp_log,"%s##mongodb_send_bled 失败\n",time_now);
        return -1;
    }

    while(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor))
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "ledset_type")) //得到ledset_type
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(ledset_type,0,24);
                memcpy(ledset_type,tmp,length);
            }

            if(strcmp(ledset_type,tmp_type) != 0)
            {
                bson_destroy(&result);
                continue;
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "ledset_row_id"))  //得到ledset_row_id
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(num,0,24);
                memcpy(num,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "ledset_show_type"))   //得到ledset_show_type
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(ledset_show_type,0,24);
                memcpy(ledset_show_type,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "ledset_show_text")) //得到ledset_show_text
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(ledset_show_text,0,256);
                memcpy(ledset_show_text,tmp,length);
            }

            if(atoi(num) == 1)
            {

                mongodb_process_led(plate,ledset_show_type,ledset_show_text,num1_text,msg->time); //拼接第一行
                if(strcmp(flag,"入口") == 0)
                {
                    if(strcmp(ledset_show_type,"车类型") == 0)
                    {
                        if(in_led_guoqi_flag)
                            sprintf(num1_text,"%s已过期",num1_text);
                        if(in_led_noauth_flag)
                            sprintf(num1_text,"%s未授权车辆",num1_text);
                    }
                }
                else
                {
                    if(strcmp(ledset_show_type,"车类型") == 0)
                    {
                        if(out_led_guoqi_flag)
                            sprintf(num1_text,"%s已过期",num1_text);
                    }
                }
            }
            else if(atoi(num) == 2)
            {

                mongodb_process_led(plate,ledset_show_type,ledset_show_text,num2_text,msg->time); //拼接第二行
                if(strcmp(flag,"入口") == 0)
                {
                    if(strcmp(ledset_show_type,"车类型") == 0)
                    {
                        if(in_led_guoqi_flag)
                            sprintf(num2_text,"%s已过期",num2_text);
                        if(in_led_noauth_flag)
                            sprintf(num2_text,"%s未授权车辆",num2_text);
                    }
                }
                else
                {
                    if(strcmp(ledset_show_type,"车类型") == 0)
                    {
                        if(out_led_guoqi_flag)
                            sprintf(num2_text,"%s已过期",num2_text);
                    }
                }
            }
            else if(atoi(num) == 3)
            {

                mongodb_process_led(plate,ledset_show_type,ledset_show_text,num3_text,msg->time); //拼接第三行
                if(strcmp(flag,"入口") == 0)
                {
                    if(strcmp(ledset_show_type,"车类型") == 0)
                    {
                        if(in_led_guoqi_flag)
                            sprintf(num3_text,"%s已过期",num3_text);
                        if(in_led_noauth_flag)
                            sprintf(num3_text,"%s未授权车辆",num3_text);
                    }
                }
                else
                {
                    if(strcmp(ledset_show_type,"车类型") == 0)
                    {
                        if(out_led_guoqi_flag)
                            sprintf(num3_text,"%s已过期",num3_text);
                    }
                }
            }
            else if(atoi(num) == 4)
            {

                mongodb_process_led(plate,ledset_show_type,ledset_show_text,num4_text,msg->time); //拼接第四行
                if(strcmp(flag,"入口") == 0)
                {
                    if(strcmp(ledset_show_type,"车类型") == 0)
                    {
                        if(in_led_guoqi_flag)
                            sprintf(num4_text,"%s已过期",num4_text);
                        if(in_led_noauth_flag)
                            sprintf(num4_text,"%s未授权车辆",num4_text);
                    }
                }
                else
                {
                    if(strcmp(ledset_show_type,"车类型") == 0)
                    {
                        if(out_led_guoqi_flag)
                            sprintf(num4_text,"%s已过期",num4_text);
                    }
                }
            }



            bson_destroy(&result);

        }
    }
    bson_destroy(query);

    mongoc_cursor_destroy(cursor);
    /******************************end***********************************************************/

    /******************************查询数据库，找LED 的 ip***********************************************/
    query = bson_new();


    BSON_APPEND_UTF8(query, "device_type","LED语音一体机");
    BSON_APPEND_UTF8(query, "device_channel_id",id);

    cursor = mongoc_collection_find (mongodb_table_device, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);

        fprintf(fp_log,"%s##mongodb_send_bled 失败\n",time_now);
        return -1;
    }
    memset(led_ip,0,24);
    if(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor)) //得到car表的每一条记录
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "device_ip_id")) //得到车牌号
            {
                tmp = bson_iter_utf8(&iter,&length);

                memcpy(led_ip,tmp,length);
            }
            bson_destroy(&result);
        }
    }
    bson_destroy(query);

    mongoc_cursor_destroy(cursor);
    mongoc_collection_destroy(mongodb_table_ledset);
    mongoc_collection_destroy(mongodb_table_device);
  //  mongoc_client_destroy(mongodb_client);

    /******************************end***********************************************************/

    if(strlen(led_ip) < 0)
    {
        fprintf(fp_log,"%s##未找到ledip\n",time_now);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 没有找到ledip line[%d]", __FUNCTION__, __LINE__);
        writelog(log_buf_);

        return 0;
    }

    // 是否加入万能语音功能
    if(true == g_has_wanwangyuyin_){
        mongodb_speak_wannengyuyin(flag,plate,car_type,money,msg,speak_send);
    }


    printf("bled send ===============    %s\n",speak_send);
    /******************************按协议给bled发送LED显示信息和语音信息***********************************************/
    if(strcmp(flag,"入口") == 0)
    {
        memset(in_led_ip,0,24);
        memcpy(in_led_ip,led_ip,strlen(led_ip));
    }
    else
    {
        memset(out_led_ip,0,24);
        memcpy(out_led_ip,led_ip,strlen(led_ip));
    }
    Json::Value json_send;  //LED显示信息
    json_send["cmd"] = Json::Value("sendled");
    json_send["row_num"] = Json::Value("4");
    json_send["row1"] = Json::Value(num1_text);
    json_send["row2"] = Json::Value(num2_text);
    if(strcmp(msg->blacklist,""))
    {
        json_send["row3"] = Json::Value(msg->blacklist);
        json_send["row4"] = Json::Value(msg->blacklistreason);
    }
    else
    {
        json_send["row3"] = Json::Value(num3_text);
        json_send["row4"] = Json::Value(num4_text);
    }
    json_send["led_ip"] = Json::Value(led_ip);

    Json::Value json_send_speak; //语音信息
    json_send_speak["cmd"] = Json::Value("sendvoice");

    if(!strcmp(msg->blacklist,"禁入"))
        sprintf(speak_send,"禁止通行");

    json_send_speak["content"] = Json::Value(speak_send);
    json_send_speak["led_ip"] = Json::Value(led_ip);

    std::string send = json_send.toStyledString(); //转成字符串
    std::string send_speak = json_send_speak.toStyledString();

    char ip[] = {"127.0.0.1"};

    struct sockaddr_in addr;
    int sock;
    sock=socket(AF_INET, SOCK_DGRAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT_UDP_BCENTER_TO_BLED); // 5006 --> 5002
    addr.sin_addr.s_addr = inet_addr(ip);
    int n = sendto(sock,send.c_str(),send.length(), 0, (struct sockaddr *)&addr, sizeof(addr));  //发送LED信息
    if(n < 0)
    {
        fprintf(fp_log,"%s##mongodb_send_bled 显示信息发送失败\n",time_now);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 发送显示信息失败 line[%d]", __FUNCTION__, __LINE__);
        writelog(log_buf_);
    }
    else
    {
        fprintf(fp_log,"%s##mongodb_send_bled 显示信息发送成功:\n",time_now);
        fprintf(fp_log,"%s\n",send.c_str());

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 向bled成功发送一条消息[%s] ip[%s] port[%d] line[%d]",
                 __FUNCTION__, send.c_str(), ip, PORT_UDP_BCENTER_TO_BLED, __LINE__);
        writelog(log_buf_);
    }

    n = sendto(sock, send_speak.c_str(), send_speak.length(), 0, (struct sockaddr *)&addr, sizeof(addr)); //发送语音信息
    if(n < 0)
    {
        fprintf(fp_log,"%s##mongodb_send_bled 语音信息发送失败\n",time_now);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 发送语音信息失败[%s] len[%ld] ip[%s] port[%d] line[%d]",
                 __FUNCTION__, send_speak.c_str(), send_speak.length(), ip, PORT_UDP_BCENTER_TO_BLED, __LINE__);
        writelog(log_buf_);
    }
    else
    {
        fprintf(fp_log,"%s##mongodb_send_bled 语音信息发送成功:\n",time_now);
        fprintf(fp_log,"%s\n", send_speak.c_str());

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 向bled成功发送一条消息[%s] len[%ld] ip[%s] port[%d] line[%d]",
                 __FUNCTION__, send_speak.c_str(), send_speak.length(), ip, PORT_UDP_BCENTER_TO_BLED, __LINE__);
        writelog(log_buf_);
    }
    close(sock);
    /******************************end***********************************************************/
    return 0;

}
/******************************end***********************************************************/
int mongodb_process_fee(char *channel_id,char *plate,char * in_time,char *out_time)
{
    snprintf(log_buf_, sizeof(log_buf_), "[%s] line[%d]", __FUNCTION__, __LINE__);
    writelog(log_buf_);

  //  mongoc_client_t *mongodb_client;
    mongoc_collection_t *mongodb_table_car;	//channel表
    mongoc_collection_t *mongodb_table_chargerule;	//device表
    mongoc_collection_t *mongodb_table_carinpark;	//device表
    mongoc_collection_t *mongodb_table_channel;	//device表
    mongoc_collection_t *mongodb_table_caroutrec;	//device表
    mongoc_collection_t *mongodb_table_wx_zfb;	//device表
//    mongodb_client = mongoc_client_new(str_con);
    mongodb_table_car = mongoc_client_get_collection(mongodb_client,"boondb","car");   //channel表
    mongodb_table_chargerule = mongoc_client_get_collection(mongodb_client,"boondb","chargerule");   //device表
    mongodb_table_carinpark = mongoc_client_get_collection(mongodb_client,"boondb","carinpark");   //device表
    mongodb_table_channel = mongoc_client_get_collection(mongodb_client,"boondb","channel");   //device表
    mongodb_table_caroutrec = mongoc_client_get_collection(mongodb_client,"boondb","caroutrec");   //device表
    mongodb_table_wx_zfb = mongoc_client_get_collection(mongodb_client,"boondb","wx_zfb");   //device表

    mongoc_cursor_t *cursor;
    bson_t *query;
    bson_t *query_tmp;
    bson_t result;
    const char *tmp;
    const bson_t *tmp1;
    bson_iter_t iter;
    bson_error_t error;
    unsigned int length;

    char tmp2[48];
    char park_id[256];

    char car_type[24] = {0};
    char charge_rule[256] = {0};
    int fee = 0;
    //int fee1 = 0;

    char time_now[64];
    time_t tm;
    time_printf(time(&tm),time_now);

    query = bson_new();

    BSON_APPEND_UTF8(query, "channel_id",channel_id);

    cursor = mongoc_collection_find (mongodb_table_channel, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);
        mongoc_cursor_destroy(cursor);

        fprintf(fp_log,"%s##mongodb_process_fee 失败\n",time_now);
        return -1;
    }

    if(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor)) //得到car表的每一条记录
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "channel_park_id"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(park_id,0,256);
                memcpy(park_id,tmp,length);
            }
            bson_destroy(&result);
        }
    }
    bson_destroy(query);

    mongoc_cursor_destroy(cursor);

    query = bson_new();

    BSON_APPEND_UTF8(query, "car_plate_id",plate);
    memset(car_type,0,24);
    cursor = mongoc_collection_find (mongodb_table_car, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);

        fprintf(fp_log,"%s##mongodb_process_fee 失败\n",time_now);
        return -1;
    }
    if(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor)) //得到car表的每一条记录
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "car_type"))
            {
                tmp = bson_iter_utf8(&iter,&length);

                memcpy(car_type,tmp,length);
            }
            bson_destroy(&result);
        }
    }
    bson_destroy(query);

    mongoc_cursor_destroy(cursor);

    if(strlen(car_type) < 3)
    {
        memset(car_type,0,24);
        strcpy(car_type,"临时车");
    }

    query = bson_new();

    BSON_APPEND_UTF8(query, "chargerule_park_id",park_id);
    BSON_APPEND_UTF8(query, "chargerule_car_type",car_type);
    cursor = mongoc_collection_find (mongodb_table_chargerule, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);
        mongoc_cursor_destroy(cursor);

        fprintf(fp_log,"%s##mongodb_process_fee 失败\n",time_now);
        return -1;
    }

    bool find_value = false;
    if(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor)) //得到car表的每一条记录
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "chargerule_name_id"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(charge_rule,0,256);
                memcpy(charge_rule,tmp,length);

                find_value = true;

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 找到charge_rule[%s] 查找chargerule表 line[%d]", __FUNCTION__, charge_rule, __LINE__);
                writelog(log_buf_);
            }
            bson_destroy(&result);
        }
    }
    bson_destroy(query);
    mongoc_cursor_destroy(cursor);

    if(!charge_rule){
        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 没有找到charge_rule 查找chargerule表，查找条件chargerule_park_id[%s] chargerule_car_type[%s] line[%d]",
                 __FUNCTION__, park_id, car_type, __LINE__);
        writelog(log_buf_);
    }

    char tmp_time_in[24];
    char tmp_time_out[24];
    memset(tmp_time_in,0,24);
    memset(tmp_time_out,0,24);
    query = bson_new();

    query_tmp = bson_new();

    BSON_APPEND_UTF8(query, "caroutrec_parent_park_id",park_id);
    BSON_APPEND_UTF8(query, "caroutrec_plate_id",plate);

    memset(tmp2,0,48);
    sprintf(tmp2,"'$gte':%s",in_time);
    BSON_APPEND_UTF8(query_tmp, "$gte",in_time);
    BSON_APPEND_DOCUMENT(query, "caroutrec_out_time",query_tmp);
    //query = BCON_NEW ("$gte", "{", "caroutrec_out_time", BCON_UTF8 (in_time),"}");
    cursor = mongoc_collection_find (mongodb_table_caroutrec, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);

        fprintf(fp_log,"%s##mongodb_process_fee失败\n",time_now);
        return -1;
    }
    while(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor)) //得到car表的每一条记录
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {

            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "caroutrec_pay_charge"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(tmp2,0,24);
                memcpy(tmp2,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "caroutrec_in_time"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memcpy(tmp_time_in,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "caroutrec_out_time"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memcpy(tmp_time_out,tmp,length);
            }
            bson_destroy(&result);
        }
    }

    bson_destroy(query);
    bson_destroy(query_tmp);
    mongoc_cursor_destroy(cursor);

    // 来自wx_zfb
    int sum_money = 0;
    query = bson_new();

    query_tmp = bson_new();

    BSON_APPEND_UTF8(query, "plate",plate);

    BSON_APPEND_UTF8(query_tmp, "$gte",in_time);
    BSON_APPEND_DOCUMENT(query, "time",query_tmp);
    //query = BCON_NEW ("$gte", "{", "caroutrec_out_time", BCON_UTF8 (in_time),"}");
    cursor = mongoc_collection_find (mongodb_table_wx_zfb, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);

        fprintf(fp_log,"%s##mongodb_process_fee失败\n",time_now);
        return -1;
    }

    while(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor)) //得到car表的每一条记录
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {

            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "sum_money"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(tmp2,0,24);
                memcpy(tmp2,tmp,length);
                sum_money = sum_money + atoi(tmp2);
            }
            bson_destroy(&result);
        }
    }

    bson_destroy(query);
    bson_destroy(query_tmp);
    mongoc_cursor_destroy(cursor);

    if(strlen(tmp_time_in) > 6 && strlen(tmp_time_out) > 6)
    {
        int mils = get_tick(tmp_time_out) - get_tick(tmp_time_in);

        char tmp_time_out1[24];
        memset(tmp_time_out1,0,24);
        unixTime2Str( get_tick(time_now) -  mils,tmp_time_out1,24);
        fprintf(fp_log,"%s##计费有下级停车场，下级停车场进入时间%s,下级停车场离开时间%s,本级停车场进入时间%s,本级停车场离开时间%s,计费离开时间%s\n",time_now,tmp_time_in,tmp_time_out,in_time,time_now,tmp_time_out1);
        fee = mongodb_cal_fee(charge_rule,in_time,tmp_time_out1,car_type,park_id,plate);

        fee = fee + atoi(tmp2);
    }
    else
    {
        snprintf(log_buf_, sizeof(log_buf_)-1, "[%s] 手机端已交费[%d]元 line[%d]", __FUNCTION__, sum_money, __LINE__);
        writelog(log_buf_);

        int wx_duration = 0;
        if(sum_money == 0)
        {
            fprintf(fp_log,"%s##计费无下级停车场  %s,%s,%s,%s,%s\n",time_now,in_time,out_time,park_id,charge_rule,car_type);
            fee = mongodb_cal_fee(charge_rule,in_time,out_time,car_type,park_id,plate);
/*
            //添加计费矫正
            int fee1 = 0;
            int mils_dur = 0;
            mils_dur = get_tick(out_time) - get_tick(in_time);
            fee1 = ((int)floor(mils_dur/60.0/60.0)+1)*2;
            if(fee1<fee && fee1>0)
            {
                fprintf(fp_log,"[mongodb_process_fee]进入计费矫正");
                fprintf(fp_log,"[mongodb_process_fee]矫正前计费%d,矫正后交费%d\n",fee,fee1);
                fee = fee1;
            }
*/



        }
        else
        {
            snprintf(log_buf_, sizeof(log_buf_)-1, "[%s] 计费无下级停车场 入场时间[%s] 出场时间[%s] 停车场id[%s] 收费方案[%s] 车辆类型[%s] line[%d]", __FUNCTION__,
                     in_time, out_time, park_id, charge_rule, car_type,  __LINE__);
            writelog(log_buf_);

            fprintf(fp_log,"%s##计费无下级停车场  %s,%s,%s,%s,%s\n",time_now,in_time,out_time,park_id,charge_rule,car_type);

            query = bson_new();
            BSON_APPEND_UTF8(query, "chargerule_car_type",car_type);
            //query = BCON_NEW ("$gte", "{", "caroutrec_out_time", BCON_UTF8 (in_time),"}");
            cursor = mongoc_collection_find (mongodb_table_chargerule, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
            if(mongoc_cursor_error(cursor,&error))
            {
                bson_destroy(query);
                mongoc_cursor_destroy(cursor);

                fprintf(fp_log,"%s##mongodb_process_fee失败\n",time_now);
                return -1;
            }

            bool find_value = false;
            if(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor)) //得到car表的每一条记录
            {
                if (mongoc_cursor_next (cursor, &tmp1))
                {
                    bson_copy_to(tmp1,&result); //得到一条完整的记录
                    if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "chargerule_weixin_free_period_duration"))
                    {
                        tmp = bson_iter_utf8(&iter,&length);
                        memset(tmp2,0,24);
                        memcpy(tmp2,tmp,length);
                        wx_duration = atoi(tmp2);

                        find_value = true;

                        snprintf(log_buf_, sizeof(log_buf_)-1, "[%s] 找到微信免费持续时间[%d], 查找chargerule表 line[%d]", __FUNCTION__, wx_duration, __LINE__);
                        writelog(log_buf_);
                    }
                    bson_destroy(&result);
                }
            }

            bson_destroy(query);
            mongoc_cursor_destroy(cursor);

            if(!find_value){
                snprintf(log_buf_, sizeof(log_buf_)-1, "[%s] error! 查找chargerule表,没有找到微信免费持续时间 查找条件chargerule_car_type[%s] line[%d]", __FUNCTION__, car_type, __LINE__);
                writelog(log_buf_);
            }

            fprintf(fp_log,"%s##微信已交费%d元，微信免费时间%d分钟\n",time_now,sum_money,wx_duration);

            char tmp_time_out1[24] = {0};

            snprintf(log_buf_, sizeof(log_buf_)-1, "[%s] 免费时间[%d]分钟 line[%d]", __FUNCTION__, wx_duration, __LINE__);
            writelog(log_buf_);

            unixTime2Str( get_tick(out_time) -  wx_duration*60, tmp_time_out1, 24);

            fprintf(fp_log,"%s##计费有下级停车场 计费离开时间%s\n", time_now, tmp_time_out1);

            snprintf(log_buf_, sizeof(log_buf_)-1, "[%s] 转换后的结果 out_time[%s] tmp_time_out1[%s] line[%d]", __FUNCTION__, out_time, tmp_time_out1, __LINE__);
            writelog(log_buf_);

            fee = mongodb_cal_fee(charge_rule,in_time,tmp_time_out1,car_type,park_id,plate);
        }
    }

    snprintf(log_buf_, sizeof(log_buf_)-1, "[%s] 应交费[%d]元, 手机扫码已交[%d]元 line[%d]", __FUNCTION__, fee, sum_money, __LINE__);
    writelog(log_buf_);

    fprintf(fp_log,"%s##应交费%d元,微信已交费%d元\n",time_now,fee,sum_money);
    fee = fee - sum_money;
    if(fee < 0) fee = 0;

    /*查询中央缴费和微信缴费*/
    mongoc_collection_destroy(mongodb_table_channel);
    mongoc_collection_destroy(mongodb_table_car);
    mongoc_collection_destroy(mongodb_table_carinpark);
    mongoc_collection_destroy(mongodb_table_caroutrec);
    mongoc_collection_destroy(mongodb_table_chargerule);
  //  mongoc_client_destroy(mongodb_client);


    return fee;
}

int mongodb_write_caroutrec(char *in_time,int pay_charge,int real_charge,char *plate_ori,char *plate_final,char *in_pic_path,char *in_channel_id,char *park_id,char *park_parent_id,char *car_type,char *operator_name,char *open_door,char *charge_type,car_msg *out)
{
  //  mongoc_client_t *mongodb_client;
    mongoc_collection_t *mongodb_table_caroutrec;	//channel表

 //   mongodb_client = mongoc_client_new(str_con);
    mongodb_table_caroutrec = mongoc_client_get_collection(mongodb_client,"boondb","caroutrec");   //channel表

    mongoc_cursor_t *cursor;
    bson_t *query;
    bson_t result;
    const char *tmp;
    const bson_t *tmp1;
    bson_iter_t iter;
    bson_error_t error;
    unsigned int length;

    char tmp2[24];
    char time_now[64];
    time_t tm;
    time_printf(time(&tm),time_now);

    query = bson_new();
    BSON_APPEND_UTF8(query, "caroutrec_in_time",in_time);
    BSON_APPEND_UTF8(query, "caroutrec_out_time",out->time);
    memset(tmp2,0,24);
    sprintf(tmp2,"%d",pay_charge);
    BSON_APPEND_UTF8(query, "caroutrec_pay_charge",tmp2);
    memset(tmp2,0,24);
    sprintf(tmp2,"%d",real_charge);
    BSON_APPEND_UTF8(query, "caroutrec_real_charge",tmp2);
    memset(tmp2,0,24);
    sprintf(tmp2,"%d",get_tick(time_now));
    BSON_APPEND_UTF8(query, "caroutrec_timestamp",tmp2);
    BSON_APPEND_UTF8(query, "caroutrec_alg_plate",plate_ori);
    BSON_APPEND_UTF8(query, "caroutrec_plate_id",plate_final);
    BSON_APPEND_UTF8(query, "caroutrec_in_pic_path",in_pic_path);
    BSON_APPEND_UTF8(query, "caroutrec_out_pic_path",out->path);
    BSON_APPEND_UTF8(query, "caroutrec_in_channel_id",in_channel_id);
    BSON_APPEND_UTF8(query, "caroutrec_out_channel_id",out->channel_id);
    BSON_APPEND_UTF8(query, "caroutrec_park_id",park_id);
    BSON_APPEND_UTF8(query, "caroutrec_parent_park_id",park_parent_id);
    BSON_APPEND_UTF8(query, "caroutrec_car_type",car_type);
    BSON_APPEND_UTF8(query, "caroutrec_operator_id",operator_name);
    BSON_APPEND_UTF8(query, "caroutrec_open_door_type",open_door);
    BSON_APPEND_UTF8(query, "caroutrec_charge_type",charge_type);
    BSON_APPEND_UTF8(query, "caroutrec_remark","");
    memset(tmp2,0,24);
    if(strlen(in_time) < 3)
    {
        sprintf(tmp2,"%d",0);
    }
    else
    {
        sprintf(tmp2,"%d",(get_tick(out->time) -get_tick(in_time) )/60);
    }
    BSON_APPEND_UTF8(query, "caroutrec_stay_time",tmp2);
    BSON_APPEND_UTF8(query, "caroutrec_car_logo",out->brand);
    BSON_APPEND_UTF8(query, "caroutrec_car_color",out->color);
    BSON_APPEND_UTF8(query, "caroutrec_aux_plate_id",out->plate1);
    BSON_APPEND_UTF8(query, "caroutrec_aux_car_logo",out->brand1);
    BSON_APPEND_UTF8(query, "caroutrec_aux_car_color",out->color1);

    if(!mongoc_collection_insert (mongodb_table_caroutrec, MONGOC_INSERT_NONE, query, NULL, &error))
    {
        fprintf(fp_log,"%s##mongodb_write_in_park 失败\n",time_now);
        bson_destroy(query);
        return -1;
    }
    fprintf(fp_log,"%s##写数据到出场表\n",time_now);

    bson_destroy(query);
    mongoc_collection_destroy(mongodb_table_caroutrec);
  //  mongoc_client_destroy(mongodb_client);
    return 0;
}

int mongodb_write_carinpark(char *plate_ori,char *plate_final,char *park_id,char *car_type,char *people_name,car_msg *msg)
{

  //  mongoc_client_t *mongodb_client;
    mongoc_collection_t *mongodb_table_carinpark;	//channel表

   // mongodb_client = mongoc_client_new(str_con);
    mongodb_table_carinpark = mongoc_client_get_collection(mongodb_client,"boondb","carinpark");   //channel表
    mongoc_cursor_t *cursor;
    bson_t *query;
    bson_t result;
    const char *tmp;
    const bson_t *tmp1;
    bson_iter_t iter;
    bson_error_t error;
    unsigned int length;

    char tmp2[24];
    char time_now[64];
    time_t tm;
    time_printf(time(&tm),time_now);

    query = bson_new();
    BSON_APPEND_UTF8(query, "carinpark_in_time",msg->time);
    memset(tmp2,0,24);
    sprintf(tmp2,"%d",get_tick(time_now));
    BSON_APPEND_UTF8(query, "carinpark_time_stamp",tmp2);
    BSON_APPEND_UTF8(query, "carinpark_alg_plate",plate_ori);
    BSON_APPEND_UTF8(query, "carinpark_plate_id",plate_final);
    BSON_APPEND_UTF8(query, "carinpark_pic_path",msg->path);
    BSON_APPEND_UTF8(query, "carinpark_channel_id",msg->channel_id);
    BSON_APPEND_UTF8(query, "carinpark_park_id",park_id);
    BSON_APPEND_UTF8(query, "carinpark_car_type",car_type);
    BSON_APPEND_UTF8(query, "carinpark_car_logo",msg->brand);
    BSON_APPEND_UTF8(query, "carinpark_car_color",msg->color);
    BSON_APPEND_UTF8(query, "carinpark_aux_plate_id",msg->plate1);
    BSON_APPEND_UTF8(query, "carinpark_aux_alg_plate",msg->plate1);
    BSON_APPEND_UTF8(query, "carinpark_aux_car_logo",msg->brand1);
    BSON_APPEND_UTF8(query, "carinpark_aux_car_color",msg->color1);
    BSON_APPEND_UTF8(query, "carinpark_people_name",people_name);

    if(!mongoc_collection_insert (mongodb_table_carinpark, MONGOC_INSERT_NONE, query, NULL, &error))
    {

        fprintf(fp_log,"%s##mongodb_write_carinpark 失败\n",time_now);
        bson_destroy(query);
        return -1;
    }

    fprintf(fp_log,"%s##写数据到在场表\n",time_now);
    bson_destroy(query);
    mongoc_collection_destroy(mongodb_table_carinpark);
  //  mongoc_client_destroy(mongodb_client);
    return 0;

}

/******************************发送bgui启动信息***********************************************************/
int mongodb_send_bgui_start()
{
  //  mongoc_client_t *mongodb_client;
    mongoc_collection_t *mongodb_table_channel;	//channel表
    mongoc_collection_t *mongodb_table_park;	//channel表
    mongoc_collection_t *mongodb_table_people;	//device表
    mongoc_collection_t *mongodb_table_tmpcartype;	//device表
  //  mongodb_client = mongoc_client_new(str_con);
    mongodb_table_channel = mongoc_client_get_collection(mongodb_client,"boondb","channel");   //channel表
    mongodb_table_people = mongoc_client_get_collection(mongodb_client,"boondb","people");   //device表
    mongodb_table_park = mongoc_client_get_collection(mongodb_client,"boondb","park");   //device表
    mongodb_table_tmpcartype = mongoc_client_get_collection(mongodb_client,"boondb","tmpcartype");   //device表


    Json::Value bgui_send;
    Json::Value content;

    mongoc_cursor_t *cursor;
    bson_t *query;
    bson_t result;
    const char *tmp;
    const bson_t *tmp1;
    bson_iter_t iter;
    bson_error_t error;
    unsigned int length;

    char time_now[64];
    time_t tm;
    time_printf(time(&tm),time_now);


    char park_id[256];//车场id
    char park_name[24];//车位名称
    char space_count[24];//车位数
    char space_count_remain[24];//剩余车位数

    query = bson_new();

    cursor = mongoc_collection_find (mongodb_table_people, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);

        fprintf(fp_log,"%s##mongodb_send_bgui_start 失败\n",time_now);
        return -1;
    }
    while(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor))
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "people_name"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                content["user"].append(tmp);
            }
            bson_destroy(&result);
        }
    }
    bson_destroy(query);

    mongoc_cursor_destroy(cursor);

    query = bson_new();

    BSON_APPEND_UTF8(query, "channel_ip",host_ip);

    cursor = mongoc_collection_find(mongodb_table_channel, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);

        fprintf(fp_log,"%s##mongodb_send_bgui_start 失败\n",time_now);
        return -1;
    }
    if(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor))
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "channel_park_id"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(park_id,0,256);
                memcpy(park_id,tmp,length);
            }
            bson_destroy(&result);
        }
    }
    bson_destroy(query);

    mongoc_cursor_destroy(cursor);

    query = bson_new();

    BSON_APPEND_UTF8(query, "park_id",park_id);
    cursor = mongoc_collection_find(mongodb_table_park, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);

        fprintf(fp_log,"%s##mongodb_send_bgui_start 失败\n",time_now);
        return -1;
    }
    if(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor))
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "park_name"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(park_name,0,256);
                memcpy(park_name,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "park_space_count"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(space_count,0,24);
                memcpy(space_count,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "park_space_count_remain"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(space_count_remain,0,24);
                memcpy(space_count_remain,tmp,length);
            }
            bson_destroy(&result);
        }
    }
    bson_destroy(query);

    mongoc_cursor_destroy(cursor);

    query = bson_new();


    cursor = mongoc_collection_find(mongodb_table_tmpcartype, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);

        fprintf(fp_log,"%s##mongodb_send_bgui_start 失败\n",time_now);
        return -1;
    }
    while(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor))
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "tmpcartype_cartype"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                content["cartype"].append(tmp);
            }
            bson_destroy(&result);
        }
    }
    bson_destroy(query);

    mongoc_cursor_destroy(cursor);
    mongoc_collection_destroy(mongodb_table_tmpcartype);
    mongoc_collection_destroy(mongodb_table_park);
    mongoc_collection_destroy(mongodb_table_channel);
    mongoc_collection_destroy(mongodb_table_people);
  //  mongoc_client_destroy(mongodb_client);

    content["parkname"] = Json::Value(park_name);
    content["totalparkposition"] = Json::Value(space_count);
    char tmp3[24];
    int use = atoi(space_count) - atoi(space_count_remain);
    memset(tmp3,0,24);
    sprintf(tmp3,"%d",use);
    content["usedparkpostion"] = Json::Value(tmp3);
    content["remainingparkposition"] = Json::Value(space_count_remain);
    content["local_ip"] = Json::Value(host_ip);
    content["server_ip"] = Json::Value(host_server_ip);

    bgui_send["cmd"] = Json::Value("start");
    bgui_send["content"] = content;

    std::string send = bgui_send.toStyledString();

    char ip[] = {"127.0.0.1"};

    struct sockaddr_in addr;
    int sock;
    sock=socket(AF_INET, SOCK_DGRAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT_UDP_BCENTER_TO_BGUI);
    addr.sin_addr.s_addr = inet_addr(ip);
    int n = sendto(sock,send.c_str(),send.length(), 0, (struct sockaddr *)&addr, sizeof(addr));
    if(n < 0)
    {
        fprintf(fp_log,"%s##bgui mongodb_send_bgui_start发送失败\n",time_now);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 向bgui发包失败 msg[%s] len[%ld] ip[%s] port[%d] line[%d]",
                 __FUNCTION__, send.c_str(), send.length(), ip, PORT_UDP_BCENTER_TO_BGUI, __LINE__);
        writelog(log_buf_);
    }
    else
    {
        fprintf(fp_log,"%s##bgui mongodb_send_bgui_start发送成功:\n",time_now);
        fprintf(fp_log,"%s\n",send.c_str());

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 向bgui发送出一条消息 msg[%s] len[%ld] ip[%s] port[%d] line[%d]",
                 __FUNCTION__, send.c_str(), send.length(), ip, PORT_UDP_BCENTER_TO_BGUI, __LINE__);
        writelog(log_buf_);
    }
    close(sock);
    return 0;

}

int mongodb_bgui_login(char *name,char *password)
{
 //   mongoc_client_t *mongodb_client;
    mongoc_collection_t *mongodb_table_people;	//channel表

  //  mongodb_client = mongoc_client_new(str_con);
    mongodb_table_people = mongoc_client_get_collection(mongodb_client,"boondb","people");   //channel表

    Json::Value bgui_send;
    Json::Value content;

    mongoc_cursor_t *cursor;
    bson_t *query;
    bson_t result;
    const char *tmp;
    const bson_t *tmp1;
    bson_iter_t iter;
    bson_error_t error;
    unsigned int length;
    int count = 0;

    char time_now[64];
    time_t tm;
    time_printf(time(&tm),time_now);

    query = bson_new();
    BSON_APPEND_UTF8(query, "people_name",name);
    BSON_APPEND_UTF8(query, "people_pass_word",password);
    count = mongoc_collection_count (mongodb_table_people, MONGOC_QUERY_NONE, query, 0, 0, NULL, &error);
    bson_destroy(query);
    mongoc_collection_destroy(mongodb_table_people);
 //   mongoc_client_destroy(mongodb_client);

    bgui_send["cmd"] = Json::Value("login");
    if(count > 0)
    {
        memset(operator_name,0,256);
        memcpy(operator_name,name,strlen(name));
        content["result"] = Json::Value("成功");
    }
    else
    {
        content["result"] = Json::Value("失败");
    }
    bgui_send["content"] = content;

    char ip[] = {"127.0.0.1"};

    std::string send = bgui_send.toStyledString();
    struct sockaddr_in addr;
    int sock;
    sock=socket(AF_INET, SOCK_DGRAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT_UDP_BCENTER_TO_BGUI);
    addr.sin_addr.s_addr = inet_addr(ip);
    int n = sendto(sock,send.c_str(),send.length(), 0, (struct sockaddr *)&addr, sizeof(addr));
    if(n < 0)
    {
        fprintf(fp_log,"%s##bgui mongodb_bgui_login发送失败\n",time_now);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 向bgui发包失败 msg[%s] len[%ld] ip[%s] port[%d] line[%d]",
                 __FUNCTION__, send.c_str(), send.length(), ip, PORT_UDP_BCENTER_TO_BGUI, __LINE__);
        writelog(log_buf_);
    }
    else
    {
        fprintf(fp_log,"%s##bgui mongodb_bgui_login发送成功:\n",time_now);
        fprintf(fp_log,"%s\n",send.c_str());

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 向bgui发送出一条消息 msg[%s] len[%ld] ip[%s] port[%d] line[%d]",
                 __FUNCTION__, send.c_str(), send.length(), ip, PORT_UDP_BCENTER_TO_BGUI, __LINE__);
        writelog(log_buf_);
    }

    close(sock);

    return 0;


}

int mongodb_bgui_process_door(char *type,char *op)
{
 //   mongoc_client_t *mongodb_client;
    mongoc_collection_t *mongodb_table_car;	//channel表
    mongoc_collection_t *mongodb_table_carinpark;	//device表
    mongoc_collection_t *mongodb_table_carinrec;	//device表
 //   mongodb_client = mongoc_client_new(str_con);
    mongodb_table_car = mongoc_client_get_collection(mongodb_client,"boondb","car");   //channel表
    mongodb_table_carinpark = mongoc_client_get_collection(mongodb_client,"boondb","carinpark");   //device表
    mongodb_table_carinrec = mongoc_client_get_collection(mongodb_client,"boondb","carinrec");   //device表

    mongoc_cursor_t *cursor;
    bson_t *query;
    bson_t result;
    const char *tmp;
    const bson_t *tmp1;
    bson_iter_t iter;
    bson_error_t error;
    unsigned int length;
    int count = 0;
    char tmp2[24];

    char time_now[64];
    time_t tm;
    time_printf(time(&tm),time_now);

    Json::Value json_send;

    if(strcmp(type,"入口") == 0)
    {
        if(strcmp(op,"开") == 0)
        {
            json_send["cmd"] = Json::Value("open_door");
            json_send["channel_id"] = Json::Value(in_channel);
            json_send["in_out"] = Json::Value("入口");
            if(in_fleet == true)
            {
                json_send["flag"] = Json::Value("keep");
            }
            else
            {
                json_send["flag"] = Json::Value("once");
            }
        }
        else
        {
            json_send["cmd"] = Json::Value("close_door");
            json_send["channel_id"] = Json::Value(in_channel);
            json_send["in_out"] = Json::Value("入口");
        }
    }
    else
    {
        if(strcmp(op,"开") == 0)
        {
            json_send["cmd"] = Json::Value("open_door");
            json_send["channel_id"] = Json::Value(out_channel);
            json_send["in_out"] = Json::Value("出口");
            if(out_fleet == true)
            {
                json_send["flag"] = Json::Value("keep");
            }
            else
            {
                json_send["flag"] = Json::Value("once");
            }
        }
        else
        {
            json_send["cmd"] = Json::Value("close_door");
            json_send["channel_id"] = Json::Value(out_channel);
            json_send["in_out"] = Json::Value("出口");
        }
    }

    if(false == g_has_cheduimoshi_){
        json_send["flag"] = Json::Value("once");
    }

    std::string send = json_send.toStyledString();

    char ip[] = {"127.0.0.1"};

    struct sockaddr_in addr;
    int sock;
    sock=socket(AF_INET, SOCK_DGRAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT_UDP_BCENTER_TO_BIPC);
    addr.sin_addr.s_addr = inet_addr(ip);
    int n = sendto(sock,send.c_str(),send.length(), 0, (struct sockaddr *)&addr, sizeof(addr));
    if(n < 0)
    {
        fprintf(fp_log,"%s##mongodb_bgui_process_door 发送失败\n",time_now);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] error, 向bled发送消息失败 msg[%s] len[%ld] ip[%s] port[%d] line[%d]",
                 __FUNCTION__, send.c_str(), send.length(), ip, PORT_UDP_BCENTER_TO_BIPC, __LINE__);
        writelog(log_buf_);
    }
    else
    {
        fprintf(fp_log,"%s##mongodb_bgui_process_door 发送成功:\n",time_now);
        fprintf(fp_log,"%s\n",send.c_str());

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 向bled发送出一条消息 msg[%s] len[%ld] ip[%s] port[%d] line[%d]",
                 __FUNCTION__, send.c_str(), send.length(), ip, PORT_UDP_BCENTER_TO_BIPC, __LINE__);
        writelog(log_buf_);
    }
    close(sock);

    if(strcmp(type,"入口") == 0)
    {
        query = bson_new();
        BSON_APPEND_UTF8(query, "carinpark_plate_id",in_plate);
        BSON_APPEND_UTF8(query, "carinpark_channel_id",in_channel);

        count = mongoc_collection_count (mongodb_table_carinpark, MONGOC_QUERY_NONE, query, 0, 0, NULL, &error);
        bson_destroy(query);


        fprintf(fp_log,"%s##入口开杆，车牌号为%s,通道id为%s，进入时间%s,carinpark数量为%d\n",time_now,in_plate,in_channel,in_time,count);

        if(count == 0)
        {
            char pic_path[256];
            char park_id[256];
            char car_type[24];
            char car_logo[24];
            char car_color[24];
            char people_name[24];


            memset(pic_path,0,256);
            memset(park_id,0,256);
            memset(car_logo,0,24);
            memset(car_type,0,24);
            memset(car_color,0,24);
            memset(people_name,0,24);

            query = bson_new();
            bson_t *result1 = bson_new();
            BSON_APPEND_UTF8(query, "carinrec_in_time",in_time);
            BSON_APPEND_UTF8(query, "carinrec_plate_id",in_plate);
            BSON_APPEND_UTF8(query, "carinrec_in_time",in_time);
            result1 = BCON_NEW ("$set", "{", "carinrec_open_door_type", BCON_UTF8 ("手动抬杆"),"updated", BCON_BOOL (true) ,"}");
            mongoc_collection_update(mongodb_table_carinrec,MONGOC_UPDATE_NONE,query,result1,NULL,&error);
            bson_destroy(query);
            bson_destroy(result1);

            query = bson_new();

            BSON_APPEND_UTF8(query, "carinrec_plate_id",in_plate);
            BSON_APPEND_UTF8(query, "carinrec_channel_id",in_channel);
            BSON_APPEND_UTF8(query, "carinrec_in_time",in_time);
            cursor = mongoc_collection_find(mongodb_table_carinrec, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
            if(mongoc_cursor_error(cursor,&error))
            {
                bson_destroy(query);

                mongoc_cursor_destroy(cursor);

                fprintf(fp_log,"%s##mongodb_send_bgui_start 失败\n",time_now);
                return -1;
            }
            if(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor))
            {
                if (mongoc_cursor_next (cursor, &tmp1))
                {
                    bson_copy_to(tmp1,&result); //得到一条完整的记录
                    if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "carinrec_pic_path"))
                    {
                        tmp = bson_iter_utf8(&iter,&length);

                        memcpy(pic_path,tmp,length);
                    }
                    if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "carinrec_park_id"))
                    {
                        tmp = bson_iter_utf8(&iter,&length);

                        memcpy(park_id,tmp,length);
                    }
                    if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "carinrec_car_type"))
                    {
                        tmp = bson_iter_utf8(&iter,&length);

                        memcpy(car_type,tmp,length);
                    }
                    if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "carinrec_car_logo"))
                    {
                        tmp = bson_iter_utf8(&iter,&length);

                        memcpy(car_logo,tmp,length);
                    }
                    if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "carinrec_car_color"))
                    {
                        tmp = bson_iter_utf8(&iter,&length);

                        memcpy(car_color,tmp,length);
                    }
                    bson_destroy(&result);
                }
            }
            bson_destroy(query);

            mongoc_cursor_destroy(cursor);


            query = bson_new();

            BSON_APPEND_UTF8(query, "car_plate_id",in_plate);

            cursor = mongoc_collection_find(mongodb_table_car, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
            if(mongoc_cursor_error(cursor,&error))
            {
                bson_destroy(query);

                mongoc_cursor_destroy(cursor);

                fprintf(fp_log,"%s##mongodb_send_bgui_start 失败\n",time_now);
                return -1;
            }
            if(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor))
            {
                if (mongoc_cursor_next (cursor, &tmp1))
                {
                    bson_copy_to(tmp1,&result); //得到一条完整的记录
                    if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "car_name"))
                    {
                        tmp = bson_iter_utf8(&iter,&length);

                        memcpy(people_name,tmp,length);
                    }
                    bson_destroy(&result);
                }
            }
            bson_destroy(query);

            mongoc_cursor_destroy(cursor);


            query = bson_new();
            BSON_APPEND_UTF8(query, "carinpark_in_time",in_time);
            memset(tmp2,0,24);
            sprintf(tmp2,"%d",get_tick(in_time));
            BSON_APPEND_UTF8(query, "carinpark_time_stamp",tmp2);
            BSON_APPEND_UTF8(query, "carinpark_alg_plate","");
            BSON_APPEND_UTF8(query, "carinpark_plate_id",in_plate);
            BSON_APPEND_UTF8(query, "carinpark_pic_path",pic_path);
            BSON_APPEND_UTF8(query, "carinpark_channel_id",in_channel);
            BSON_APPEND_UTF8(query, "carinpark_park_id",park_id);
            BSON_APPEND_UTF8(query, "carinpark_car_type",car_type);
            BSON_APPEND_UTF8(query, "carinpark_car_logo",car_logo);
            BSON_APPEND_UTF8(query, "carinpark_car_color",car_color);
            BSON_APPEND_UTF8(query, "carinpark_aux_plate_id","");
            BSON_APPEND_UTF8(query, "carinpark_aux_alg_plate","");
            BSON_APPEND_UTF8(query, "carinpark_aux_car_logo","");
            BSON_APPEND_UTF8(query, "carinpark_aux_car_color","");
            BSON_APPEND_UTF8(query, "carinpark_people_name",people_name);

            if(!mongoc_collection_insert (mongodb_table_carinpark, MONGOC_INSERT_NONE, query, NULL, &error))
            {

                fprintf(fp_log,"%s##mongodb_write_carinpark 失败\n",time_now);
                bson_destroy(query);
                return -1;
            }
            bson_destroy(query);

        }

    }
    else
    {
        char park_id[256];
        memset(park_id,0,256);
        query = bson_new();

        BSON_APPEND_UTF8(query, "channel_id",out_channel);

        cursor = mongoc_collection_find(mongodb_table_car, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
        if(mongoc_cursor_error(cursor,&error))
        {
            bson_destroy(query);

            mongoc_cursor_destroy(cursor);

            fprintf(fp_log,"%s##mongodb_send_bgui_start 失败\n",time_now);
            return -1;
        }
        if(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor))
        {
            if (mongoc_cursor_next (cursor, &tmp1))
            {
                bson_copy_to(tmp1,&result); //得到一条完整的记录
                if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "channel_park_id"))
                {
                    tmp = bson_iter_utf8(&iter,&length);

                    memcpy(park_id,tmp,length);
                }
                bson_destroy(&result);
            }
        }
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);


        query = bson_new();
        BSON_APPEND_UTF8(query, "carinpark_park_id",park_id);
        BSON_APPEND_UTF8(query, "carinpark_plate_id",out_plate);

        count = mongoc_collection_count (mongodb_table_carinpark, MONGOC_QUERY_NONE, query, 0, 0, NULL, &error);
        bson_destroy(query);

        if(count > 0)
        {
            mongodb_delete_car_inpark(out_plate,park_id);
        }

    }
    mongoc_collection_destroy(mongodb_table_car);
    mongoc_collection_destroy(mongodb_table_carinpark);
    mongoc_collection_destroy(mongodb_table_carinrec);
 //   mongoc_client_destroy(mongodb_client);
    return 0;
}

int mongodb_bgui_process_youhui(char *type,char *plate)
{
   // mongoc_client_t *mongodb_client;
    mongoc_collection_t *mongodb_table_chargerule;	//channel表
    mongoc_collection_t *mongodb_table_carinpark;	//device表
    mongoc_collection_t *mongodb_table_caroutrec;	//device表
   // mongodb_client = mongoc_client_new(str_con);
    mongodb_table_chargerule = mongoc_client_get_collection(mongodb_client,"boondb","chargerule");   //channel表
    mongodb_table_carinpark = mongoc_client_get_collection(mongodb_client,"boondb","carinpark");   //device表
    mongodb_table_caroutrec = mongoc_client_get_collection(mongodb_client,"boondb","caroutrec");   //device表

    mongoc_cursor_t *cursor;
    bson_t *query;
    bson_t result;
    const char *tmp;
    const bson_t *tmp1;
    bson_iter_t iter;
    bson_error_t error;
    unsigned int length;
    int count = 0;
    char tmp2[24];

    char time_now[64];
    time_t tm;
    time_printf(time(&tm),time_now);

    char park_id[256];
    char chargerule[256];
    char in_time[24];
    memset(park_id,0,256);
    memset(chargerule,0,256);
    memset(in_time,0,24);


    int fee = 0;
    query = bson_new();

    BSON_APPEND_UTF8(query, "carinpark_plate_id",plate);

    cursor = mongoc_collection_find(mongodb_table_carinpark, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);

        fprintf(fp_log,"%s##mongodb_bgui_process_youhui 失败\n",time_now);
        return -1;
    }
    if(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor))
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "carinpark_park_id"))
            {
                tmp = bson_iter_utf8(&iter,&length);

                memcpy(park_id,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "carinpark_in_time"))
            {
                tmp = bson_iter_utf8(&iter,&length);

                memcpy(in_time,tmp,length);
            }

        }
    }
    bson_destroy(query);
    fprintf(fp_log,"%s##进入时间%s 车场id%s  \n",time_now,in_time,park_id);
    mongoc_cursor_destroy(cursor);

    if(strlen(park_id) < 10)
    {
        fee = 0;
    }
    else
    {
        query = bson_new();

        BSON_APPEND_UTF8(query, "chargerule_park_id",park_id);
        BSON_APPEND_UTF8(query, "chargerule_car_type",type);
        cursor = mongoc_collection_find(mongodb_table_chargerule, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
        if(mongoc_cursor_error(cursor,&error))
        {
            bson_destroy(query);

            mongoc_cursor_destroy(cursor);

            fprintf(fp_log,"%s##mongodb_bgui_process_youhui 失败\n",time_now);
            return -1;
        }
        if(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor))
        {
            if (mongoc_cursor_next (cursor, &tmp1))
            {
                bson_copy_to(tmp1,&result); //得到一条完整的记录
                if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "chargerule_name_id"))
                {
                    tmp = bson_iter_utf8(&iter,&length);

                    memcpy(chargerule,tmp,length);
                }
                bson_destroy(&result);
            }
        }
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);

        fee = mongodb_cal_fee(chargerule,in_time,time_now,type,park_id,plate);

    }

    char paycharge[24];
    memset(paycharge,0,24);
    sprintf(paycharge,"%d",fee);
    query = bson_new();
    bson_t *result1 = bson_new();
    BSON_APPEND_UTF8(query, "caroutrec_out_time",out_time);
    BSON_APPEND_UTF8(query, "caroutrec_plate_id",plate);
    result1 = BCON_NEW ("$set", "{", "caroutrec_pay_charge", BCON_UTF8 (paycharge),"updated", BCON_BOOL (true) ,"}");
    mongoc_collection_update(mongodb_table_caroutrec,MONGOC_UPDATE_NONE,query,result1,NULL,&error);
    bson_destroy(query);
    bson_destroy(result1);

    query = bson_new();
    result1 = bson_new();
    BSON_APPEND_UTF8(query, "caroutrec_out_time",out_time);
    BSON_APPEND_UTF8(query, "caroutrec_plate_id",plate);
    result1 = BCON_NEW ("$set", "{", "caroutrec_car_type", BCON_UTF8 (type),"updated", BCON_BOOL (true) ,"}");
    mongoc_collection_update(mongodb_table_caroutrec,MONGOC_UPDATE_NONE,query,result1,NULL,&error);
    bson_destroy(query);
    bson_destroy(result1);
    mongoc_collection_destroy(mongodb_table_chargerule);
    mongoc_collection_destroy(mongodb_table_carinpark);
    mongoc_collection_destroy(mongodb_table_caroutrec);
 //   mongoc_client_destroy(mongodb_client);

    Json::Value bgui_send;
    Json::Value content;

    bgui_send["cmd"] = Json::Value("changediscount");

    content["plate"] = Json::Value(plate);
    char fee_tmp[24];
    memset(fee_tmp,0,24);
    sprintf(fee_tmp,"%d",fee);
    content["charge"] = Json::Value(fee_tmp);

    bgui_send["content"] = content;

    char ip[] = {"127.0.0.1"};

    std::string send = bgui_send.toStyledString();
    struct sockaddr_in addr;
    int sock;
    sock=socket(AF_INET, SOCK_DGRAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT_UDP_BCENTER_TO_BGUI);
    addr.sin_addr.s_addr = inet_addr(ip);
    int n = sendto(sock,send.c_str(),send.length(), 0, (struct sockaddr *)&addr, sizeof(addr));
    if(n < 0)
    {
        fprintf(fp_log,"%s##bgui mongodb_bgui_process_youhui发送失败\n",time_now);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 向bgui发包失败 msg[%s] len[%ld] ip[%s] port[%d] line[%d]",
                 __FUNCTION__, send.c_str(), send.length(), ip, PORT_UDP_BCENTER_TO_BGUI, __LINE__);
        writelog(log_buf_);
    }
    else
    {
        fprintf(fp_log,"%s##bgui mongodb_bgui_process_youhui发送成功:\n",time_now);
        fprintf(fp_log,"%s\n",send.c_str());

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 向bgui发出去一个消息 msg[%s] len[%ld] ip[%s] port[%d] line[%d]",
                 __FUNCTION__, send.c_str(), send.length(), ip, PORT_UDP_BCENTER_TO_BGUI, __LINE__);
        writelog(log_buf_);
    }
    close(sock);
    return 0;

}

int mongodb_bgui_charge_pass(char *plate)
{
    snprintf(log_buf_, sizeof(log_buf_), "[%s] 放行方式: 收费放行 plate[%s] line[%d]", __FUNCTION__, plate, __LINE__);
    writelog(log_buf_);

  //  mongoc_client_t *mongodb_client;
    mongoc_collection_t *mongodb_table_channel;	//channel表
    mongoc_collection_t *mongodb_table_caroutrec;	//device表
 //   mongodb_client = mongoc_client_new(str_con);
    mongodb_table_channel = mongoc_client_get_collection(mongodb_client,"boondb","channel");   //channel表
    mongodb_table_caroutrec = mongoc_client_get_collection(mongodb_client,"boondb","caroutrec");   //device表

    mongoc_cursor_t *cursor;
    bson_t *query;
    bson_t result;
    const char *tmp;
    const bson_t *bson_t_tmp1;
    bson_iter_t iter;
    bson_error_t error;
    unsigned int length;
    int count = 0;
    char tmp2[24];

    char time_now[64];
    time_t tm;
    time_printf(time(&tm),time_now);

    Json::Value json_send;

    json_send["cmd"] = Json::Value("open_door");
    json_send["channel_id"] = Json::Value(out_channel);
    json_send["in_out"] = Json::Value("出口");
    json_send["flag"] = Json::Value("once");

    Json::Value json_send1;  //语音信息
    json_send1["cmd"] = Json::Value("sendvoice");
    json_send1["content"] = Json::Value("祝您一路顺风");
    json_send1["led_ip"] = Json::Value(out_led_ip);

    std::string send = json_send.toStyledString();
    std::string send1 = json_send1.toStyledString();

    char ip[] = {"127.0.0.1"};

    struct sockaddr_in addr;
    int sock;
    sock=socket(AF_INET, SOCK_DGRAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT_UDP_BCENTER_TO_BIPC);
    addr.sin_addr.s_addr = inet_addr(ip);
    int n = sendto(sock,send.c_str(),send.length(), 0, (struct sockaddr *)&addr, sizeof(addr));
    if(n < 0)
    {
        fprintf(fp_log,"%s##mongodb_bgui_charge_pass 发送失败\n",time_now);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] error, 向bled发送语音信息失败 msg[%s] len[%ld] ip[%s] port[%d] line[%d]",
                 __FUNCTION__, send.c_str(), send.length(), ip, PORT_UDP_BCENTER_TO_BIPC, __LINE__);
        writelog(log_buf_);
    }
    else
    {
        fprintf(fp_log,"%s##mongodb_bgui_charge_pass 发送成功:\n",time_now);
        fprintf(fp_log,"%s\n",send.c_str());

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 向bled发送出一条消息 msg[%s] len[%ld] ip[%s] port[%d] line[%d]",
                 __FUNCTION__, send.c_str(), send.length(), ip, PORT_UDP_BCENTER_TO_BIPC, __LINE__);
        writelog(log_buf_);
    }
    close(sock);

    struct sockaddr_in addr1;
    int sock1;
    sock1=socket(AF_INET, SOCK_DGRAM, 0);
    addr1.sin_family = AF_INET;
    addr1.sin_port = htons(PORT_UDP_BCENTER_TO_BLED); // 5006 --> 5002
    addr1.sin_addr.s_addr = inet_addr(ip);

    n = sendto(sock1,send1.c_str(),send1.length(), 0, (struct sockaddr *)&addr1, sizeof(addr1)); //发送语音信息
    if(n < 0)
    {
        fprintf(fp_log,"%s##mongo_send_bled 语音信息发送失败\n",time_now);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] error, 向bled发送语音消息失败 msg[%s] len[%ld] ip[%s] port[%d] line[%d]",
                 __FUNCTION__, send1.c_str(), send1.length(), ip, PORT_UDP_BCENTER_TO_BLED, __LINE__);
        writelog(log_buf_);
    }
    else
    {
        fprintf(fp_log,"%s##mongo_send_bled 语音信息发送成功:\n",time_now);
        fprintf(fp_log,"%s\n",send1.c_str());

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 向bled发送出一条消息 msg[%s] len[%ld] ip[%s] port[%d] line[%d]",
                 __FUNCTION__, send1.c_str(), send1.length(), ip, PORT_UDP_BCENTER_TO_BLED, __LINE__);
        writelog(log_buf_);
    }
    close(sock1);

    char pay_charge[24] = {0};

    query = bson_new();
    BSON_APPEND_UTF8(query, "caroutrec_out_time",out_time);
    BSON_APPEND_UTF8(query, "caroutrec_plate_id",plate);

    snprintf(log_buf_, sizeof(log_buf_), "[%s] 准备查询caroutrec表 查询条件caroutrec_out_time[%s] caroutrec_plate_id[%s] line[%d]",
             __FUNCTION__, out_time, plate, __LINE__);
    writelog(log_buf_);

    cursor = mongoc_collection_find(mongodb_table_caroutrec, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);
        mongoc_cursor_destroy(cursor);
        fprintf(fp_log,"%s##mongodb_send_bgui_start 失败\n",time_now);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 查找caroutrec表 操作数据库失败 line[%d]", __FUNCTION__, __LINE__);
        writelog(log_buf_);

        return -1;
    }

    bool find_value = false; // 是否找到数据 add diaoguangqiang
    int index_count = 1;
    while(!mongoc_cursor_error(cursor,&error) && mongoc_cursor_more(cursor))  // modify
    {
        snprintf(log_buf_, sizeof(log_buf_), "[%s] 查找caroutrec表 存在[%d]个记录 line[%d]", __FUNCTION__, index_count++, __LINE__);
        writelog(log_buf_);

        if(mongoc_cursor_next(cursor, &bson_t_tmp1))
        {
            bson_copy_to(bson_t_tmp1, &result); //得到一条完整的记录
            if (bson_iter_init(&iter, &result) &&bson_iter_find(&iter, "caroutrec_pay_charge"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memcpy(pay_charge,tmp,length);

                find_value = true; // 已经找到数据

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 查找caroutrec表 找到第[%d]个caroutrec_pay_charge[%s] line[%d]", __FUNCTION__, index_count++, pay_charge, __LINE__);
                writelog(log_buf_);
            }
            bson_destroy(&result);
        }
    }
    bson_destroy(query);
    mongoc_cursor_destroy(cursor);

    // add by diaoguangqiang
    if(strlen(pay_charge) <= 0){
        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 查询caroutrec表 查询caroutrec_pay_charge字段为空 是否找到数据find_value[%d] 查询条件caroutrec_out_time[%s] caroutrec_plate_id[%s] line[%d]",
                 __FUNCTION__, find_value, out_time, plate, __LINE__);
        writelog(log_buf_);
    } else{
        snprintf(log_buf_, sizeof(log_buf_), "[%s] 查询caroutrec表 找到caroutrec_pay_charge[%s] 车牌号[%s] 离开时间[%s] 应收金额[%s]元 把收费类型改为现金, 把实收金额改为[%s]元 line[%d]",
                 __FUNCTION__, pay_charge, plate, out_time, pay_charge, pay_charge, __LINE__);
        writelog(log_buf_);
    }

    fprintf(fp_log,"%s##更新caroutrec表，车牌号%s,离开时间 %s,应收金额%s元，把收费类型改为现金，把实收金额改为%s元\n",time_now,plate,out_time,pay_charge,pay_charge);
    query = bson_new();
    bson_t *result1 = bson_new();
    BSON_APPEND_UTF8(query, "caroutrec_out_time",out_time);
    BSON_APPEND_UTF8(query, "caroutrec_plate_id",plate);
    result1 = BCON_NEW ("$set", "{", "caroutrec_charge_type", BCON_UTF8 ("现金"),"updated", BCON_BOOL (true) ,"}");
    bool update_ret = mongoc_collection_update(mongodb_table_caroutrec,MONGOC_UPDATE_NONE,query,result1,NULL,&error); //更新car表里的车辆剩余时间
    bson_destroy(query);
    bson_destroy(result1);
    if(!update_ret){
        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 更新caroutrec表失败 车牌号caroutrec_plate_id[%s] 离开时间caroutrec_out_time[%s] 收费方式:现金 line[%d]",
                 __FUNCTION__, plate, out_time, __LINE__);
        writelog(log_buf_);
    }

    query = bson_new();
    result1 = bson_new();
    BSON_APPEND_UTF8(query, "caroutrec_out_time",out_time);
    BSON_APPEND_UTF8(query, "caroutrec_plate_id",plate);
    result1 = BCON_NEW ("$set", "{", "caroutrec_real_charge", BCON_UTF8 (pay_charge),"updated", BCON_BOOL (true) ,"}");
    update_ret = mongoc_collection_update(mongodb_table_caroutrec,MONGOC_UPDATE_NONE,query,result1,NULL,&error); //更新car表里的车辆剩余时间
    bson_destroy(query);
    bson_destroy(result1);
    if(!update_ret){
        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 更新caroutrec表失败 车牌号caroutrec_plate_id[%s] 离开时间caroutrec_out_time[%s] 收费金额[%s] line[%d]",
                 __FUNCTION__, plate, out_time, pay_charge, __LINE__);
        writelog(log_buf_);
    }

    query = bson_new();
    result1 = bson_new();
    BSON_APPEND_UTF8(query, "caroutrec_out_time",out_time);
    BSON_APPEND_UTF8(query, "caroutrec_plate_id",plate);
    result1 = BCON_NEW ("$set", "{", "caroutrec_open_door_type", BCON_UTF8 ("收费放行"),"updated", BCON_BOOL (true) ,"}");
    update_ret = mongoc_collection_update(mongodb_table_caroutrec,MONGOC_UPDATE_NONE,query,result1,NULL,&error); //更新car表里的车辆剩余时间
    bson_destroy(query);
    bson_destroy(result1);
    if(!update_ret){
        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 更新caroutrec表失败 车牌号caroutrec_plate_id[%s] 离开时间caroutrec_out_time[%s] 放行方式：收费放行 line[%d]",
                 __FUNCTION__, plate, out_time, __LINE__);
        writelog(log_buf_);
    }

    char park_id[256] = {0};
    query = bson_new();

    BSON_APPEND_UTF8(query, "channel_id",out_channel);
    cursor = mongoc_collection_find(mongodb_table_channel, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);
        mongoc_cursor_destroy(cursor);

        fprintf(fp_log,"%s##mongodb_send_bgui_start 失败\n",time_now);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 查询数据库失败 line[%d]", __FUNCTION__, __LINE__);
        writelog(log_buf_);

        return -1;
    }

    find_value = false;
    if(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor))
    {
        if (mongoc_cursor_next(cursor, &bson_t_tmp1))
        {
            bson_copy_to(bson_t_tmp1,&result); //得到一条完整的记录
            if (bson_iter_init(&iter, &result) &&bson_iter_find (&iter, "channel_park_id"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memcpy(park_id,tmp,length);

                find_value = true;

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 查询channel_park_id[%s], 查询channel, 查询条件channel_id[%s] line[%d]",
                         __FUNCTION__, park_id, out_channel, __LINE__);
                writelog(log_buf_);
            }
            bson_destroy(&result);
        }
    }
    bson_destroy(query);
    mongoc_cursor_destroy(cursor);

    if(!find_value){
        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 查找channel_park_id失败, 查询channel表, 查询条件channel_id[%s] line[%d]",
                 __FUNCTION__, out_channel, __LINE__);
        writelog(log_buf_);
    }

    mongodb_delete_car_inpark(plate,park_id);
    mongoc_collection_destroy(mongodb_table_channel);
    mongoc_collection_destroy(mongodb_table_caroutrec);

  //  mongoc_client_destroy(mongodb_client);
    return 0;
}

int mongodb_bgui_free_pass(char *plate)
{
    snprintf(log_buf_, sizeof(log_buf_), "[%s] 放行方式: 免费放行 plate[%s] line[%d]", __FUNCTION__, plate, __LINE__);
    writelog(log_buf_);

 //   mongoc_client_t *mongodb_client;
    mongoc_collection_t *mongodb_table_channel;	//channel表
    mongoc_collection_t *mongodb_table_caroutrec;	//device表
 //   mongodb_client = mongoc_client_new(str_con);
    mongodb_table_channel = mongoc_client_get_collection(mongodb_client,"boondb","channel");   //channel表
    mongodb_table_caroutrec = mongoc_client_get_collection(mongodb_client,"boondb","caroutrec");   //device表

    mongoc_cursor_t *cursor;
    bson_t *query;
    bson_t result;
    const char *tmp;
    const bson_t *tmp1;
    bson_iter_t iter;
    bson_error_t error;
    unsigned int length;
    int count = 0;
    char tmp2[24];

    char time_now[64];
    time_t tm;
    time_printf(time(&tm),time_now);

    Json::Value json_send;
    Json::Value json_send1;  //语音信息
    json_send1["cmd"] = Json::Value("sendvoice");
    json_send1["content"] = Json::Value("祝您一路顺风");
    json_send1["led_ip"] = Json::Value(out_led_ip);

    json_send["cmd"] = Json::Value("open_door");
    json_send["channel_id"] = Json::Value(out_channel);
    json_send["in_out"] = Json::Value("出口");
    json_send["flag"] = Json::Value("once");

    std::string send = json_send.toStyledString();
    std::string send1 = json_send1.toStyledString();

    char ip[] = {"127.0.0.1"};

    struct sockaddr_in addr;
    int sock;
    sock=socket(AF_INET, SOCK_DGRAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT_UDP_BCENTER_TO_BIPC);
    addr.sin_addr.s_addr = inet_addr(ip);
    int n = sendto(sock,send.c_str(),send.length(), 0, (struct sockaddr *)&addr, sizeof(addr));
    if(n < 0)
    {
        fprintf(fp_log,"%s##mongodb_bgui_charge_pass 发送失败\n",time_now);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 向bgui发包失败 msg[%s] len[%ld] ip[%s] port[%d] line[%d]",
                 __FUNCTION__, send.c_str(), send.length(), ip, PORT_UDP_BCENTER_TO_BGUI, __LINE__);
        writelog(log_buf_);
    }
    else
    {
        fprintf(fp_log,"%s##mongodb_bgui_charge_pass 发送成功:\n",time_now);
        fprintf(fp_log,"%s\n",send.c_str());

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 向bgui发出去一个消息 msg[%s] len[%ld] ip[%s] port[%d] line[%d]",
                 __FUNCTION__, send.c_str(), send.length(), ip, PORT_UDP_BCENTER_TO_BGUI, __LINE__);
        writelog(log_buf_);
    }
    close(sock);


    struct sockaddr_in addr1;
    int sock1;
    sock1=socket(AF_INET, SOCK_DGRAM, 0);
    addr1.sin_family = AF_INET;
    addr1.sin_port = htons(PORT_UDP_BCENTER_TO_BLED); // 5006 --> 5002
    addr1.sin_addr.s_addr = inet_addr(ip);

    n = sendto(sock1,send1.c_str(),send1.length(), 0, (struct sockaddr *)&addr1, sizeof(addr1)); //发送语音信息
    if(n < 0)
    {
        fprintf(fp_log,"%s##mongo_send_bled 语音信息发送失败\n",time_now);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 向bled发包失败 msg[%s] len[%ld] ip[%s] port[%d] line[%d]",
                 __FUNCTION__, send.c_str(), send.length(), ip, PORT_UDP_BCENTER_TO_BLED, __LINE__);
        writelog(log_buf_);
    }
    else
    {
        fprintf(fp_log,"%s##mongo_send_bled 语音信息发送成功:\n",time_now);
        fprintf(fp_log,"%s\n",send1.c_str());

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 向bled发出去一个消息 msg[%s] len[%ld] ip[%s] port[%d] line[%d]",
                 __FUNCTION__, send.c_str(), send.length(), ip, PORT_UDP_BCENTER_TO_BLED, __LINE__);
        writelog(log_buf_);
    }
    close(sock1);

    query = bson_new();
    bson_t *result1 = bson_new();
    BSON_APPEND_UTF8(query, "caroutrec_out_time",out_time);
    BSON_APPEND_UTF8(query, "caroutrec_plate_id",plate);

    result1 = BCON_NEW ("$set", "{", "caroutrec_charge_type", BCON_UTF8 ("免费"),"updated", BCON_BOOL (true) ,"}");
    mongoc_collection_update(mongodb_table_caroutrec,MONGOC_UPDATE_NONE,query,result1,NULL,&error); //更新car表里的车辆剩余时间
    bson_destroy(query);
    bson_destroy(result1);

    query = bson_new();
    result1 = bson_new();
    BSON_APPEND_UTF8(query, "caroutrec_out_time",out_time);
    BSON_APPEND_UTF8(query, "caroutrec_plate_id",plate);
    result1 = BCON_NEW ("$set", "{", "caroutrec_open_door_type", BCON_UTF8 ("免费放行"),"updated", BCON_BOOL (true) ,"}");
    mongoc_collection_update(mongodb_table_caroutrec,MONGOC_UPDATE_NONE,query,result1,NULL,&error); //更新car表里的车辆剩余时间
    bson_destroy(query);
    bson_destroy(result1);

    char park_id[256];
    memset(park_id,0,256);
    query = bson_new();

    BSON_APPEND_UTF8(query, "channel_id",out_channel);

    cursor = mongoc_collection_find(mongodb_table_channel, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);
        mongoc_cursor_destroy(cursor);

        fprintf(fp_log,"%s##mongodb_send_bgui_start 失败\n",time_now);
        return -1;
    }
    if(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor))
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "channel_park_id"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memcpy(park_id,tmp,length);
            }
            bson_destroy(&result);
        }
    }
    bson_destroy(query);

    mongoc_cursor_destroy(cursor);

    mongodb_delete_car_inpark(plate,park_id);
    mongoc_collection_destroy(mongodb_table_channel);
    mongoc_collection_destroy(mongodb_table_caroutrec);
  //  mongoc_client_destroy(mongodb_client);
    return 0;

}

int mongodb_bgui_modif_passwd(char *name,char *old_pass,char *new_pass)
{
  //  mongoc_client_t *mongodb_client;
    mongoc_collection_t *mongodb_table_people;	//channel表

//    mongodb_client = mongoc_client_new(str_con);
    mongodb_table_people = mongoc_client_get_collection(mongodb_client,"boondb","people");   //channel表

    mongoc_cursor_t *cursor;
    bson_t *query;
    bson_t result;
    const char *tmp;
    const bson_t *tmp1;
    bson_iter_t iter;
    bson_error_t error;
    unsigned int length;
    int count = 0;
    char tmp2[24];

    char time_now[64];
    time_t tm;
    time_printf(time(&tm),time_now);

    query = bson_new();
    BSON_APPEND_UTF8(query, "people_name",name);
    BSON_APPEND_UTF8(query, "people_pass_word",old_pass);
    count = mongoc_collection_count (mongodb_table_people, MONGOC_QUERY_NONE, query, 0, 0, NULL, &error);
    bson_destroy(query);

    if(count > 0)
    {
        query = bson_new();
        bson_t *result1 = bson_new();
        BSON_APPEND_UTF8(query, "people_name",name);
        result1 = BCON_NEW ("$set", "{", "people_pass_word", BCON_UTF8 (new_pass),"updated", BCON_BOOL (true) ,"}");
        mongoc_collection_update(mongodb_table_people,MONGOC_UPDATE_NONE,query,result1,NULL,&error);
        bson_destroy(result1);
    }
    mongoc_collection_destroy(mongodb_table_people);
   // mongoc_client_destroy(mongodb_client);

    Json::Value bgui_send;
    Json::Value content;

    bgui_send["cmd"] = Json::Value("modifypasswd");

    if(count > 0)
    {
        content["result"] = Json::Value("成功");
    }
    else
    {
        content["result"] = Json::Value("失败");
    }
    bgui_send["content"] = content;

    std::string send = bgui_send.toStyledString();

    char ip[] = {"127.0.0.1"};

    struct sockaddr_in addr;
    int sock;
    sock=socket(AF_INET, SOCK_DGRAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT_UDP_BCENTER_TO_BGUI);
    addr.sin_addr.s_addr = inet_addr(ip);
    int n = sendto(sock,send.c_str(),send.length(), 0, (struct sockaddr *)&addr, sizeof(addr));
    if(n < 0)
    {
        fprintf(fp_log,"%s##bgui mongodb_bgui_modif_passwd发送失败\n",time_now);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 向bled发包失败 msg[%s] len[%ld] ip[%s] port[%d] line[%d]",
                                              __FUNCTION__, send.c_str(), send.length(), ip, PORT_UDP_BCENTER_TO_BGUI, __LINE__);
        writelog(log_buf_);
    }
    else
    {
        fprintf(fp_log,"%s##bgui mongodb_bgui_modif_passwd发送成功:\n",time_now);
        fprintf(fp_log,"%s\n",send.c_str());

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 向bled发出去一个消息 msg[%s] len[%ld] ip[%s] port[%d] line[%d]",
                                              __FUNCTION__, send.c_str(), send.length(), ip, PORT_UDP_BCENTER_TO_BGUI, __LINE__);
        writelog(log_buf_);
    }
    close(sock);

}

int mongodb_process_chewei(char *channel_id,char *in_out,char *flag)
{
  //  mongoc_client_t *mongodb_client;
    mongoc_collection_t *mongodb_table_channel;	//channel表
    mongoc_collection_t *mongodb_table_carinpark;	//device表
    mongoc_collection_t *mongodb_table_park;	//device表
    mongoc_collection_t *mongodb_table_car;	//device表
    mongoc_collection_t *mongodb_table_carinrec;	//device表
  //  mongodb_client = mongoc_client_new(str_con);
    mongodb_table_channel = mongoc_client_get_collection(mongodb_client,"boondb","channel");   //channel表
    mongodb_table_carinpark = mongoc_client_get_collection(mongodb_client,"boondb","carinpark");   //device表
    mongodb_table_park = mongoc_client_get_collection(mongodb_client,"boondb","park");   //device表
    mongodb_table_car = mongoc_client_get_collection(mongodb_client,"boondb","car");   //device表
    mongodb_table_carinrec = mongoc_client_get_collection(mongodb_client,"boondb","carinrec");   //device表

    mongoc_cursor_t *cursor;
    bson_t *query;
    bson_t result;
    const char *tmp;
    const bson_t *tmp1;
    bson_iter_t iter;
    bson_error_t error;
    unsigned int length;
    int count = 0;
    char park_id[256];
    char park_space_remain[24];
    char park_space[24];
    char tmp2[24];

    char time_now[64];
    time_t tm;
    time_printf(time(&tm),time_now);


    memset(park_id,0,256);
    memset(park_space_remain,0,24);
    memset(park_space,0,24);
    query = bson_new();

    BSON_APPEND_UTF8(query, "channel_id",channel_id);

    cursor = mongoc_collection_find(mongodb_table_channel, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);

        fprintf(fp_log,"%s##mongodb_process_chewei 失败\n",time_now);
        return -1;
    }
    if(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor))
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "channel_park_id"))
            {
                tmp = bson_iter_utf8(&iter,&length);

                memcpy(park_id,tmp,length);
            }
            bson_destroy(&result);
        }
    }
    bson_destroy(query);

    mongoc_cursor_destroy(cursor);


    query = bson_new();

    BSON_APPEND_UTF8(query, "park_id",park_id);
    cursor = mongoc_collection_find(mongodb_table_park, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);

        fprintf(fp_log,"%s##mongodb_process_chewei 失败\n",time_now);
        return -1;
    }
    if(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor))
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "park_space_count_remain"))
            {
                tmp = bson_iter_utf8(&iter,&length);

                memcpy(park_space_remain,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "park_space_count"))
            {
                tmp = bson_iter_utf8(&iter,&length);

                memcpy(park_space,tmp,length);
            }
            bson_destroy(&result);
        }
    }
    bson_destroy(query);

    mongoc_cursor_destroy(cursor);

    count = atoi(park_space_remain);
    if(strcmp(in_out,"入口") == 0)
    {
        count = count - 1;
    }
    else
    {
        count = count + 1;
    }

    if(count < 0)
    {
        count = 0;
    }
    if(count > atoi(park_space))
    {
        count = atoi(park_space);
    }
    memset(park_space_remain,0,24);
    sprintf(park_space_remain,"%d",count);

    query = bson_new();
    bson_t *result2 = bson_new();

    BSON_APPEND_UTF8(query, "park_id",park_id);
    result2 = BCON_NEW ("$set", "{", "park_space_count_remain", BCON_UTF8 (park_space_remain),"updated", BCON_BOOL (true) ,"}");
    mongoc_collection_update(mongodb_table_park,MONGOC_UPDATE_NONE,query,result2,NULL,&error); //更新车位
    bson_destroy(result2);
    fprintf(fp_log,"%s##总车位为%d,剩余车位为%d\n",time_now,atoi(park_space),count);




    Json::Value json_msg; //发送给bled的json格式的信息
    json_msg["cmd"] = Json::Value("availablepark");

    json_msg["number"] = Json::Value(park_space_remain);
    std::string send = json_msg.toStyledString();

    char ip[] = {"127.0.0.1"};

    struct sockaddr_in addr;
    int sock;
    sock=socket(AF_INET, SOCK_DGRAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT_UDP_BCENTER_TO_BLED); // 5006 --> 5002
    addr.sin_addr.s_addr = inet_addr(ip);
    int n = sendto(sock,send.c_str(),send.length(), 0, (struct sockaddr *)&addr, sizeof(addr));
    if(n < 0)
    {
        fprintf(fp_log,"%s##bled 车位发送失败\n",time_now);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 向bled发包失败 msg[%s] len[%ld] ip[%s] port[%d] line[%d]",
                 __FUNCTION__, send.c_str(), send.length(), ip, PORT_UDP_BCENTER_TO_BLED, __LINE__);
        writelog(log_buf_);
    }
    else
    {
        fprintf(fp_log,"%s##bled 车位发送成功:\n",time_now);
        fprintf(fp_log,"%s\n",send.c_str());

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 向bled发出去一个消息 msg[%s] len[%ld] ip[%s] port[%d] line[%d]",
                 __FUNCTION__, send.c_str(), send.length(), ip, PORT_UDP_BCENTER_TO_BLED, __LINE__);
        writelog(log_buf_);
    }
    close(sock);


    if(strcmp(in_out,"入口") == 0)
    {
        query = bson_new();
        BSON_APPEND_UTF8(query, "carinpark_plate_id",in_plate);
        BSON_APPEND_UTF8(query, "carinpark_channel_id",in_channel);

        count = mongoc_collection_count (mongodb_table_carinpark, MONGOC_QUERY_NONE, query, 0, 0, NULL, &error);
        bson_destroy(query);

        fprintf(fp_log,"%s##入口开杆，车牌号为%s,通道id为%s，进入时间%s,carinpark数量为%d\n",time_now,in_plate,in_channel,in_time,count);

        if(count == 0)
        {
            char pic_path[256];
            char park_id[256];
            char car_type[24];
            char car_logo[24];
            char car_color[24];
            char people_name[24];


            memset(pic_path,0,256);
            memset(park_id,0,256);
            memset(car_logo,0,24);
            memset(car_type,0,24);
            memset(car_color,0,24);
            memset(people_name,0,24);

            query = bson_new();
            bson_t *result1 = bson_new();
            BSON_APPEND_UTF8(query, "carinrec_in_time",in_time);
            BSON_APPEND_UTF8(query, "carinrec_plate_id",in_plate);
            BSON_APPEND_UTF8(query, "carinrec_in_time",in_time);
            result1 = BCON_NEW ("$set", "{", "carinrec_open_door_type", BCON_UTF8 ("手动抬杆"),"updated", BCON_BOOL (true) ,"}");
            mongoc_collection_update(mongodb_table_carinrec,MONGOC_UPDATE_NONE,query,result1,NULL,&error);
            bson_destroy(query);
            bson_destroy(result1);

            query = bson_new();

            BSON_APPEND_UTF8(query, "carinrec_plate_id",in_plate);
            BSON_APPEND_UTF8(query, "carinrec_channel_id",in_channel);
            BSON_APPEND_UTF8(query, "carinrec_in_time",in_time);
            cursor = mongoc_collection_find(mongodb_table_carinrec, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
            if(mongoc_cursor_error(cursor,&error))
            {
                bson_destroy(query);

                mongoc_cursor_destroy(cursor);

                fprintf(fp_log,"%s##mongodb_send_bgui_start 失败\n",time_now);
                return -1;
            }
            while(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor))
            {
                if (mongoc_cursor_next (cursor, &tmp1))
                {
                    bson_copy_to(tmp1,&result); //得到一条完整的记录
                    if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "carinrec_pic_path"))
                    {
                        tmp = bson_iter_utf8(&iter,&length);

                        memcpy(pic_path,tmp,length);
                    }
                    if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "carinrec_park_id"))
                    {
                        tmp = bson_iter_utf8(&iter,&length);

                        memcpy(park_id,tmp,length);
                    }
                    if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "carinrec_car_type"))
                    {
                        tmp = bson_iter_utf8(&iter,&length);

                        memcpy(car_type,tmp,length);
                    }
                    if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "carinrec_car_logo"))
                    {
                        tmp = bson_iter_utf8(&iter,&length);

                        memcpy(car_logo,tmp,length);
                    }
                    if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "carinrec_car_color"))
                    {
                        tmp = bson_iter_utf8(&iter,&length);

                        memcpy(car_color,tmp,length);
                    }
                    bson_destroy(&result);
                }
            }
            bson_destroy(query);

            mongoc_cursor_destroy(cursor);


            query = bson_new();

            BSON_APPEND_UTF8(query, "car_plate_id",in_plate);

            cursor = mongoc_collection_find(mongodb_table_car, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
            if(mongoc_cursor_error(cursor,&error))
            {
                bson_destroy(query);

                mongoc_cursor_destroy(cursor);

                fprintf(fp_log,"%s##mongodb_send_bgui_start 失败\n",time_now);
                return -1;
            }
            while(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor))
            {
                if (mongoc_cursor_next (cursor, &tmp1))
                {
                    bson_copy_to(tmp1,&result); //得到一条完整的记录
                    if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "car_name"))
                    {
                        tmp = bson_iter_utf8(&iter,&length);

                        memcpy(people_name,tmp,length);
                    }
                    bson_destroy(&result);
                }
            }
            bson_destroy(query);

            mongoc_cursor_destroy(cursor);


            query = bson_new();
            BSON_APPEND_UTF8(query, "carinpark_in_time",in_time);
            memset(tmp2,0,24);
            sprintf(tmp2,"%ld",get_tick(in_time));
            BSON_APPEND_UTF8(query, "carinpark_time_stamp",tmp2);
            BSON_APPEND_UTF8(query, "carinpark_alg_plate","");
            BSON_APPEND_UTF8(query, "carinpark_plate_id",in_plate);
            BSON_APPEND_UTF8(query, "carinpark_pic_path",pic_path);
            BSON_APPEND_UTF8(query, "carinpark_channel_id",in_channel);
            BSON_APPEND_UTF8(query, "carinpark_park_id",park_id);
            BSON_APPEND_UTF8(query, "carinpark_car_type",car_type);
            BSON_APPEND_UTF8(query, "carinpark_car_logo",car_logo);
            BSON_APPEND_UTF8(query, "carinpark_car_color",car_color);
            BSON_APPEND_UTF8(query, "carinpark_aux_plate_id","");
            BSON_APPEND_UTF8(query, "carinpark_aux_alg_plate","");
            BSON_APPEND_UTF8(query, "carinpark_aux_car_logo","");
            BSON_APPEND_UTF8(query, "carinpark_aux_car_color","");
            BSON_APPEND_UTF8(query, "carinpark_people_name",people_name);

            if(!mongoc_collection_insert (mongodb_table_carinpark, MONGOC_INSERT_NONE, query, NULL, &error))
            {

                fprintf(fp_log,"%s##mongodb_write_carinpark 失败\n",time_now);
                bson_destroy(query);
                return -1;
            }
            bson_destroy(query);

        }

    }
    else
    {
        char park_id[256];
        memset(park_id,0,256);
        query = bson_new();

        BSON_APPEND_UTF8(query, "channel_id",out_channel);

        cursor = mongoc_collection_find(mongodb_table_car, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
        if(mongoc_cursor_error(cursor,&error))
        {
            bson_destroy(query);
            bson_destroy(&result);
            mongoc_cursor_destroy(cursor);

            fprintf(fp_log,"%s##mongodb_send_bgui_start 失败\n",time_now);
            return -1;
        }
        while(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor))
        {
            if (mongoc_cursor_next (cursor, &tmp1))
            {
                bson_copy_to(tmp1,&result); //得到一条完整的记录
                if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "channel_park_id"))
                {
                    tmp = bson_iter_utf8(&iter,&length);

                    memcpy(park_id,tmp,length);
                }
                bson_destroy(&result);
            }
        }
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);


        query = bson_new();
        BSON_APPEND_UTF8(query, "carinpark_park_id",park_id);
        BSON_APPEND_UTF8(query, "carinpark_plate_id",out_plate);

        count = mongoc_collection_count (mongodb_table_carinpark, MONGOC_QUERY_NONE, query, 0, 0, NULL, &error);

        bson_destroy(query);
        fprintf(fp_log,"%s##出口开杆，车牌号为%s,carinpark数量为%d\n",time_now,out_plate,count);
        if(count > 0)
        {
            mongodb_delete_car_inpark(out_plate,park_id);
        }

    }
    mongoc_collection_destroy(mongodb_table_channel);
    mongoc_collection_destroy(mongodb_table_car);
    mongoc_collection_destroy(mongodb_table_park);
    mongoc_collection_destroy(mongodb_table_carinrec);
    mongoc_collection_destroy(mongodb_table_carinpark);
  //  mongoc_client_destroy(mongodb_client);
    return 0;
}

int mongodb_bgui_modif_parkcount( char *count)
{
 //   mongoc_client_t *mongodb_client;
    mongoc_collection_t *mongodb_table_channel;	//channel表
    mongoc_collection_t *mongodb_table_park;	//device表
 //   mongodb_client = mongoc_client_new(str_con);
    mongodb_table_channel = mongoc_client_get_collection(mongodb_client,"boondb","channel");   //channel表
    mongodb_table_park = mongoc_client_get_collection(mongodb_client,"boondb","park");   //device表

    mongoc_cursor_t *cursor;
    bson_t *query;
    bson_t result;
    const char *tmp;
    const bson_t *tmp1;
    bson_iter_t iter;
    bson_error_t error;
    unsigned int length;

    char time_now[64];
    time_t tm;
    time_printf(time(&tm),time_now);

    char park_id[256];
    memset(park_id,0,256);
    query = bson_new();

    BSON_APPEND_UTF8(query, "channel_ip",host_ip);

    cursor = mongoc_collection_find (mongodb_table_channel, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);

        fprintf(fp_log,"%s##mongodb_process_led 失败\n",time_now);
        return -1;
    }
    while(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor)) //得到car表的每一条记录
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "channel_park_id")) //得到车牌号
            {
                tmp = bson_iter_utf8(&iter,&length);

                memcpy(park_id,tmp,length);
            }
            bson_destroy(&result);
        }
    }
    bson_destroy(query);

    mongoc_cursor_destroy(cursor);


    query = bson_new();
    bson_t *result1 = bson_new();
    BSON_APPEND_UTF8(query, "park_id",park_id); //查询条件

    result1 = BCON_NEW ("$set", "{", "park_space_count_remain", BCON_UTF8 (count),"updated", BCON_BOOL (true) ,"}");
    mongoc_collection_update(mongodb_table_park,MONGOC_UPDATE_NONE,query,result1,NULL,&error);
    bson_destroy(query);
    bson_destroy(result1);

    mongoc_collection_destroy(mongodb_table_channel);
    mongoc_collection_destroy(mongodb_table_park);
  //  mongoc_client_destroy(mongodb_client);
    return 0;
}

int mongodb_query_space_count( char *space_count)
{
  //  mongoc_client_t *mongodb_client;
    mongoc_collection_t *mongodb_table_channel;	//channel表
    mongoc_collection_t *mongodb_table_park;	//device表
 //   mongodb_client = mongoc_client_new(str_con);
    mongodb_table_channel = mongoc_client_get_collection(mongodb_client,"boondb","channel");   //channel表
    mongodb_table_park = mongoc_client_get_collection(mongodb_client,"boondb","park");   //device表

    mongoc_cursor_t *cursor;
    bson_t *query;
    bson_t result;
    const char *tmp;
    const bson_t *tmp1;
    bson_iter_t iter;
    bson_error_t error;
    unsigned int length;

    char time_now[64];
    time_t tm;
    time_printf(time(&tm),time_now);

    char park_id[256];
    memset(park_id,0,256);
    query = bson_new();

    BSON_APPEND_UTF8(query, "channel_ip",host_ip);

    cursor = mongoc_collection_find (mongodb_table_channel, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);
        //bson_destroy(result);
        mongoc_cursor_destroy(cursor);

        fprintf(fp_log,"%s##mongodb_query_space_count 失败\n",time_now);
        return -1;
    }
    if(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor)) //得到car表的每一条记录
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "channel_park_id")) //得到车牌号
            {
                tmp = bson_iter_utf8(&iter,&length);

                memcpy(park_id,tmp,length);
            }
            bson_destroy(&result);
        }
    }
    bson_destroy(query);
    //	bson_destroy(result);
    mongoc_cursor_destroy(cursor);


    query = bson_new();
    //result = bson_new();
    BSON_APPEND_UTF8(query, "park_id",park_id);
    cursor = mongoc_collection_find (mongodb_table_park, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);
        //bson_destroy(result);
        mongoc_cursor_destroy(cursor);

        fprintf(fp_log,"%s##mongodb_query_space_count 失败\n",time_now);
        return -1;
    }
    if(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor)) //得到car表的每一条记录
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "park_space_count_remain")) //得到车牌号
            {
                tmp = bson_iter_utf8(&iter,&length);

                memcpy(space_count,tmp,length);
            }
            bson_destroy(&result);
        }
    }
    bson_destroy(query);
    //bson_destroy(result);
    mongoc_cursor_destroy(cursor);
    mongoc_collection_destroy(mongodb_table_channel);
    mongoc_collection_destroy(mongodb_table_park);
   // mongoc_client_destroy(mongodb_client);
    return 0;
}

int mongodb_get_welcome_msg(int _dst_port)
{
  //  mongoc_client_t *mongodb_client;
    mongoc_collection_t *mongodb_table_channel;	//channel表
    mongoc_collection_t *mongodb_table_device;	//device表
    mongoc_collection_t *mongodb_table_ledset;	//device表
    mongoc_collection_t *mongodb_table_park_set;	//device表
 //   mongodb_client = mongoc_client_new(str_con);
    mongodb_table_channel = mongoc_client_get_collection(mongodb_client,"boondb","channel");   //channel表
    mongodb_table_device = mongoc_client_get_collection(mongodb_client,"boondb","device");   //device表
    mongodb_table_ledset = mongoc_client_get_collection(mongodb_client,"boondb","ledset");   //device表
    mongodb_table_park_set = mongoc_client_get_collection(mongodb_client,"boondb","park_set");   //device表

    mongoc_cursor_t *cursor_channel;
    mongoc_cursor_t *cursor_device;

    bson_t *query;
    bson_t result;
    bson_iter_t iter;
    const char *tmp;
    const bson_t *tmp1;
    bson_error_t error;
    unsigned int length;



    char led_row[24];
    char led_content[24];
    char led_type[24];
    char show_type[24];
    char show_text[256];
    char channel_id[256];
    char channel_in_out[24];
    char led_ip[24];

    Json::Value json_msg; //发送给bled的json格式的信息
    Json::Value json_led_in; //发送给bled的json格式的信息
    Json::Value json_led_out; //发送给bled的json格式的信息


    query = bson_new();


    cursor_channel = mongoc_collection_find (mongodb_table_ledset, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查channel表
    if(mongoc_cursor_error(cursor_channel,&error))
    {

        return -1;
    }


    while(!mongoc_cursor_error(cursor_channel,&error)&&mongoc_cursor_more(cursor_channel)) //得到小主机下属的每一个通道信息
    {


        if (mongoc_cursor_next (cursor_channel, &tmp1))
        {


            bson_copy_to(tmp1,&result); //得到一条完整的记录
            memset(led_row,0,24);
            memset(led_type,0,24);
            memset(show_type,0,24);
            memset(show_text,0,256);
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "ledset_row_id")) //得到channel_id
            {
                tmp = bson_iter_utf8(&iter,&length);
                memcpy(led_row,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "ledset_type")) //得到channel_id
            {
                tmp = bson_iter_utf8(&iter,&length);
                memcpy(led_type,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "ledset_show_type")) //得到channel_id
            {
                tmp = bson_iter_utf8(&iter,&length);
                memcpy(show_type,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "ledset_show_text")) //得到channel_id
            {
                tmp = bson_iter_utf8(&iter,&length);
                memcpy(show_text,tmp,length);
            }
            bson_destroy(&result);
            if(strcmp(led_type,"入口无车") == 0)
            {
                if(atoi(led_row) == 1)
                {
                    if(strcmp(show_type,"固定内容") == 0)
                    {
                        json_led_in["row1"] = Json::Value(show_text);
                    }
                    else
                    {
                        json_led_in["row1"] = Json::Value(show_type);
                    }
                }
                else if(atoi(led_row) == 2)
                {
                    if(strcmp(show_type,"固定内容") == 0)
                    {
                        json_led_in["row2"] = Json::Value(show_text);
                    }
                    else
                    {
                        json_led_in["row2"] = Json::Value(show_type);
                    }
                }
                else if(atoi(led_row) == 3)
                {
                    if(strcmp(show_type,"固定内容") == 0)
                    {
                        json_led_in["row3"] = Json::Value(show_text);
                    }
                    else
                    {
                        json_led_in["row3"] = Json::Value(show_type);
                    }
                }
                else if(atoi(led_row) == 4)
                {
                    if(strcmp(show_type,"固定内容") == 0)
                    {
                        json_led_in["row4"] = Json::Value(show_text);
                    }
                    else
                    {
                        json_led_in["row4"] = Json::Value(show_type);
                    }
                }
            }
            else if(strcmp(led_type,"出口无车") == 0)
            {
                if(atoi(led_row) == 1)
                {
                    if(strcmp(show_type,"固定内容") == 0)
                    {
                        json_led_out["row1"] = Json::Value(show_text);
                    }
                    else
                    {
                        json_led_out["row1"] = Json::Value(show_type);
                    }
                }
                else if(atoi(led_row) == 2)
                {
                    if(strcmp(show_type,"固定内容") == 0)
                    {
                        json_led_out["row2"] = Json::Value(show_text);
                    }
                    else
                    {
                        json_led_out["row2"] = Json::Value(show_type);
                    }
                }
                else if(atoi(led_row) == 3)
                {
                    if(strcmp(show_type,"固定内容") == 0)
                    {
                        json_led_out["row3"] = Json::Value(show_text);
                    }
                    else
                    {
                        json_led_out["row3"] = Json::Value(show_type);
                    }
                }
                else if(atoi(led_row) == 4)
                {
                    if(strcmp(show_type,"固定内容") == 0)
                    {
                        json_led_out["row4"] = Json::Value(show_text);
                    }
                    else
                    {
                        json_led_out["row4"] = Json::Value(show_type);
                    }
                }
            }
        }
    }

    bson_destroy(query);

    mongoc_cursor_destroy(cursor_channel);


    query = bson_new();

    BSON_APPEND_UTF8 (query, "channel_ip", host_ip); //查询条件

    cursor_channel = mongoc_collection_find (mongodb_table_channel, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查channel表
    if(mongoc_cursor_error(cursor_channel,&error))
    {

        return -1;
    }
    while(!mongoc_cursor_error(cursor_channel,&error)&&mongoc_cursor_more(cursor_channel)) //得到小主机下属的每一个通道信息
    {
        if (mongoc_cursor_next (cursor_channel, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            memset(channel_id,0,256);
            memset(channel_in_out,0,24);

            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "channel_id")) //得到channel_id
            {
                tmp = bson_iter_utf8(&iter,&length);
                memcpy(channel_id,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "channel_in_out")) //得到channel_id
            {
                tmp = bson_iter_utf8(&iter,&length);
                memcpy(channel_in_out,tmp,length);
            }
            bson_destroy(&result);

            bson_t *query_device;
            query_device = bson_new();
            BSON_APPEND_UTF8 (query_device, "device_channel_id", channel_id);  //查询条件
            BSON_APPEND_UTF8 (query_device, "device_type","LED语音一体机");  //查询条件
            cursor_device = mongoc_collection_find (mongodb_table_device, MONGOC_QUERY_NONE, 0, 0, 0, query_device, NULL, NULL); //根据通道id号查device表
            memset(led_ip,0,24);
            if(!mongoc_cursor_error(cursor_device,&error)&&mongoc_cursor_more(cursor_device)) //遍历该通道下每一条设备记录
            {
                if (mongoc_cursor_next (cursor_device, &tmp1))
                {
                    bson_copy_to(tmp1,&result);
                    if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "device_ip_id")) //得到channel_id
                    {
                        tmp = bson_iter_utf8(&iter,&length);
                        memcpy(led_ip,tmp,length);
                    }
                    bson_destroy(&result);
                }
            }
            if(strcmp(channel_in_out,"入口") == 0)
            {
                json_led_in["led_ip"] = Json::Value(led_ip);
            }
            else
            {
                json_led_out["led_ip"] = Json::Value(led_ip);
            }


        }
    }
    mongoc_cursor_destroy(cursor_channel);


    char park_ID[256];
    char park_name[256];
    char park_switch[256];
    memset(park_ID,0,256);
    memset(park_name,0,256);
    memset(park_switch,0,256);
    query = bson_new();

    cursor_channel = mongoc_collection_find (mongodb_table_park_set, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查channel表
    if(mongoc_cursor_error(cursor_channel,&error))
    {

        return -1;
    }
    while(!mongoc_cursor_error(cursor_channel,&error)&&mongoc_cursor_more(cursor_channel)) //得到小主机下属的每一个通道信息
    {
        if (mongoc_cursor_next (cursor_channel, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录

            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "park_ID")) //得到channel_id
            {
                tmp = bson_iter_utf8(&iter,&length);
                memcpy(park_ID,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "park_name")) //得到channel_id
            {
                tmp = bson_iter_utf8(&iter,&length);
                memcpy(park_name,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "park_switch")) //得到channel_id
            {
                tmp = bson_iter_utf8(&iter,&length);
                memcpy(park_switch,tmp,length);
            }
            bson_destroy(&result);
        }
    }
    mongoc_cursor_destroy(cursor_channel);

    mongoc_collection_destroy(mongodb_table_channel);
    mongoc_collection_destroy(mongodb_table_device);
    mongoc_collection_destroy(mongodb_table_ledset);
    mongoc_collection_destroy(mongodb_table_park_set);
 //   mongoc_client_destroy(mongodb_client);


    json_msg["cmd"] = Json::Value("welcome_msg");
    json_led_in["row_num"] = Json::Value("4");
    json_led_in["park_id"] = Json::Value(park_ID);
    json_led_in["park_name"] = Json::Value(park_name);
    json_led_in["park_switch"] = Json::Value(park_switch);
    json_led_out["park_id"] = Json::Value(park_ID);
    json_led_out["park_name"] = Json::Value(park_name);
    json_led_out["park_switch"] = Json::Value(park_switch);
    json_led_out["row_num"] = Json::Value("4");
    
    json_msg["content"].append(json_led_in);
    json_msg["content"].append(json_led_out);
    std::string send = json_msg.toStyledString();
    char time_now[64];
    time_t tm;
    time_printf(time(&tm),time_now);

    char ip[] = {"127.0.0.1"};

    struct sockaddr_in addr;
    int sock;
    sock=socket(AF_INET, SOCK_DGRAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(_dst_port);
    addr.sin_addr.s_addr = inet_addr(ip);
    int n = sendto(sock,send.c_str(),send.length(), 0, (struct sockaddr *)&addr, sizeof(addr));
    if(n < 0)
    {
        fprintf(fp_log,"%s##bled welcome msg发送失败\n",time_now);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 向bled发包失败 msg[%s] len[%ld] ip[%s] port[%d] line[%d]",
                 __FUNCTION__, send.c_str(), send.length(), ip, _dst_port, __LINE__);
        writelog(log_buf_);
    }
    else
    {
        fprintf(fp_log,"%s##bled welcome msg发送成功:\n",time_now);
        fprintf(fp_log,"%s\n",send.c_str());

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 向bled发出去一个消息 msg[%s] len[%ld] ip[%s] port[%d] line[%d]",
                 __FUNCTION__, send.c_str(), send.length(), ip, _dst_port, __LINE__);
        writelog(log_buf_);
    }
    close(sock);

    return 0;
}

//0 不放行 1 授权车放行 2 所有车放行
int mongodb_validate_full_fangxing(char *parkid)
{
    snprintf(log_buf_, sizeof(log_buf_), "[%s] line[%d]", __FUNCTION__, __LINE__);
    writelog(log_buf_);

    // mongoc_client_t *mongodb_client = mongoc_client_new(str_con);
    mongoc_collection_t *mongodb_table_park = mongoc_client_get_collection(mongodb_client,"boondb","park");//park表

    mongoc_cursor_t *cursor;
    bson_t *query;
    bson_t *query_tmp;
    bson_t result;
    bson_iter_t iter;
    const char *tmp;
    const bson_t *tmp1;
    bson_error_t error;
    unsigned int length;

    char park_space_count_remain[24] = {0};
    char full_park[24] = {0};

    query = bson_new();
    BSON_APPEND_UTF8(query, "park_id",parkid);

    cursor = mongoc_collection_find (mongodb_table_park, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
    if(mongoc_cursor_error(cursor,&error))
    {
        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 查询mongodb失败，查询park表的park_id[%s]失败 line[%d]", __FUNCTION__, parkid, __LINE__);
        writelog(log_buf_);

        bson_destroy(query);
        mongoc_cursor_destroy(cursor);

        return -1;
    }
    if(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor))
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "park_space_count_remain"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memcpy(park_space_count_remain,tmp,length);

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 查询park表 parkid[%s] 的剩余车位数 park_space_count_remain[%s] line[%d]", __FUNCTION__, parkid, park_space_count_remain, __LINE__);
                writelog(log_buf_);
            }

            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "park_full"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memcpy(full_park,tmp,length);

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 查询park表 规则为 park_full[%s] line[%d]", __FUNCTION__, full_park, __LINE__);
                writelog(log_buf_);
            }
            bson_destroy(&result);
        }
    }else{
        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 查询park表的park_id[%s]失败, 没有找到数据记录 line[%d]", __FUNCTION__, parkid, __LINE__);
        writelog(log_buf_);
    }

    bson_destroy(query);
    mongoc_cursor_destroy(cursor);
    mongoc_collection_destroy(mongodb_table_park);
    //  mongoc_client_destroy(mongodb_client);

    fprintf(fp_log,"park_space_count_remain: %s, fullpark: %s\n",park_space_count_remain,full_park);
    if(atoi(park_space_count_remain) <= 0)      //如果车位满
    {
        if(!strcmp(full_park,"不放行"))
        {
            snprintf(log_buf_, sizeof(log_buf_), "[%s] 车位数量已满，不放行 full_park[%s] line[%d]", __FUNCTION__, full_park, __LINE__);
            writelog(log_buf_);

            return 0;
        }

        if(!strcmp(full_park,"授权车放行"))
        {
            snprintf(log_buf_, sizeof(log_buf_), "[%s] 车位数量已满，授权车放行 full_park[%s] line[%d]", __FUNCTION__, full_park, __LINE__);
            writelog(log_buf_);

            return 1;
        }

        if(!strcmp(full_park,"所有车放行"))
        {
            snprintf(log_buf_, sizeof(log_buf_), "[%s] 车位数量已满，所有车放行 full_park[%s] line[%d]", __FUNCTION__, full_park, __LINE__);
            writelog(log_buf_);

            return 2;
        }
    }
    else
    {
        snprintf(log_buf_, sizeof(log_buf_), "[%s] 车位数量充足, 剩余车位数 park_space_count_remain[%s] line[%d]", __FUNCTION__, park_space_count_remain, __LINE__);
        writelog(log_buf_);

        return -1;
    }
}

// 0 不在黑名单 1 提醒 2 警告 3 禁入
int mongodb_validate_blacklist(char *plate, char *blacklistreason)
{
    //  mongoc_client_t *mongodb_client = mongoc_client_new(str_con);
    mongoc_collection_t *mongodb_table_blacklist = mongoc_client_get_collection(mongodb_client,"boondb","blacklist");//blacklist表

    mongoc_cursor_t *cursor;
    bson_t *query;
    bson_t result;
    bson_iter_t iter;
    const char *tmp;
    const bson_t *tmp1;
    bson_error_t error;
    unsigned int length;

    bool flag = false;

    char blacklist_vehicle_type[256] = {0};
    char blacklist_black_type[256] = {0};

    query = bson_new();
    BSON_APPEND_UTF8(query, "blacklist_plate",plate);

    cursor = mongoc_collection_find (mongodb_table_blacklist, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 查询blacklist记录失败, 没有找到plate[%s]的黑名单记录 line[%d]", __FUNCTION__, plate, __LINE__);
        writelog(log_buf_);

        return -1;
    }
    if(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor))
    {
        flag = true;
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "blacklist_vehicle_type"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memcpy(blacklist_vehicle_type,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "blacklist_black_type"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memcpy(blacklist_black_type,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "blacklist_instructions"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memcpy(blacklistreason,tmp,length);
            }
            bson_destroy(&result);
        }
    }
    bson_destroy(query);
    mongoc_cursor_destroy(cursor);
    mongoc_collection_destroy(mongodb_table_blacklist);

    int ret = 0;
    // mongoc_client_destroy(mongodb_client);
    if(!flag) {
        ret = 0;
    }

    if(!strcmp(blacklist_black_type,"提醒")){
        ret = 1;
    }

    if(!strcmp(blacklist_black_type,"警告")){
        ret = 2;
    }

    if(!strcmp(blacklist_black_type,"禁入")) {
        ret = 3;
    }

    if(0 != ret){
        snprintf(log_buf_, sizeof(log_buf_), "[%s] 查询黑名单blacklist记录, 车牌[%s]为黑名单 [%s] line[%d]", __FUNCTION__, plate, blacklist_black_type, __LINE__);
        writelog(log_buf_);
    }

    return ret;
}

int mongodb_query_remain_space_count(char *space_count)
{
    // mongoc_client_t *mongodb_client = mongoc_client_new(str_con);
    mongoc_collection_t *mongodb_table_park = mongoc_client_get_collection(mongodb_client,"boondb","park");//park表

    mongoc_cursor_t *cursor;
    bson_t *query;
    bson_t result;
    bson_iter_t iter;
    const char *tmp;
    const bson_t *tmp1;
    bson_error_t error;
    unsigned int length;

    query = bson_new();

    cursor = mongoc_collection_find (mongodb_table_park, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);

        return -1;
    }

    if(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor))
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "park_space_count_remain"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memcpy(space_count,tmp,length);

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 查找park表，获取剩余车位数[%s] line[%d]", __FUNCTION__, space_count, __LINE__);
                writelog(log_buf_);

            }
            bson_destroy(&result);
        }
    }
    bson_destroy(query);
    mongoc_cursor_destroy(cursor);
    mongoc_collection_destroy(mongodb_table_park);
    // mongoc_client_destroy(mongodb_client);
}

int mongodb_speak_wannengyuyin(char *in_out, char *plate, char *car_type, int fee, car_msg *msg, char *speak_send)
{
    snprintf(log_buf_, sizeof(log_buf_)-1, "[%s] 万能语音 speak_send[%s] line[%d]", __FUNCTION__, speak_send, __LINE__);
    writelog(log_buf_);

    printf("mongodb_speak_wannengyuyin === %s,%s,%s,%s\n ",in_out,plate,car_type,msg->in_time);
    // mongoc_client_t *mongodb_client = mongoc_client_new(str_con);
    mongoc_collection_t *mongodb_table_voice_save = mongoc_client_get_collection(mongodb_client,"boondb","voice_save1");//voice_save表
    mongoc_collection_t *mongodb_table_carinpark = mongoc_client_get_collection(mongodb_client,"boondb","carinpark");//voice_save表

    mongoc_cursor_t *cursor;
    bson_t *query;
    bson_t result;
    bson_iter_t iter;
    const char *tmp;
    const bson_t *tmp1;
    bson_error_t error;
    unsigned int length;

    char *content1;
    char *content2;
    char *content3;
    char *content4;
    char *content5;
    char *content6;

    char *channelIp;

    char time_now[64];
    memset(time_now,0,64);
    time_t tm;
    time_printf(time(&tm),time_now);

    int mil_now = get_tick(msg->time); //得到当前时间的秒数
    int mil_in = get_tick(msg->in_time);	 //得到进入时间的秒数
    int park_time = (mil_now - mil_in)/60;

    content1 = (char *)malloc(256);
    memset(content1,0,256);
    content2 = (char *)malloc(256);
    memset(content2,0,256);
    content3 = (char *)malloc(256);
    memset(content3,0,256);
    content4 = (char *)malloc(256);
    memset(content4,0,256);
    content5 = (char *)malloc(256);
    memset(content5,0,256);
    content6 = (char *)malloc(256);
    memset(content6,0,256);
    channelIp = (char *)malloc(256);
    memset(channelIp,0,256);

    query = bson_new();
    BSON_APPEND_UTF8(query, "voice_save_car_type1",car_type);
    BSON_APPEND_UTF8(query, "voice_save_tongdao",in_out);
    BSON_APPEND_UTF8(query, "gtip",host_ip);

    snprintf(log_buf_, sizeof(log_buf_), "[%s] 准备查询voice_save1表 查询条件 voice_save_car_type1[%s] voice_save_tongdao[%s] gtip[%s] line[%d]", __FUNCTION__, car_type, in_out, host_ip, __LINE__);
    writelog(log_buf_);

    cursor = mongoc_collection_find (mongodb_table_voice_save, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);
        mongoc_cursor_destroy(cursor);
        return -1;
    }
    if(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor))
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "voice_save_content1"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memcpy(content1,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "voice_save_content2"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memcpy(content2,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "voice_save_content3"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memcpy(content3,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "voice_save_content4"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memcpy(content4,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "voice_save_content5"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memcpy(content5,tmp,length);
            }
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "voice_save_content6"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memcpy(content6,tmp,length);
            }
            /*
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "gtip"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memcpy(channelIp,tmp,length);
            }
            */
            bson_destroy(&result);
        }
    }
    bson_destroy(query);
    mongoc_cursor_destroy(cursor);
    mongoc_collection_destroy(mongodb_table_voice_save);
    printf("show bled content %s,%s,%s,%s,%s,%s\n",content1,content2,content3,content4,content5,content6);

    snprintf(log_buf_, sizeof(log_buf_)-1, "[%s] content1[%s] content2[%s] content3[%s] content4[%s] content5[%s] content6[%s] line[%d]", __FUNCTION__, content1,content2,content3,content4,content5,content6, __LINE__);
    writelog(log_buf_);

    char *in_time;
    in_time = (char *)malloc(24);
    memset(in_time,0,24);

    query = bson_new();
    BSON_APPEND_UTF8(query, "carinpark_plate_id",plate);

    cursor = mongoc_collection_find (mongodb_table_carinpark, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);
        mongoc_cursor_destroy(cursor);
        return -1;
    }
    if(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor))
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "carinpark_in_time"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memcpy(in_time,tmp,length);
            }
            bson_destroy(&result);
        }
    }
    bson_destroy(query);
    mongoc_cursor_destroy(cursor);
    mongoc_collection_destroy(mongodb_table_carinpark);

    memset(speak_send,0,256);
    if(strcmp(in_out,"入口") == 0)
    {
        if(strcmp(content1,"车牌号") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,plate);
        }
        else if(strcmp(content1,"车辆类型") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,car_type);
        }
        else if(strcmp(content1,"欢迎光临") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,"欢迎光临");
        }
        else if(strcmp(content1,"一路顺风") == 0)
        {
            sprintf(speak_send,"%s%s", speak_send, "一路顺风");
        }
        else if(strcmp(content1,"剩余时间") == 0)
        {
            sprintf(speak_send,"%s%s", speak_send, remain_car_time);
        }
        else
        {
            sprintf(speak_send,"%s%s", speak_send, content1);

            snprintf(log_buf_, sizeof(log_buf_), "[%s] warn! 入口 无法匹配 content1[%s] line[%d]", __FUNCTION__, content1, __LINE__);
            writelog(log_buf_);
        }

        if(strcmp(content2,"车牌号") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,plate);
        }
        else if(strcmp(content2,"车辆类型") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,car_type);
        }
        else if(strcmp(content2,"欢迎光临") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,"欢迎光临");
        }
        else if(strcmp(content2,"一路顺风") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,"一路顺风");
        }
        else if(strcmp(content2,"剩余时间") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,remain_car_time);
        }
        else
        {
            sprintf(speak_send,"%s%s",speak_send,content2);

            snprintf(log_buf_, sizeof(log_buf_), "[%s] warn! 入口 无法匹配 content2[%s] line[%d]", __FUNCTION__, content2, __LINE__);
            writelog(log_buf_);
        }

        if(strcmp(content3,"车牌号") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,plate);
        }
        else if(strcmp(content3,"车辆类型") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,car_type);
        }
        else if(strcmp(content3,"欢迎光临") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,"欢迎光临");
        }
        else if(strcmp(content3,"一路顺风") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,"一路顺风");
        }
        else if(strcmp(content3,"剩余时间") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,remain_car_time);
        }
        else
        {
            sprintf(speak_send,"%s%s",speak_send,content3);

            snprintf(log_buf_, sizeof(log_buf_), "[%s] warn! 入口 无法匹配 content3[%s] line[%d]", __FUNCTION__, content3, __LINE__);
            writelog(log_buf_);
        }

        if(strcmp(content4,"车牌号") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,plate);
        }
        else if(strcmp(content4,"车辆类型") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,car_type);
        }
        else if(strcmp(content4,"欢迎光临") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,"欢迎光临");
        }
        else if(strcmp(content4,"一路顺风") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,"一路顺风");
        }
        else if(strcmp(content4,"剩余时间") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,remain_car_time);
        }
        else
        {
            sprintf(speak_send,"%s%s",speak_send,content4);

            snprintf(log_buf_, sizeof(log_buf_), "[%s] warn! 入口 无法匹配 content4[%s] line[%d]", __FUNCTION__, content4, __LINE__);
            writelog(log_buf_);
        }

        if(strcmp(content5,"车牌号") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,plate);
        }
        else if(strcmp(content5,"车辆类型") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,car_type);
        }
        else if(strcmp(content5,"欢迎光临") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,"欢迎光临");
        }
        else if(strcmp(content5,"一路顺风") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,"一路顺风");
        }
        else if(strcmp(content5,"剩余时间") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,remain_car_time);
        }
        else
        {
            sprintf(speak_send,"%s%s",speak_send,content5);

            snprintf(log_buf_, sizeof(log_buf_), "[%s] warn! 入口 无法匹配 content5[%s] line[%d]", __FUNCTION__, content5, __LINE__);
            writelog(log_buf_);
        }

        if(strcmp(content6,"车牌号") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,plate);
        }
        else if(strcmp(content6,"车辆类型") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,car_type);
        }
        else if(strcmp(content6,"欢迎光临") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,"欢迎光临");
        }
        else if(strcmp(content6,"一路顺风") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,"一路顺风");
        }
        else if(strcmp(content6,"剩余时间") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,remain_car_time);
        }
        else
        {
            sprintf(speak_send,"%s%s",speak_send,content6);

            snprintf(log_buf_, sizeof(log_buf_), "[%s] warn! 入口 无法匹配 content6[%s] line[%d]", __FUNCTION__, content6, __LINE__);
            writelog(log_buf_);
        }
    }

    if(strcmp(in_out,"出口") == 0)
    {
        if(strcmp(content1,"车牌号") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,plate);
        }
        else if(strcmp(content1,"车辆类型") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,car_type);
        }
        else if(strcmp(content1,"欢迎光临") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,"欢迎光临");
        }
        else if(strcmp(content1,"一路顺风") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,"一路顺风");
        }
        else if(strcmp(content1,"收费") == 0)
        {
            sprintf(speak_send,"%s%s%d%s",speak_send,"收费",fee,"元");
        }
        else if(strcmp(content1,"剩余时间") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,remain_car_time);
        }
        else if(strcmp(content1,"停车时长") == 0)
        {
            int day = park_time/60/24;
            int hour = park_time%(60*24)/60;
            int min = park_time%60;
            if(day == 0 && hour == 0)
            {
                sprintf(speak_send,"%s%s%d%s",speak_send,"停车时长",min,"分钟");
            }
            else if(day == 0 && hour != 0)
            {
                sprintf(speak_send,"%s%s%d%s%d%s",speak_send,"停车时长",hour,"小时",min,"分钟");
            }
            else if(day != 0 && hour == 0)
            {
                sprintf(speak_send,"%s%s%d%s%d%s",speak_send,"停车时长",day,"天",min,"分钟");
            }
            else if(day != 0 && hour != 0)
            {
                sprintf(speak_send,"%s%s%d%s%d%s%d%s",speak_send,"停车时长",day,"天",hour,"小时",min,"分钟");
            }
            else
            {
                snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 出口 1 无法匹配 day[%d] hour[%d] line[%d]", __FUNCTION__, day, hour, __LINE__);
                writelog(log_buf_);
            }
        }
        else
        {
            sprintf(speak_send,"%s%s",speak_send,content1);

            snprintf(log_buf_, sizeof(log_buf_), "[%s] warn! 出口 无法匹配 content1[%s] line[%d]", __FUNCTION__, content1, __LINE__);
            writelog(log_buf_);
        }

        if(strcmp(content2,"车牌号") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,plate);
        }
        else if(strcmp(content2,"车辆类型") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,car_type);
        }
        else if(strcmp(content2,"欢迎光临") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,"欢迎光临");
        }
        else if(strcmp(content2,"一路顺风") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,"一路顺风");
        }
        else if(strcmp(content2,"收费") == 0)
        {
            sprintf(speak_send,"%s%s%d%s",speak_send,"收费",fee,"元");
        }
        else if(strcmp(content2,"剩余时间") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,remain_car_time);
        }
        else if(strcmp(content2,"停车时长") == 0)
        {
            int day = park_time/60/24;
            int hour = park_time%(60*24)/60;
            int min = park_time%60;
            if(day == 0 && hour == 0)
            {
                sprintf(speak_send,"%s%s%d%s",speak_send,"停车时长",min,"分钟");
            }
            else if(day == 0 && hour != 0)
            {
                sprintf(speak_send,"%s%s%d%s%d%s",speak_send,"停车时长",hour,"小时",min,"分钟");
            }
            else if(day != 0 && hour == 0)
            {
                sprintf(speak_send,"%s%s%d%s%d%s",speak_send,"停车时长",day,"天",min,"分钟");
            }
            else if(day != 0 && hour != 0)
            {
                sprintf(speak_send,"%s%s%d%s%d%s%d%s",speak_send,"停车时长",day,"天",hour,"小时",min,"分钟");
            }
            else
            {
                snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 出口 2 无法匹配 day[%d] hour[%d] line[%d]", __FUNCTION__, day, hour, __LINE__);
                writelog(log_buf_);
            }
        }
        else
        {
            sprintf(speak_send,"%s%s",speak_send,content2);

            snprintf(log_buf_, sizeof(log_buf_), "[%s] warn! 出口 无法匹配 content2[%s] line[%d]", __FUNCTION__, content2, __LINE__);
            writelog(log_buf_);
        }

        if(strcmp(content3,"车牌号") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,plate);
        }
        else if(strcmp(content3,"车辆类型") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,car_type);
        }
        else if(strcmp(content3,"欢迎光临") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,"欢迎光临");
        }
        else if(strcmp(content3,"一路顺风") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,"一路顺风");
        }
        else if(strcmp(content3,"收费") == 0)
        {
            sprintf(speak_send,"%s%s%d%s",speak_send,"收费",fee,"元");
        }
        else if(strcmp(content3,"剩余时间") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,remain_car_time);
        }
        else if(strcmp(content3,"停车时长") == 0)
        {
            int day = park_time/60/24;
            int hour = park_time%(60*24)/60;
            int min = park_time%60;
            if(day == 0 && hour == 0)
            {
                sprintf(speak_send,"%s%s%d%s",speak_send,"停车时长",min,"分钟");
            }
            else if(day == 0 && hour != 0)
            {
                sprintf(speak_send,"%s%s%d%s%d%s",speak_send,"停车时长",hour,"小时",min,"分钟");
            }
            else if(day != 0 && hour == 0)
            {
                sprintf(speak_send,"%s%s%d%s%d%s",speak_send,"停车时长",day,"天",min,"分钟");
            }
            else if(day != 0 && hour != 0)
            {
                sprintf(speak_send,"%s%s%d%s%d%s%d%s",speak_send,"停车时长",day,"天",hour,"小时",min,"分钟");
            }
            else
            {
                snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 出口 3 无法匹配 day[%d] hour[%d] line[%d]", __FUNCTION__, day, hour, __LINE__);
                writelog(log_buf_);
            }
        }
        else
        {
            sprintf(speak_send,"%s%s",speak_send,content3);

            snprintf(log_buf_, sizeof(log_buf_), "[%s] warn! 出口 无法匹配 content3[%s] line[%d]", __FUNCTION__, content3, __LINE__);
            writelog(log_buf_);
        }

        if(strcmp(content4,"车牌号") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,plate);
        }
        else if(strcmp(content4,"车辆类型") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,car_type);
        }
        else if(strcmp(content4,"欢迎光临") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,"欢迎光临");
        }
        else if(strcmp(content4,"一路顺风") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,"一路顺风");
        }
        else if(strcmp(content4,"收费") == 0)
        {
            sprintf(speak_send,"%s%s%d%s",speak_send,"收费",fee,"元");
        }
        else if(strcmp(content4,"剩余时间") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,remain_car_time);
        }
        else if(strcmp(content4,"停车时长") == 0)
        {
            int day = park_time/60/24;
            int hour = park_time%(60*24)/60;
            int min = park_time%60;
            if(day == 0 && hour == 0)
            {
                sprintf(speak_send,"%s%s%d%s",speak_send,"停车时长",min,"分钟");
            }
            else if(day == 0 && hour != 0)
            {
                sprintf(speak_send,"%s%s%d%s%d%s",speak_send,"停车时长",hour,"小时",min,"分钟");
            }
            else if(day != 0 && hour == 0)
            {
                sprintf(speak_send,"%s%s%d%s%d%s",speak_send,"停车时长",day,"天",min,"分钟");
            }
            else if(day != 0 && hour != 0)
            {
                sprintf(speak_send,"%s%s%d%s%d%s%d%s",speak_send,"停车时长",day,"天",hour,"小时",min,"分钟");
            }
            else
            {
                snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 出口 4 无法匹配 day[%d] hour[%d] line[%d]", __FUNCTION__, day, hour, __LINE__);
                writelog(log_buf_);
            }
        }
        else
        {
            sprintf(speak_send,"%s%s",speak_send,content4);

            snprintf(log_buf_, sizeof(log_buf_), "[%s] warn! 出口 无法匹配 content4[%s] line[%d]", __FUNCTION__, content4, __LINE__);
            writelog(log_buf_);
        }

        if(strcmp(content5,"车牌号") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,plate);
        }
        else if(strcmp(content5,"车辆类型") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,car_type);
        }
        else if(strcmp(content5,"欢迎光临") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,"欢迎光临");
        }
        else if(strcmp(content5,"一路顺风") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,"一路顺风");
        }
        else if(strcmp(content5,"收费") == 0)
        {
            sprintf(speak_send,"%s%s%d%s",speak_send,"收费",fee,"元");
        }
        else if(strcmp(content5,"剩余时间") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,remain_car_time);
        }
        else if(strcmp(content5,"停车时长") == 0)
        {
            int day = park_time/60/24;
            int hour = park_time%(60*24)/60;
            int min = park_time%60;
            if(day == 0 && hour == 0)
            {
                sprintf(speak_send,"%s%s%d%s",speak_send,"停车时长",min,"分钟");
            }
            else if(day == 0 && hour != 0)
            {
                sprintf(speak_send,"%s%s%d%s%d%s",speak_send,"停车时长",hour,"小时",min,"分钟");
            }
            else if(day != 0 && hour == 0)
            {
                sprintf(speak_send,"%s%s%d%s%d%s",speak_send,"停车时长",day,"天",min,"分钟");
            }
            else if(day != 0 && hour != 0)
            {
                sprintf(speak_send,"%s%s%d%s%d%s%d%s",speak_send,"停车时长",day,"天",hour,"小时",min,"分钟");
            }
            else
            {
                snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 出口 5 无法匹配 day[%d] hour[%d] line[%d]", __FUNCTION__, day, hour, __LINE__);
                writelog(log_buf_);
            }
        }
        else
        {
            sprintf(speak_send,"%s%s",speak_send,content5);

            snprintf(log_buf_, sizeof(log_buf_), "[%s] warn! 出口 无法匹配 content5[%s] line[%d]", __FUNCTION__, content5, __LINE__);
            writelog(log_buf_);
        }

        if(strcmp(content6,"车牌号") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,plate);
        }
        else if(strcmp(content6,"车辆类型") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,car_type);
        }
        else if(strcmp(content6,"欢迎光临") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,"欢迎光临");
        }
        else if(strcmp(content6,"一路顺风") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,"一路顺风");
        }
        else if(strcmp(content6,"收费") == 0)
        {
            sprintf(speak_send,"%s%s%d%s",speak_send,"收费",fee,"元");
        }
        else if(strcmp(content6,"剩余时间") == 0)
        {
            sprintf(speak_send,"%s%s",speak_send,remain_car_time);
        }
        else if(strcmp(content6,"停车时长") == 0)
        {
            int day = park_time/60/24;
            int hour = park_time%(60*24)/60;
            int min = park_time%60;
            if(day == 0 && hour == 0)
            {
                sprintf(speak_send,"%s%s%d%s",speak_send,"停车时长",min,"分钟");
            }
            else if(day == 0 && hour != 0)
            {
                sprintf(speak_send,"%s%s%d%s%d%s",speak_send,"停车时长",hour,"小时",min,"分钟");
            }
            else if(day != 0 && hour == 0)
            {
                sprintf(speak_send,"%s%s%d%s%d%s",speak_send,"停车时长",day,"天",min,"分钟");
            }
            else if(day != 0 && hour != 0)
            {
                sprintf(speak_send,"%s%s%d%s%d%s%d%s",speak_send,"停车时长",day,"天",hour,"小时",min,"分钟");
            }
            else
            {
                snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 出口 6 无法匹配 day[%d] hour[%d] line[%d]", __FUNCTION__, day, hour, __LINE__);
                writelog(log_buf_);
            }
        }
        else
        {
            sprintf(speak_send,"%s%s",speak_send,content6);

            snprintf(log_buf_, sizeof(log_buf_), "[%s] warn! 出口 无法匹配 content6[%s] line[%d]", __FUNCTION__, content6, __LINE__);
            writelog(log_buf_);
        }
    }

    free(content1);
    free(content2);
    free(content3);
    free(content4);
    free(content5);
    free(content6);
    free(channelIp);

    snprintf(log_buf_, sizeof(log_buf_), "[%s] speak_send[%s] line[%d]", __FUNCTION__, speak_send, __LINE__);
    writelog(log_buf_);

    //  mongoc_client_destroy(mongodb_client);
}

/**
 * @brief: 发送udp消息
 * @param _msg ：发送数据包
 * @param _ip ： 目标ip
 * @param _port : 目标端口
 * @return :返回发送的数据包
 */
int mogodbb_process_udp_send(const std::string& _msg, const std::string& _ip, const int& _port)
{
    struct sockaddr_in addr;
    int sock;
    sock=socket(AF_INET, SOCK_DGRAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(_port);
    addr.sin_addr.s_addr = inet_addr(_ip.c_str());

    int n = sendto(sock, _msg.c_str(), _msg.length(), 0, (struct sockaddr *)&addr, sizeof(addr));
    if(n < 0)
    {
        snprintf(log_buf_, sizeof(log_buf_), "[%s] erorr! 发包失败{需检查网络连接状态} len[%ld] ip[%s] port[%d] msg[%s] line[%d]", __FUNCTION__, _msg.length(), _ip.c_str(), _port, _msg.c_str(), __LINE__);
        writelog(log_buf_);
    }
    else
    {
        snprintf(log_buf_, sizeof(log_buf_), "[%s] 发出去一个消息包 len[%ld] ip[%s] port[%d] msg[%s] line[%d]", __FUNCTION__, _msg.length(), _ip.c_str(), _port, _msg.c_str(), __LINE__);
        writelog(log_buf_);
    }
    close(sock);

    return n;
}

/**
 * @brief: 获取json数据
 * @param _cmd ：命令类型
 * @param _rsp ：返回消息
 * @param _flag ：返回正确与否的标记
 * @return ：获取json包失败
 */
bool mongodb_get_jsondata(const std::string& _cmd, std::string& _rsp, bool _flag/* = true*/)
{
    bool ret = true;

    // 拼接标准的json包
    rapidjson::Document doc;
    doc.SetObject();

    Value value(rapidjson::kObjectType);

    value.SetString(StringRef(_cmd.c_str()));
    doc.AddMember("cmd", value, doc.GetAllocator());

    if(0 == _cmd.compare(C_CMD_SMQ_PAY)){
        if(_flag){
            doc.AddMember("recv", "ok", doc.GetAllocator());
        }else{
            doc.AddMember("recv", "fail", doc.GetAllocator());
        }
    }else{
        doc.AddMember("ret", "bcenter error!", doc.GetAllocator());
        ret = false;
    }

    rapidjson::StringBuffer buffer;//in rapidjson/stringbuffer.h
    rapidjson::Writer<StringBuffer> writer(buffer); //in rapidjson/writer.h
    doc.Accept(writer);

    _rsp = buffer.GetString(); //json_msg.toStyledString();

    if(!ret){
        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 拼接消息异常 无法识别消息类型 cmd[%s] line[%d]", __FUNCTION__, _cmd.c_str(), __LINE__);
        writelog(log_buf_);
    }

    return ret;
}

/**
 * @brief: 出口，支付完成，开闸
 * @param money
 * @param park_id
 * @param box_ip
 * @param plate
 * @param openid
 * @param flag
 * @param userid
 * @return -1， 失败
 *          0, 成功
 */
int mongodb_process_wx_pay_open( char *money,char *park_id,char * box_ip,char *plate,
                                 char *_in_openid, char *flag, char* _in_userid,
                                 const char* _in_dis_money, const char* _in_fact_money)
{
    snprintf(log_buf_, sizeof(log_buf_), "[%s] 处理出场逻辑 money[%s] dis_money[%s] fact_money[%s] park_id[%s] box_ip[%s] plate[%s] openid[%s] userid[%s] flag[%s] line[%d]",
             __FUNCTION__, money, _in_dis_money, _in_fact_money, park_id, box_ip, plate, _in_openid, _in_userid, flag, __LINE__);
    writelog(log_buf_);

    std::string userid_str = _in_userid;
    std::string openid_str = _in_openid;

    std::string wx_zfb_id = "";
    std::string wx_zfb_cmd = "";

    if(!openid_str.empty() && openid_str.length() > 0){
        wx_zfb_id = openid_str;
        wx_zfb_cmd = "微信";
    }else if(!userid_str.empty() && userid_str.length() > 0){
        wx_zfb_id = userid_str;
        wx_zfb_cmd = "支付宝";
    }else{
        snprintf(log_buf_, sizeof(log_buf_), "[%s] 入参异常 openid[%s] userid[%s] line[%d]", __FUNCTION__, _in_openid, _in_userid, __LINE__);
        writelog(log_buf_);
    }

    char time_now[64];
    time_t tm;
    time_printf(time(&tm),time_now);

    mongoc_collection_t *mongodb_table_wx_zfb = mongoc_client_get_collection(mongodb_client,"boondb","wx_zfb");   //wx表

    mongoc_cursor_t *cursor;

    bson_t *query;
    bson_t result;
    bson_iter_t iter;
    const char *tmp;
    const bson_t *tmp1;
    bson_error_t error;
    unsigned int length;

    query = bson_new();

    BSON_APPEND_UTF8(query, "plate", plate);
    BSON_APPEND_UTF8(query, "sum_money", money);
    BSON_APPEND_UTF8(query, "dis_money", _in_dis_money);
    BSON_APPEND_UTF8(query, "fact_money", _in_fact_money);
    BSON_APPEND_UTF8(query, "park_id", park_id);
    BSON_APPEND_UTF8(query, "box_ip", box_ip); // 通道ip
    BSON_APPEND_UTF8(query, "id", wx_zfb_id.c_str()); // openid / userid
    BSON_APPEND_UTF8(query, "flag", flag); // open/pay
    BSON_APPEND_UTF8(query, "time", time_now); //
    BSON_APPEND_UTF8(query, "cmd", wx_zfb_cmd.c_str()); // 支付宝\微信

    bool insert_ret = true;

    // step1 写入数据库
    if(!mongoc_collection_insert (mongodb_table_wx_zfb, MONGOC_INSERT_NONE, query, NULL, &error))
    {
        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 插入wx_zfb表失败 line[%d]", __FUNCTION__, __LINE__);
        writelog(log_buf_);

        fprintf(fp_log,"%s##mongodb_process_wx_pay 失败\n",time_now);
        bson_destroy(query);

        insert_ret = false;

        //bson_destroy(query);
        //mongoc_collection_destroy(mongodb_table_wx);
        //  mongoc_client_destroy(mongodb_client);

        //return -1;
    }else{

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 写入wx_zfb表成功 plate[%s] sum_money[%s] dis_money[%s] fact_money[%s] park_id[%s] box_ip[%s] id[%s] flag[%s] time[%s] cmd[%s] line[%d]",
                 __FUNCTION__, plate, money, _in_dis_money, _in_fact_money, park_id, box_ip, wx_zfb_id.c_str(), flag, time_now, wx_zfb_cmd.c_str(), __LINE__);
        writelog(log_buf_);

        bson_destroy(query);
        mongoc_collection_destroy(mongodb_table_wx_zfb);
        //  mongoc_client_destroy(mongodb_client);
    }

    // step2 发送抬起杆信号
    std::string recv_flag = flag;
    if(0 == recv_flag.compare("open")){ // 如果是open，说明是场外支付，即需要出场,此时需要发送太杆信号

        // 向bled发送抬杆信号
        mongodb_process_wx_opendoor();

    }else{
        snprintf(log_buf_, sizeof(log_buf_), "[%s] 场内支付不抬杆 flag[%s] line[%d]", __FUNCTION__, flag, __LINE__);
        writelog(log_buf_);
    }

    // step3 发送支付成功消息
    // 拼接标准的json包
    rapidjson::Document doc;
    doc.SetObject();

    Value value(rapidjson::kObjectType);

    doc.AddMember("cmd", C_CMD_PAY, doc.GetAllocator());

    value.SetString(StringRef(park_id));
    doc.AddMember("park_id", value, doc.GetAllocator());

    value.SetString(StringRef(_in_openid));
    doc.AddMember("openid", value, doc.GetAllocator());

    value.SetString(StringRef(_in_userid));
    doc.AddMember("userid", value, doc.GetAllocator());

    if(insert_ret){
        doc.AddMember("ret", "ok", doc.GetAllocator());
    }else{
        doc.AddMember("ret", "fail", doc.GetAllocator());
    }

    rapidjson::StringBuffer buffer;//in rapidjson/stringbuffer.h
    rapidjson::Writer<StringBuffer> writer(buffer); //in rapidjson/writer.h
    doc.Accept(writer);

    std::string send = buffer.GetString(); //json_msg.toStyledString();

    return mogodbb_process_wx_udp_send(send, host_server_ip, PORT_UDP_BCENTER_TO_ONCALLCLIENT);
}

/**
 * @brief: 场内支付，支付金额入库
 * @param money
 * @param park_id
 * @param box_ip
 * @param plate
 * @param openid
 * @param flag
 * @param userid
 * @return
 */
int mongodb_process_wx_pay_in(const char *_in_sum_money, const char *park_id, const char *plate,
                              const char *_in_openid, const char* _in_userid, const char *flag,
                              const char* _in_dis_money, const char* _in_fact_money)
{
    snprintf(log_buf_, sizeof(log_buf_), "[%s] 处理场内支付 money[%s] dis_money[%s] fact_money[%s] park_id[%s] plate[%s] openid[%s] userid[%s] flag[%s] line[%d]",
             __FUNCTION__, _in_sum_money, _in_dis_money, _in_fact_money, park_id, plate, _in_openid, _in_userid, flag, __LINE__);
    writelog(log_buf_);

    std::string userid_str = _in_userid;
    std::string openid_str = _in_openid;

    std::string wx_zfb_id = "";
    std::string wx_zfb_cmd = "";

    if(!openid_str.empty() && openid_str.length() > 0){
        wx_zfb_id = openid_str;
        wx_zfb_cmd = "微信";
    }else if(!userid_str.empty() && userid_str.length() > 0){
        wx_zfb_id = userid_str;
        wx_zfb_cmd = "支付宝";
    }else{
        snprintf(log_buf_, sizeof(log_buf_), "[%s] 入参异常 openid[%s] userid[%s] line[%d]", __FUNCTION__, _in_openid, _in_userid, __LINE__);
        writelog(log_buf_);
    }

    char time_now[64];
    time_t tm;
    time_printf(time(&tm),time_now);

    mongoc_collection_t *mongodb_table_wx_zfb = mongoc_client_get_collection(mongodb_client,"boondb","wx_zfb");   //wx表

    mongoc_cursor_t *cursor;

    bson_t *query;
    bson_t result;
    bson_iter_t iter;
    const char *tmp;
    const bson_t *tmp1;
    bson_error_t error;
    unsigned int length;

    query = bson_new();

    BSON_APPEND_UTF8(query, "plate", plate);
    BSON_APPEND_UTF8(query, "sum_money", _in_sum_money);
    BSON_APPEND_UTF8(query, "dis_money", _in_dis_money);
    BSON_APPEND_UTF8(query, "fact_money", _in_fact_money);
    BSON_APPEND_UTF8(query, "park_id", park_id);
    BSON_APPEND_UTF8(query, "id", wx_zfb_id.c_str()); // openid / userid
    BSON_APPEND_UTF8(query, "flag", flag); // open/pay
    BSON_APPEND_UTF8(query, "time", time_now); //
    BSON_APPEND_UTF8(query, "cmd", wx_zfb_cmd.c_str()); // 支付宝\微信

    bool insert_ret = true;

    // step1 写入数据库
    if(!mongoc_collection_insert (mongodb_table_wx_zfb, MONGOC_INSERT_NONE, query, NULL, &error))
    {
        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 插入wx_zfb表失败 line[%d]", __FUNCTION__, __LINE__);
        writelog(log_buf_);

        bson_destroy(query);

        insert_ret = false;

        //bson_destroy(query);
        //mongoc_collection_destroy(mongodb_table_wx);
        //  mongoc_client_destroy(mongodb_client);

        //return -1;
    }else{

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 写入wx表成功 plate[%s] sum_money[%s] dis_money[%s] fact_money[%s] park_id[%s] id[%s] flag[%s] time[%s] cmd[%s] line[%d]",
                 __FUNCTION__, plate, _in_sum_money, _in_dis_money, _in_fact_money, park_id, wx_zfb_id.c_str(), flag, time_now, wx_zfb_cmd.c_str(), __LINE__);
        writelog(log_buf_);

        bson_destroy(query);
        mongoc_collection_destroy(mongodb_table_wx_zfb);
        //  mongoc_client_destroy(mongodb_client);
    }

    // step2 发送抬起杆信号
    std::string recv_flag = flag;
    if(0 == recv_flag.compare("open")){ // 如果是open，说明是场外支付，即需要出场,此时需要发送太杆信号

        // 向bled发送抬杆信号
        mongodb_process_wx_opendoor();

    }else{
        snprintf(log_buf_, sizeof(log_buf_), "[%s] 场内支付 不抬杆 flag[%s] line[%d]", __FUNCTION__, flag, __LINE__);
        writelog(log_buf_);
    }

    // step3 发送支付成功消息
    // 拼接标准的json包
    rapidjson::Document doc;
    doc.SetObject();

    Value value(rapidjson::kObjectType);

    doc.AddMember("cmd", C_CMD_PAY, doc.GetAllocator());

    value.SetString(StringRef(park_id));
    doc.AddMember("park_id", value, doc.GetAllocator());

    value.SetString(StringRef(_in_openid));
    doc.AddMember("openid", value, doc.GetAllocator());

    value.SetString(StringRef(_in_userid));
    doc.AddMember("userid", value, doc.GetAllocator());

    if(insert_ret){
        doc.AddMember("ret", "ok", doc.GetAllocator());
    }else{
        doc.AddMember("ret", "fail", doc.GetAllocator());
    }

    rapidjson::StringBuffer buffer;//in rapidjson/stringbuffer.h
    rapidjson::Writer<StringBuffer> writer(buffer); //in rapidjson/writer.h
    doc.Accept(writer);

    std::string send = buffer.GetString(); //json_msg.toStyledString();

    return mogodbb_process_wx_udp_send(send, host_server_ip, PORT_UDP_BCENTER_TO_ONCALLCLIENT);
}

/**
 * @brief :场内 查询支付费用
 * @param park_id
 * @param box_ip
 * @param plate
 * @param openid
 * @param userid
 * @param outime
 * @return
 */
int mongodb_process_wx_query_fee_in(const char* _in_cmd, const char *_in_park_id, const char *_in_plate, const char *_in_openid, const char* _in_userid, const char *_in_outime)
{
    snprintf(log_buf_, sizeof(log_buf_), "[%s] 场内查询费用 park_id[%s] plate[%s] openid[%s] userid[%s] outtime[%s] line[%d]",
             __FUNCTION__, _in_park_id, _in_plate, _in_openid, _in_userid, _in_outime, __LINE__);
    writelog(log_buf_);

    char time_now[64];
    time_t tm;
    time_printf(time(&tm),time_now);

    //  mongoc_client_t *mongodb_client;
    mongoc_collection_t *mongodb_table_car;	//car表
    mongoc_collection_t *mongodb_table_park;	//car表
    mongoc_collection_t *mongodb_table_carinpark;	//device表
    mongoc_collection_t *mongodb_table_chargerule;	//device表
    mongoc_collection_t *mongodb_table_park_set;	//device表
    mongoc_collection_t *mongodb_table_wx_zfb;	//device表
    //  mongodb_client = mongoc_client_new(str_con);

    mongodb_table_car = mongoc_client_get_collection(mongodb_client,"boondb","car");   //channel表
    mongodb_table_carinpark = mongoc_client_get_collection(mongodb_client,"boondb","carinpark");   //device表
    mongodb_table_chargerule = mongoc_client_get_collection(mongodb_client,"boondb","chargerule");   //device表
    mongodb_table_park = mongoc_client_get_collection(mongodb_client,"boondb","park");   //device表
    mongodb_table_park_set = mongoc_client_get_collection(mongodb_client,"boondb","park_set");   //device表
    mongodb_table_wx_zfb = mongoc_client_get_collection(mongodb_client,"boondb","wx_zfb");   //device表

    mongoc_cursor_t *cursor;

    bson_t *query;
    bson_t *query_tmp;
    bson_t result;
    bson_iter_t iter;
    const char *tmp;
    const bson_t *tmp1;
    bson_error_t error;
    unsigned int length;

    char cartype[24] = {0};
    char parkid[128] = {0};
    char parkname[128] = {0};
    char in_time[24] = {0};
    char out_time[24] = {0};
    char charge_rule[256] = {0};
    char plate[128] = {0};
    char tmp2[24] = {0};

    snprintf(out_time, sizeof(out_time), "%s", _in_outime);

    // 拼接标准的json包
    rapidjson::Document doc;
    doc.SetObject();
    Value value(rapidjson::kObjectType);

    value.SetString(StringRef(_in_cmd));
    doc.AddMember("cmd", value, doc.GetAllocator());
    value.SetString(StringRef(_in_park_id));
    doc.AddMember("park_id", value, doc.GetAllocator());
    value.SetString(StringRef(_in_plate));
    doc.AddMember("plate", value, doc.GetAllocator());
    value.SetString(StringRef(_in_openid));
    doc.AddMember("openid", value, doc.GetAllocator());
    value.SetString(StringRef(_in_userid));
    doc.AddMember("userid", value, doc.GetAllocator());
    doc.AddMember("money", "0", doc.GetAllocator());
    doc.AddMember("ret", "fail", doc.GetAllocator());

    rapidjson::StringBuffer buffer;//in rapidjson/stringbuffer.h
    rapidjson::Writer<StringBuffer> writer(buffer); //in rapidjson/writer.h

    // 车牌长度异常
    if(strlen(_in_plate) < 3)
    {
        // 场内支付，web必须传入车牌号
        //strncpy(plate, out_plate_last,strlen(out_plate_last));

        snprintf(log_buf_, sizeof(log_buf_), "[%s] error, 严重错误， 场内支付时，没有收到车牌号 车牌号长度len[%ld], 取车牌out_plate_last[%s] line[%d]", __FUNCTION__, strlen(_in_plate), plate, __LINE__);
        writelog(log_buf_);

        doc.Accept(writer);
        mogodbb_process_wx_udp_send(buffer.GetString(), host_server_ip, PORT_UDP_BCENTER_TO_ONCALLCLIENT);

        return -1;
    }

    strncpy(plate, _in_plate,strlen(_in_plate));

    // step1 根据车牌号在car表里面查询车辆类型, 查询出临时车，贵宾车，月租车，
    mongodb_query_cartype(plate, cartype);

    query = bson_new();

    // step2 查询在场表，
    BSON_APPEND_UTF8(query, "carinpark_plate_id", plate); //142342
    cursor = mongoc_collection_find (mongodb_table_carinpark, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);
        mongoc_cursor_destroy(cursor);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 没有在carinpark表找到车牌[%s]的在场记录, return line[%d]", __FUNCTION__, plate, __LINE__);
        writelog(log_buf_);

        doc.Accept(writer);
        mogodbb_process_wx_udp_send(buffer.GetString(), host_server_ip, PORT_UDP_BCENTER_TO_ONCALLCLIENT);

        return -1;
    }

    bool find_value = false;
    while(!mongoc_cursor_error(cursor, &error) && mongoc_cursor_more(cursor)) //得到car表的每一条记录
    {
        //snprintf(log_buf_, sizeof(log_buf_), "[%s] begin line[%d]", __FUNCTION__, __LINE__);
        //writelog(log_buf_);

        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) && bson_iter_find (&iter, "carinpark_in_time"))
            {
                tmp = bson_iter_utf8(&iter, &length);

                memcpy(in_time,tmp,length);

                find_value = true;

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 在carinpark表中找到车牌[%s]的入场时间[%s] line[%d]", __FUNCTION__, plate, in_time, __LINE__);
                writelog(log_buf_);
            }
            bson_destroy(&result);
        }

        //snprintf(log_buf_, sizeof(log_buf_), "[%s] end line[%d]", __FUNCTION__, __LINE__);
        //writelog(log_buf_);
    }
    bson_destroy(query);
    mongoc_cursor_destroy(cursor);

    if(!find_value){
        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 在carinpark表 没有找到车牌[%s]的入场时间 line[%d]", __FUNCTION__, plate, __LINE__);
        writelog(log_buf_);

        doc.Accept(writer);

        mogodbb_process_wx_udp_send(buffer.GetString(), host_server_ip, PORT_UDP_BCENTER_TO_ONCALLCLIENT);

        return -1;
    }

    query = bson_new();

    find_value = false;
    // 查询收费规则
    BSON_APPEND_UTF8(query, "chargerule_car_type", cartype);
    cursor = mongoc_collection_find (mongodb_table_chargerule, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 没有在chargerule表中找车辆类型[%s]对应的收费规则, return line[%d]", __FUNCTION__, cartype, __LINE__);
        writelog(log_buf_);

        doc.Accept(writer);
        mogodbb_process_wx_udp_send(buffer.GetString(), host_server_ip, PORT_UDP_BCENTER_TO_ONCALLCLIENT);

        return -1;
    }
    if(!mongoc_cursor_error(cursor,&error) && mongoc_cursor_more(cursor)) //得到car表的每一条记录
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1, &result); //得到一条完整的记录
            if (bson_iter_init(&iter, &result) && bson_iter_find (&iter, "chargerule_name_id"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(charge_rule,0,256);
                memcpy(charge_rule,tmp,length);

                find_value = true;

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 在chargerule表中找到车辆类型[%s]对应的收费规则[%s] line[%d]", __FUNCTION__, cartype, charge_rule, __LINE__);
                writelog(log_buf_);
            }
            bson_destroy(&result);
        }
    }
    bson_destroy(query);
    mongoc_cursor_destroy(cursor);

    if(!find_value){
        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 在chargerule表 没有找到车辆类型[%s]对应的收费规则 line[%d]", __FUNCTION__, cartype, __LINE__);
        writelog(log_buf_);
    }

    query = bson_new();
    cursor = mongoc_collection_find (mongodb_table_park, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);
        mongoc_cursor_destroy(cursor);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 查找park表失败, 车场信息不存在. line[%d]", __FUNCTION__, __LINE__);
        writelog(log_buf_);

        doc.Accept(writer);
        mogodbb_process_wx_udp_send(buffer.GetString(), host_server_ip, PORT_UDP_BCENTER_TO_ONCALLCLIENT);

        return -1;
    }

    if(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor)) //得到car表的每一条记录
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "park_id"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(parkid,0,256);
                memcpy(parkid,tmp,length);

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 在park表中找到park_id[%s] line[%d]", __FUNCTION__, parkid, __LINE__);
                writelog(log_buf_);
            }
            bson_destroy(&result);
        }
    }
    bson_destroy(query);
    mongoc_cursor_destroy(cursor);

    query = bson_new();
    cursor = mongoc_collection_find (mongodb_table_park_set, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);
        mongoc_cursor_destroy(cursor);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 查找park_set表失败, park_set记录为空! line[%d]", __FUNCTION__, __LINE__);
        writelog(log_buf_);

        doc.Accept(writer);
        mogodbb_process_wx_udp_send(buffer.GetString(), host_server_ip, PORT_UDP_BCENTER_TO_ONCALLCLIENT);

        return -1;
    }
    if(!mongoc_cursor_error(cursor,&error) && mongoc_cursor_more(cursor)) //得到car表的每一条记录
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "park_name"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memcpy(parkname,tmp,length);

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 在park_set表中找到park_name[%s] line[%d]", __FUNCTION__, parkname, __LINE__);
                writelog(log_buf_);
            }
            bson_destroy(&result);
        }
    }
    bson_destroy(query);
    mongoc_cursor_destroy(cursor);

    mongoc_collection_destroy(mongodb_table_car);
    mongoc_collection_destroy(mongodb_table_chargerule);
    mongoc_collection_destroy(mongodb_table_carinpark);
    mongoc_collection_destroy(mongodb_table_park);
    mongoc_collection_destroy(mongodb_table_park_set);

    snprintf(log_buf_, sizeof(log_buf_), "[%s] 开始计费 收费规则[%s] 入场时间[%s] 出场时间[%s] 车辆类型[%s] 车场id[%s] 车牌[%s] line[%d]",
             __FUNCTION__, charge_rule, in_time, _in_outime, cartype, parkid, plate, __LINE__);
    writelog(log_buf_);

    int fee = mongodb_cal_fee(charge_rule,in_time, out_time,cartype,parkid,plate);

    snprintf(log_buf_, sizeof(log_buf_), "[%s] 应收缴费[%d] line[%d]", __FUNCTION__, fee, __LINE__);
    writelog(log_buf_);

    int sum_money = 0;

    query = bson_new();
    query_tmp = bson_new();

    BSON_APPEND_UTF8(query, "plate",plate);

    BSON_APPEND_UTF8(query_tmp, "$gte",in_time);
    BSON_APPEND_DOCUMENT(query, "time",query_tmp);
    //query = BCON_NEW ("$gte", "{", "caroutrec_out_time", BCON_UTF8 (in_time),"}");

    // 查询是否已经场内支付过
    cursor = mongoc_collection_find (mongodb_table_wx_zfb, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);
        mongoc_cursor_destroy(cursor);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] warn! 查询wx_zfb表 查询场内支付记录为空, 未曾缴费. line[%d]", __FUNCTION__, __LINE__);
        writelog(log_buf_);

        doc.Accept(writer);
        mogodbb_process_wx_udp_send(buffer.GetString(), host_server_ip, PORT_UDP_BCENTER_TO_ONCALLCLIENT);

        return -1;
    }

    int jiaofei_counts = 1;
    while(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor)) //得到car表的每一条记录
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "sum_money"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(tmp2,0,24);
                memcpy(tmp2,tmp,length);

                sum_money = sum_money + atoi(tmp2);

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 查询wx_zfb表 查询场内第[%d]次支付为[%s]元 累积[%d]元 查询条件 plate[%s] in_time[%s] line[%d]",
                         __FUNCTION__, jiaofei_counts++, tmp2, sum_money, plate, in_time, __LINE__);
                writelog(log_buf_);
            }
            bson_destroy(&result);
        }
    }

    bson_destroy(query);
    bson_destroy(query_tmp);
    mongoc_cursor_destroy(cursor);
    mongoc_collection_destroy(mongodb_table_wx_zfb);
    //  mongoc_client_destroy(mongodb_client);

    snprintf(log_buf_, sizeof(log_buf_), "[%s] 应收缴费[%d]元, 手机端已支付[%d]元 line[%d]", __FUNCTION__, fee, sum_money, __LINE__);
    writelog(log_buf_);

    fee = fee - sum_money;

    if(fee < 0)fee = 0;

    int duration = (get_tick(out_time) - get_tick(in_time))/60;

    snprintf(log_buf_, sizeof(log_buf_), "[%s] 停车[%d]分钟 入场时间[%s] 出场时间[%s] line[%d]", __FUNCTION__, duration, in_time, out_time, __LINE__);
    writelog(log_buf_);

    char du[24] = {0};
    snprintf(du, sizeof(du), "%d",duration);

    char mon[24] = {0};
    snprintf(mon, sizeof(mon), "%d", fee);

    value.SetString(StringRef(parkname));
    doc.AddMember("park_name", value, doc.GetAllocator());
    value.SetString(StringRef(cartype));
    doc.AddMember("cartype", value, doc.GetAllocator());
    value.SetString(StringRef(in_time));
    doc.AddMember("intime", value, doc.GetAllocator());
    //value.SetString(StringRef(du));
    //doc.AddMember("duration", value, doc.GetAllocator());
    Value::MemberIterator mon_value = doc.FindMember("money");
    mon_value->value.SetString(StringRef(mon));
    Value::MemberIterator ret = doc.FindMember("ret");
    ret->value.SetString(StringRef("ok"));

    doc.Accept(writer);
    mogodbb_process_wx_udp_send(buffer.GetString(), host_server_ip, PORT_UDP_BCENTER_TO_ONCALLCLIENT);

    return 0;
}

/**
 * @brief :场内 查询支付费用   TCP
 * @param park_id
 * @param box_ip
 * @param plate
 * @param openid
 * @param userid
 * @param outime
 * @return
 */
int mongodb_process_wx_tcp_query_fee_in(const char* _in_cmd, const char *_in_park_id, const char *_in_plate, const char *_in_openid, const char* _in_userid, const char *_in_outime, int sock)
{
    snprintf(log_buf_, sizeof(log_buf_), "[%s] 场内查询费用 park_id[%s] plate[%s] openid[%s] userid[%s] outtime[%s] line[%d]",
             __FUNCTION__, _in_park_id, _in_plate, _in_openid, _in_userid, _in_outime, __LINE__);
    writelog(log_buf_);
    
    char time_now[64];
    time_t tm;
    time_printf(time(&tm),time_now);
    
    //  mongoc_client_t *mongodb_client;
    mongoc_collection_t *mongodb_table_car;    //car表
    mongoc_collection_t *mongodb_table_park;    //car表
    mongoc_collection_t *mongodb_table_carinpark;    //device表
    mongoc_collection_t *mongodb_table_chargerule;    //device表
    mongoc_collection_t *mongodb_table_park_set;    //device表
    mongoc_collection_t *mongodb_table_wx_zfb;    //device表
    //  mongodb_client = mongoc_client_new(str_con);
    
    mongodb_table_car = mongoc_client_get_collection(mongodb_client,"boondb","car");   //channel表
    mongodb_table_carinpark = mongoc_client_get_collection(mongodb_client,"boondb","carinpark");   //device表
    mongodb_table_chargerule = mongoc_client_get_collection(mongodb_client,"boondb","chargerule");   //device表
    mongodb_table_park = mongoc_client_get_collection(mongodb_client,"boondb","park");   //device表
    mongodb_table_park_set = mongoc_client_get_collection(mongodb_client,"boondb","park_set");   //device表
    mongodb_table_wx_zfb = mongoc_client_get_collection(mongodb_client,"boondb","wx_zfb");   //device表
    
    mongoc_cursor_t *cursor;
    
    bson_t *query;
    bson_t *query_tmp;
    bson_t result;
    bson_iter_t iter;
    const char *tmp;
    const bson_t *tmp1;
    bson_error_t error;
    unsigned int length;
    
    char cartype[24] = {0};
    char parkid[128] = {0};
    char parkname[128] = {0};
    char in_time[24] = {0};
    char out_time[24] = {0};
    char charge_rule[256] = {0};
    char plate[128] = {0};
    char tmp2[24] = {0};
    
    snprintf(out_time, sizeof(out_time), "%s", _in_outime);
    
    // 拼接标准的json包
    rapidjson::Document doc;
    doc.SetObject();
    Value value(rapidjson::kObjectType);
    
    value.SetString(StringRef(_in_cmd));
    doc.AddMember("cmd", value, doc.GetAllocator());
    value.SetString(StringRef(_in_park_id));
    doc.AddMember("park_id", value, doc.GetAllocator());
    value.SetString(StringRef(_in_plate));
    doc.AddMember("plate", value, doc.GetAllocator());
    value.SetString(StringRef(_in_openid));
    doc.AddMember("openid", value, doc.GetAllocator());
    value.SetString(StringRef(_in_userid));
    doc.AddMember("userid", value, doc.GetAllocator());
    doc.AddMember("money", "0", doc.GetAllocator());
    doc.AddMember("ret", "fail", doc.GetAllocator());
    
    rapidjson::StringBuffer buffer;//in rapidjson/stringbuffer.h
    rapidjson::Writer<StringBuffer> writer(buffer); //in rapidjson/writer.h
    
    // 车牌长度异常
    if(strlen(_in_plate) < 3)
    {
        // 场内支付，web必须传入车牌号
        //strncpy(plate, out_plate_last,strlen(out_plate_last));
        
        snprintf(log_buf_, sizeof(log_buf_), "[%s] error, 严重错误， 场内支付时，没有收到车牌号 车牌号长度len[%ld], 取车牌out_plate_last[%s] line[%d]", __FUNCTION__, strlen(_in_plate), plate, __LINE__);
        writelog(log_buf_);
        
        doc.Accept(writer);
        mogodbb_process_wx_udp_send(buffer.GetString(), host_server_ip, PORT_UDP_BCENTER_TO_ONCALLCLIENT);
        
        return -1;
    }
    
    strncpy(plate, _in_plate,strlen(_in_plate));
    
    // step1 根据车牌号在car表里面查询车辆类型, 查询出临时车，贵宾车，月租车，
    mongodb_query_cartype(plate, cartype);
    
    query = bson_new();
    
    // step2 查询在场表，
    BSON_APPEND_UTF8(query, "carinpark_plate_id", plate); //142342
    cursor = mongoc_collection_find (mongodb_table_carinpark, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);
        mongoc_cursor_destroy(cursor);
        
        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 没有在carinpark表找到车牌[%s]的在场记录, return line[%d]", __FUNCTION__, plate, __LINE__);
        writelog(log_buf_);
        
        doc.Accept(writer);
        mogodbb_process_wx_udp_send(buffer.GetString(), host_server_ip, PORT_UDP_BCENTER_TO_ONCALLCLIENT);
        
        return -1;
    }
    
    bool find_value = false;
    while(!mongoc_cursor_error(cursor, &error) && mongoc_cursor_more(cursor)) //得到car表的每一条记录
    {
        //snprintf(log_buf_, sizeof(log_buf_), "[%s] begin line[%d]", __FUNCTION__, __LINE__);
        //writelog(log_buf_);
        
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) && bson_iter_find (&iter, "carinpark_in_time"))
            {
                tmp = bson_iter_utf8(&iter, &length);
                
                memcpy(in_time,tmp,length);
                
                find_value = true;
                
                snprintf(log_buf_, sizeof(log_buf_), "[%s] 在carinpark表中找到车牌[%s]的入场时间[%s] line[%d]", __FUNCTION__, plate, in_time, __LINE__);
                writelog(log_buf_);
            }
            bson_destroy(&result);
        }
        
        //snprintf(log_buf_, sizeof(log_buf_), "[%s] end line[%d]", __FUNCTION__, __LINE__);
        //writelog(log_buf_);
    }
    bson_destroy(query);
    mongoc_cursor_destroy(cursor);
    
    if(!find_value){
        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 在carinpark表 没有找到车牌[%s]的入场时间 line[%d]", __FUNCTION__, plate, __LINE__);
        writelog(log_buf_);
        
        doc.Accept(writer);
        
        mogodbb_process_wx_udp_send(buffer.GetString(), host_server_ip, PORT_UDP_BCENTER_TO_ONCALLCLIENT);
        
        return -1;
    }
    
    query = bson_new();
    
    find_value = false;
    // 查询收费规则
    BSON_APPEND_UTF8(query, "chargerule_car_type", cartype);
    cursor = mongoc_collection_find (mongodb_table_chargerule, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);
        
        mongoc_cursor_destroy(cursor);
        
        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 没有在chargerule表中找车辆类型[%s]对应的收费规则, return line[%d]", __FUNCTION__, cartype, __LINE__);
        writelog(log_buf_);
        
        doc.Accept(writer);
        mogodbb_process_wx_udp_send(buffer.GetString(), host_server_ip, PORT_UDP_BCENTER_TO_ONCALLCLIENT);
        
        return -1;
    }
    if(!mongoc_cursor_error(cursor,&error) && mongoc_cursor_more(cursor)) //得到car表的每一条记录
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1, &result); //得到一条完整的记录
            if (bson_iter_init(&iter, &result) && bson_iter_find (&iter, "chargerule_name_id"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(charge_rule,0,256);
                memcpy(charge_rule,tmp,length);
                
                find_value = true;
                
                snprintf(log_buf_, sizeof(log_buf_), "[%s] 在chargerule表中找到车辆类型[%s]对应的收费规则[%s] line[%d]", __FUNCTION__, cartype, charge_rule, __LINE__);
                writelog(log_buf_);
            }
            bson_destroy(&result);
        }
    }
    bson_destroy(query);
    mongoc_cursor_destroy(cursor);
    
    if(!find_value){
        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 在chargerule表 没有找到车辆类型[%s]对应的收费规则 line[%d]", __FUNCTION__, cartype, __LINE__);
        writelog(log_buf_);
    }
    
    query = bson_new();
    cursor = mongoc_collection_find (mongodb_table_park, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);
        mongoc_cursor_destroy(cursor);
        
        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 查找park表失败, 车场信息不存在. line[%d]", __FUNCTION__, __LINE__);
        writelog(log_buf_);
        
        doc.Accept(writer);
        mogodbb_process_wx_udp_send(buffer.GetString(), host_server_ip, PORT_UDP_BCENTER_TO_ONCALLCLIENT);
        
        return -1;
    }
    
    if(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor)) //得到car表的每一条记录
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "park_id"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(parkid,0,256);
                memcpy(parkid,tmp,length);
                
                snprintf(log_buf_, sizeof(log_buf_), "[%s] 在park表中找到park_id[%s] line[%d]", __FUNCTION__, parkid, __LINE__);
                writelog(log_buf_);
            }
            bson_destroy(&result);
        }
    }
    bson_destroy(query);
    mongoc_cursor_destroy(cursor);
    
    query = bson_new();
    cursor = mongoc_collection_find (mongodb_table_park_set, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);
        mongoc_cursor_destroy(cursor);
        
        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 查找park_set表失败, park_set记录为空! line[%d]", __FUNCTION__, __LINE__);
        writelog(log_buf_);
        
        doc.Accept(writer);
        mogodbb_process_wx_udp_send(buffer.GetString(), host_server_ip, PORT_UDP_BCENTER_TO_ONCALLCLIENT);
        
        return -1;
    }
    if(!mongoc_cursor_error(cursor,&error) && mongoc_cursor_more(cursor)) //得到car表的每一条记录
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "park_name"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memcpy(parkname,tmp,length);
                
                snprintf(log_buf_, sizeof(log_buf_), "[%s] 在park_set表中找到park_name[%s] line[%d]", __FUNCTION__, parkname, __LINE__);
                writelog(log_buf_);
            }
            bson_destroy(&result);
        }
    }
    bson_destroy(query);
    mongoc_cursor_destroy(cursor);
    
    mongoc_collection_destroy(mongodb_table_car);
    mongoc_collection_destroy(mongodb_table_chargerule);
    mongoc_collection_destroy(mongodb_table_carinpark);
    mongoc_collection_destroy(mongodb_table_park);
    mongoc_collection_destroy(mongodb_table_park_set);
    
    snprintf(log_buf_, sizeof(log_buf_), "[%s] 开始计费 收费规则[%s] 入场时间[%s] 出场时间[%s] 车辆类型[%s] 车场id[%s] 车牌[%s] line[%d]",
             __FUNCTION__, charge_rule, in_time, _in_outime, cartype, parkid, plate, __LINE__);
    writelog(log_buf_);
    
    int fee = mongodb_cal_fee(charge_rule,in_time, out_time,cartype,parkid,plate);
    
    snprintf(log_buf_, sizeof(log_buf_), "[%s] 应收缴费[%d] line[%d]", __FUNCTION__, fee, __LINE__);
    writelog(log_buf_);
    
    int sum_money = 0;
    
    query = bson_new();
    query_tmp = bson_new();
    
    BSON_APPEND_UTF8(query, "plate",plate);
    
    BSON_APPEND_UTF8(query_tmp, "$gte",in_time);
    BSON_APPEND_DOCUMENT(query, "time",query_tmp);
    //query = BCON_NEW ("$gte", "{", "caroutrec_out_time", BCON_UTF8 (in_time),"}");
    
    // 查询是否已经场内支付过
    cursor = mongoc_collection_find (mongodb_table_wx_zfb, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);
        mongoc_cursor_destroy(cursor);
        
        snprintf(log_buf_, sizeof(log_buf_), "[%s] warn! 查询wx_zfb表 查询场内支付记录为空, 未曾缴费. line[%d]", __FUNCTION__, __LINE__);
        writelog(log_buf_);
        
        doc.Accept(writer);
        mogodbb_process_wx_udp_send(buffer.GetString(), host_server_ip, PORT_UDP_BCENTER_TO_ONCALLCLIENT);
        
        return -1;
    }
    
    int jiaofei_counts = 1;
    while(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor)) //得到car表的每一条记录
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "sum_money"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(tmp2,0,24);
                memcpy(tmp2,tmp,length);
                
                sum_money = sum_money + atoi(tmp2);
                
                snprintf(log_buf_, sizeof(log_buf_), "[%s] 查询wx_zfb表 查询场内第[%d]次支付为[%s]元 累积[%d]元 查询条件 plate[%s] in_time[%s] line[%d]",
                         __FUNCTION__, jiaofei_counts++, tmp2, sum_money, plate, in_time, __LINE__);
                writelog(log_buf_);
            }
            bson_destroy(&result);
        }
    }
    
    bson_destroy(query);
    bson_destroy(query_tmp);
    mongoc_cursor_destroy(cursor);
    mongoc_collection_destroy(mongodb_table_wx_zfb);
    //  mongoc_client_destroy(mongodb_client);
    
    snprintf(log_buf_, sizeof(log_buf_), "[%s] 应收缴费[%d]元, 手机端已支付[%d]元 line[%d]", __FUNCTION__, fee, sum_money, __LINE__);
    writelog(log_buf_);
    
    fee = fee - sum_money;
    
    if(fee < 0)fee = 0;
    
    int duration = (get_tick(out_time) - get_tick(in_time))/60;
    
    snprintf(log_buf_, sizeof(log_buf_), "[%s] 停车[%d]分钟 入场时间[%s] 出场时间[%s] line[%d]", __FUNCTION__, duration, in_time, out_time, __LINE__);
    writelog(log_buf_);
    
    char du[24] = {0};
    snprintf(du, sizeof(du), "%d",duration);
    
    char mon[24] = {0};
    snprintf(mon, sizeof(mon), "%d", fee);
    
    value.SetString(StringRef(parkname));
    doc.AddMember("park_name", value, doc.GetAllocator());
    value.SetString(StringRef(cartype));
    doc.AddMember("cartype", value, doc.GetAllocator());
    value.SetString(StringRef(in_time));
    doc.AddMember("intime", value, doc.GetAllocator());
    //value.SetString(StringRef(du));
    //doc.AddMember("duration", value, doc.GetAllocator());
    Value::MemberIterator mon_value = doc.FindMember("money");
    mon_value->value.SetString(StringRef(mon));
    Value::MemberIterator ret = doc.FindMember("ret");
    ret->value.SetString(StringRef("ok"));
    
    doc.Accept(writer);
    //mogodbb_process_wx_udp_send(buffer.GetString(), host_server_ip, PORT_UDP_BCENTER_TO_ONCALLCLIENT);
    mogodbb_process_wx_tcp_send(buffer.GetString(), sock);
    return 0;
}

/**
 * @brief: 微信支付成功后，需要放行,向bled发送抬杆消息，并语音播报
 * @return -1, 失败
 *          0, 成功
 */
int mongodb_process_wx_opendoor()
{
    char time_now[64];
    time_t tm;
    time_printf(time(&tm),time_now);   //获取当前时间

    Json::Value json_send;
    json_send["cmd"] = Json::Value("open_door");
    json_send["channel_id"] = Json::Value(out_channel);
    json_send["in_out"] = Json::Value("出口"); //"入口"　“出口”

    // 车队模式标记
    if(out_fleet == true)
    {
        json_send["flag"] = Json::Value("keep");
    }
    else
    {
        json_send["flag"] = Json::Value("once");
    }

    if(false == g_has_cheduimoshi_){
        json_send["flag"] = Json::Value("once");
    }

    std::string send = json_send.toStyledString();

    char ip[] = {"127.0.0.1"};

    int ret = mogodbb_process_wx_udp_send(send, ip, PORT_UDP_BCENTER_TO_BIPC);

    if(ret <= 0){
        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 发包失败 line[%d]", __FUNCTION__, __LINE__);
        writelog(log_buf_);
    }


    //add 0807
    //char time_now[64];
    //time_t tm;
    int n;
    time_printf(time(&tm),time_now);
    Json::Value json_send1;  //语音信息
    json_send1["cmd"] = Json::Value("sendvoice");
    json_send1["content"] = Json::Value("祝您一路顺风");
    json_send1["led_ip"] = Json::Value(out_led_ip);

    std::string send1 = json_send1.toStyledString();
    //char ip[] = {"127.0.0.1"};
    struct sockaddr_in addr1;
    int sock1;
    sock1=socket(AF_INET, SOCK_DGRAM, 0);
    addr1.sin_family = AF_INET;
    addr1.sin_port = htons(PORT_UDP_BCENTER_TO_BLED); // 5006 --> 5002
    addr1.sin_addr.s_addr = inet_addr(ip);

    n = sendto(sock1,send1.c_str(),send1.length(), 0, (struct sockaddr *)&addr1, sizeof(addr1)); //发送语音信息
    if(n < 0)
    {
        fprintf(fp_log,"%s##mongo_send_bled 语音信息发送失败\n",time_now);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 向bled发包失败 msg[%s] len[%ld] ip[%s] port[%d] line[%d]",
                 __FUNCTION__, send1.c_str(), send1.length(), ip, PORT_UDP_BCENTER_TO_BLED, __LINE__);
        writelog(log_buf_);
    }
    else
    {
        fprintf(fp_log,"%s##mongo_send_bled 语音信息发送成功:\n",time_now);
        fprintf(fp_log,"%s\n",send1.c_str());

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 向bled发出去一个消息 msg[%s] len[%ld] ip[%s] port[%d] line[%d]",
                 __FUNCTION__, send1.c_str(), send1.length(), ip, PORT_UDP_BCENTER_TO_BLED, __LINE__);
        writelog(log_buf_);
    }
    close(sock1);






    return ret;
}

/**
 * @brief:
 * @param _msg
 * @return
 */
int mogodbb_process_wx_sendto_bled(const std::string& _msg)
{
    if(_msg.empty() || _msg.length() <= 0){  // json包
        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 入参异常 msg[%s] line[%d]", __FUNCTION__, _msg.c_str(), __LINE__);
        writelog(log_buf_);
    }

    char ip[] = {"127.0.0.1"};

    struct sockaddr_in addr;
    int sock;
    sock=socket(AF_INET, SOCK_DGRAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT_UDP_BCENTER_TO_BLED);
    addr.sin_addr.s_addr = inet_addr(ip);

    int ret = sendto(sock, _msg.c_str(), _msg.length(), 0, (struct sockaddr *)&addr, sizeof(addr));
    if(ret < 0)
    {
        snprintf(log_buf_, sizeof(log_buf_), "[%s] erorr! 发包[%s]失败{需检查网络连接状态} ip[%s] port[%d] line[%d]", __FUNCTION__, _msg.c_str(), ip, PORT_UDP_BCENTER_TO_BLED, __LINE__);
        writelog(log_buf_);
    }
    else
    {
        snprintf(log_buf_, sizeof(log_buf_), "[%s] 发出去一个消息包[%s] len[%ld] ip[%s] port[%d] line[%d]", __FUNCTION__, _msg.c_str(), _msg.length(), ip, PORT_UDP_BCENTER_TO_BLED, __LINE__);
        writelog(log_buf_);
    }
    close(sock);

    return ret;
}

int mogodbb_process_wx_tcp_send(const std::string& _msg, int sock) // tcp消息
{
    int n = write(sock, _msg.c_str(), _msg.length());
    // 将发送的消息内容写入日志
    write_log(_msg.c_str());
}

/**
 *
 * @param _msg
 * @param _port
 * @return
 */
int mogodbb_process_wx_udp_send(const std::string& _msg, const std::string& _ip, const int& _port)
{
    struct sockaddr_in addr;
    int sock;
    sock=socket(AF_INET, SOCK_DGRAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(_port);
    addr.sin_addr.s_addr = inet_addr(_ip.c_str());

    int n = sendto(sock, _msg.c_str(), _msg.length(), 0, (struct sockaddr *)&addr, sizeof(addr));
    if(n < 0)
    {
        snprintf(log_buf_, sizeof(log_buf_), "[%s] erorr! 发包[%s]失败{需检查网络连接状态} len[%ld] ip[%s] port[%d] line[%d]", __FUNCTION__, _msg.c_str(), _msg.length(), _ip.c_str(), _port, __LINE__);
        writelog(log_buf_);
    }
    else
    {
        snprintf(log_buf_, sizeof(log_buf_), "[%s] 发出去一个消息包[%s] len[%ld] ip[%s] port[%d] line[%d]", __FUNCTION__, _msg.c_str(), _msg.length(), _ip.c_str(), _port, __LINE__);
        writelog(log_buf_);
    }
    close(sock);

/*
//add 0807
char time_now[64];
    time_t tm;
time_printf(time(&tm),time_now);
Json::Value json_send1;  //语音信息
    json_send1["cmd"] = Json::Value("sendvoice");
    json_send1["content"] = Json::Value("祝您一路顺风");
    json_send1["led_ip"] = Json::Value(out_led_ip);

    std::string send1 = json_send1.toStyledString();
    char ip[] = {"127.0.0.1"};
    struct sockaddr_in addr1;
    int sock1;
    sock1=socket(AF_INET, SOCK_DGRAM, 0);
    addr1.sin_family = AF_INET;
    addr1.sin_port = htons(PORT_UDP_BCENTER_TO_BLED); // 5006 --> 5002
    addr1.sin_addr.s_addr = inet_addr(ip);

    n = sendto(sock1,send1.c_str(),send1.length(), 0, (struct sockaddr *)&addr1, sizeof(addr1)); //发送语音信息
    if(n < 0)
    {
        fprintf(fp_log,"%s##mongo_send_bled 语音信息发送失败\n",time_now);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 向bled发包失败 msg[%s] len[%ld] ip[%s] port[%d] line[%d]",
                 __FUNCTION__, send1.c_str(), send1.length(), ip, PORT_UDP_BCENTER_TO_BLED, __LINE__);
        writelog(log_buf_);
    }
    else
    {
        fprintf(fp_log,"%s##mongo_send_bled 语音信息发送成功:\n",time_now);
        fprintf(fp_log,"%s\n",send1.c_str());

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 向bled发出去一个消息 msg[%s] len[%ld] ip[%s] port[%d] line[%d]",
                 __FUNCTION__, send1.c_str(), send1.length(), ip, PORT_UDP_BCENTER_TO_BLED, __LINE__);
        writelog(log_buf_);
    }
    close(sock1);
*/
    return n;
}

/**
 * @brief: 添加 char* userid 参数， diaoguangqiang, 20171124 1531
 */
int mongodb_process_wx_carout(char *name,char *park_id,char *box_ip,char *plate1, char *openid, char *outime, char* userid)
{
    char time_now[64];
    time_t tm;
    time_printf(time(&tm),time_now);

    //  mongoc_client_t *mongodb_client;
    mongoc_collection_t *mongodb_table_car;	//car表
    mongoc_collection_t *mongodb_table_park;	//car表
    mongoc_collection_t *mongodb_table_carinpark;	//
    mongoc_collection_t *mongodb_table_chargerule;	//
    mongoc_collection_t *mongodb_table_park_set;	//
    mongoc_collection_t *mongodb_table_wx_zfb;	//
    //  mongodb_client = mongoc_client_new(str_con);

    mongodb_table_car = mongoc_client_get_collection(mongodb_client,"boondb","car");   //channel表
    mongodb_table_carinpark = mongoc_client_get_collection(mongodb_client,"boondb","carinpark");   //device表
    mongodb_table_chargerule = mongoc_client_get_collection(mongodb_client,"boondb","chargerule");   //device表
    mongodb_table_park = mongoc_client_get_collection(mongodb_client,"boondb","park");   //device表
    mongodb_table_park_set = mongoc_client_get_collection(mongodb_client,"boondb","park_set");   //device表
    mongodb_table_wx_zfb = mongoc_client_get_collection(mongodb_client,"boondb","wx_zfb");   //device表

    mongoc_cursor_t *cursor;

    bson_t *query;
    bson_t *query_tmp;
    bson_t result;
    bson_iter_t iter;
    const char *tmp;
    const bson_t *tmp1;
    bson_error_t error;
    unsigned int length;

    char cartype[24];
    char parkid[128];
    char parkname[128];
    char in_time[24] = {0};
    char charge_rule[256];
    memset(charge_rule,0,256);
    memset(in_time,0,24);
    memset(cartype,0,24);
    char plate[128];
    char tmp2[24];
    memset(plate,0,128);
    memset(parkname,0,128);
    memset(parkid,0,128);

    // 车牌长度异常
    if(strlen(plate1) < 3)
    {
        //fprintf(fp_log,"%s##%s 失败 line[%d]\n",time_now, __FUNCTION__, __LINE__);

        strncpy(plate, out_plate_last,strlen(out_plate_last));

        snprintf(log_buf_, sizeof(log_buf_), "[%s] error, 车牌号长度异常len[%ld], 取车牌out_plate_last[%s] line[%d]", __FUNCTION__, strlen(plate1), plate, __LINE__);
        writelog(log_buf_);
    }
    else
    {
        strncpy(plate,plate1,strlen(plate1));

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 收到车牌 plate[%s] line[%d]", __FUNCTION__, plate, __LINE__);
        writelog(log_buf_);
    }

    // step1 根据车牌号在car表里面查询车辆类型, 查询出临时车，贵宾车，月租车，
    mongodb_query_cartype(plate, cartype);

    query = bson_new();

    // step2 查询在场表，
    BSON_APPEND_UTF8(query, "carinpark_plate_id",plate);

    cursor = mongoc_collection_find (mongodb_table_carinpark, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);
        mongoc_cursor_destroy(cursor);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 没有在carinpark表找到车牌[%s]的在场记录, return line[%d]", __FUNCTION__, plate, __LINE__);
        writelog(log_buf_);

        return -1;
    }

    while(!mongoc_cursor_error(cursor, &error) && mongoc_cursor_more(cursor)) //得到car表的每一条记录
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) && bson_iter_find (&iter, "carinpark_in_time"))
            {
                tmp = bson_iter_utf8(&iter, &length);

                memcpy(in_time,tmp,length);

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 在carinpark表中找到车牌[%s]的入场时间[%s] line[%d]", __FUNCTION__, plate, in_time, __LINE__);
                writelog(log_buf_);
            }
            bson_destroy(&result);
        }
    }
    bson_destroy(query);
    mongoc_cursor_destroy(cursor);

    query = bson_new();

    // 查询收费规则
    BSON_APPEND_UTF8(query, "chargerule_car_type", cartype);
    cursor = mongoc_collection_find (mongodb_table_chargerule, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);

        fprintf(fp_log,"%s##mongodb_process_wx_carout 失败\n",time_now);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 没有在chargerule表中找车辆类型[%s]对应的收费规则, return line[%d]", __FUNCTION__, cartype, __LINE__);
        writelog(log_buf_);

        return -1;
    }

    if(!mongoc_cursor_error(cursor,&error) && mongoc_cursor_more(cursor)) //得到car表的每一条记录
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1, &result); //得到一条完整的记录
            if (bson_iter_init(&iter, &result) && bson_iter_find (&iter, "chargerule_name_id"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(charge_rule,0,256);
                memcpy(charge_rule,tmp,length);

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 在chargerule表中找到车辆类型[%s]对应的收费规则[%s] line[%d]", __FUNCTION__, cartype, charge_rule, __LINE__);
                writelog(log_buf_);
            }
            bson_destroy(&result);
        }
    }
    bson_destroy(query);

    mongoc_cursor_destroy(cursor);

    query = bson_new();
    cursor = mongoc_collection_find (mongodb_table_park, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);

        fprintf(fp_log,"%s##mongodb_process_wx_carout 失败\n",time_now);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 查找park表失败, 车场信息不存在. line[%d]", __FUNCTION__, __LINE__);
        writelog(log_buf_);

        return -1;
    }

    if(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor)) //得到car表的每一条记录
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "park_id"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(parkid,0,256);
                memcpy(parkid,tmp,length);

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 在park表中找到park_id[%s] line[%d]", __FUNCTION__, parkid, __LINE__);
                writelog(log_buf_);
            }
            bson_destroy(&result);
        }
    }
    bson_destroy(query);
    mongoc_cursor_destroy(cursor);

    query = bson_new();
    cursor = mongoc_collection_find (mongodb_table_park_set, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);

        fprintf(fp_log,"%s##mongodb_process_wx_carout 失败\n",time_now);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 查找park_set表失败, park_set记录为空! line[%d]", __FUNCTION__, __LINE__);
        writelog(log_buf_);

        return -1;
    }
    if(!mongoc_cursor_error(cursor,&error) && mongoc_cursor_more(cursor)) //得到car表的每一条记录
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "park_name"))
            {
                tmp = bson_iter_utf8(&iter,&length);
                memcpy(parkname,tmp,length);

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 在park_set表中找到park_name[%s] line[%d]", __FUNCTION__, parkname, __LINE__);
                writelog(log_buf_);
            }
            bson_destroy(&result);
        }
    }
    bson_destroy(query);
    mongoc_cursor_destroy(cursor);

    mongoc_collection_destroy(mongodb_table_car);
    mongoc_collection_destroy(mongodb_table_chargerule);
    mongoc_collection_destroy(mongodb_table_carinpark);
    mongoc_collection_destroy(mongodb_table_park);
    mongoc_collection_destroy(mongodb_table_park_set);

    snprintf(log_buf_, sizeof(log_buf_), "[%s] 开始计费 收费规则[%s] 入场时间[%s] 出场时间[%s] 车辆类型[%s] 车场id[%s] 车牌[%s] line[%d]",
             __FUNCTION__, charge_rule, in_time, outime, cartype, parkid, plate, __LINE__);
    writelog(log_buf_);

    fprintf(fp_log,"%s##开始计费 %s %s %s %s %s %s  \n",time_now,charge_rule,in_time,outime,cartype,parkid,plate);

    // 开始计费
    int fee = mongodb_cal_fee(charge_rule,in_time,outime,cartype,parkid,plate);

    fprintf(fp_log,"%s##应缴费%d元\n",time_now,fee);

    snprintf(log_buf_, sizeof(log_buf_), "[%s] 应收金额[%d] line[%d]", __FUNCTION__, fee, __LINE__);
    writelog(log_buf_);

    int sum_money = 0;
    query = bson_new();

    query_tmp = bson_new();

    BSON_APPEND_UTF8(query, "plate",plate); // 车牌号
    BSON_APPEND_UTF8(query_tmp, "$gte",in_time);
    BSON_APPEND_DOCUMENT(query, "time",query_tmp); // 支付时间
    //query = BCON_NEW ("$gte", "{", "caroutrec_out_time", BCON_UTF8 (in_time),"}");

    // 查询是否已经场内支付过
    cursor = mongoc_collection_find (mongodb_table_wx_zfb, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL); //查car表
    if(mongoc_cursor_error(cursor,&error))
    {
        bson_destroy(query);

        mongoc_cursor_destroy(cursor);

        fprintf(fp_log,"%s##mongodb_process_fee失败\n",time_now);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] warn! 查询场内支付记录为空, 未曾缴费. line[%d]", __FUNCTION__, __LINE__);
        writelog(log_buf_);

        return -1;
    }

    int jiaofei_counts = 1;
    while(!mongoc_cursor_error(cursor,&error)&&mongoc_cursor_more(cursor)) //得到car表的每一条记录
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {

            bson_copy_to(tmp1,&result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) &&bson_iter_find (&iter, "sum_money")) // 总金额
            {
                tmp = bson_iter_utf8(&iter,&length);
                memset(tmp2,0,24);
                memcpy(tmp2,tmp,length);

                sum_money = sum_money + atoi(tmp2);

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 查询wx_zfb表 查询场内第[%d]次支付为[%s]元 累积[%d]元 查询条件 plate[%s] in_time[%s] line[%d]",
                         __FUNCTION__, jiaofei_counts++, tmp2, sum_money, plate, in_time, __LINE__);
                writelog(log_buf_);
            }
            bson_destroy(&result);
        }
    }

    bson_destroy(query);
    bson_destroy(query_tmp);
    mongoc_cursor_destroy(cursor);
    mongoc_collection_destroy(mongodb_table_wx_zfb);

    //  mongoc_client_destroy(mongodb_client);
    fprintf(fp_log,"%s##应缴费%d元,微信已缴费%d元 line[%d]\n",time_now,fee, sum_money, __LINE__);

    snprintf(log_buf_, sizeof(log_buf_), "[%s] 折算后 总需缴费[%d]元, 场内微信已支付[%d]元, 最后应收[%d]元 line[%d]", __FUNCTION__, fee, sum_money, fee-sum_money, __LINE__);
    writelog(log_buf_);

    fee = fee - sum_money;

    if(fee < 0)fee = 0;

    // 拼接标准的json包
    rapidjson::Document doc;
    doc.SetObject();

    Value value(rapidjson::kObjectType);

    doc.AddMember("cmd", C_CMD_OUT, doc.GetAllocator());

    value.SetString(StringRef(park_id));
    doc.AddMember("park_id", value, doc.GetAllocator());

    value.SetString(StringRef(parkname));
    doc.AddMember("park_name", value, doc.GetAllocator());

    value.SetString(StringRef(plate));
    doc.AddMember("plate", value, doc.GetAllocator());

    value.SetString(StringRef(openid));
    doc.AddMember("openid", value, doc.GetAllocator());

    value.SetString(StringRef(userid));
    doc.AddMember("userid", value, doc.GetAllocator());

    value.SetString(StringRef(cartype));
    doc.AddMember("cartype", value, doc.GetAllocator());

    value.SetString(StringRef(in_time));
    doc.AddMember("intime", value, doc.GetAllocator());

    int duration = (get_tick(outime) - get_tick(in_time))/60;

    snprintf(log_buf_, sizeof(log_buf_), "[%s] 停车[%d]分钟 入场时间[%s] 出场时间[%s] line[%d]", __FUNCTION__, duration, in_time, outime, __LINE__);
    writelog(log_buf_);

    char du[24];
    memset(du,0,24);
    sprintf(du,"%d",duration);

    value.SetString(StringRef(du));
    doc.AddMember("duration", value, doc.GetAllocator());

    //json_msg["duration"] = Json::Value(du);

    char mon[24];
    memset(mon,0,24);
    sprintf(mon,"%d",fee);

    value.SetString(StringRef(mon));
    doc.AddMember("money", value, doc.GetAllocator());

    //json_msg["money"] = Json::Value(mon);

    rapidjson::StringBuffer buffer;//in rapidjson/stringbuffer.h
    rapidjson::Writer<StringBuffer> writer(buffer); //in rapidjson/writer.h
    doc.Accept(writer);

    std::string send = buffer.GetString(); //json_msg.toStyledString();

    struct sockaddr_in addr;
    int sock;
    sock=socket(AF_INET, SOCK_DGRAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT_UDP_BCENTER_TO_ONCALLCLIENT);
    addr.sin_addr.s_addr = inet_addr(host_server_ip);
    int n = sendto(sock,send.c_str(),send.length(), 0, (struct sockaddr *)&addr, sizeof(addr));
    if(n < 0)
    {
        fprintf(fp_log,"%s##wx carout msg发送失败\n",time_now);

        snprintf(log_buf_, sizeof(log_buf_), "[%s] erorr! 向oncallclient发包[%s]失败{需检查网络连接状态} ip[%s] port[%d] line[%d]", __FUNCTION__, send.c_str(), host_server_ip, PORT_UDP_BCENTER_TO_ONCALLCLIENT, __LINE__);
        writelog(log_buf_);
    }
    else
    {
        fprintf(fp_log,"%s##wx carout msg发送成功:\n",time_now);
        fprintf(fp_log,"%s\n",send.c_str());

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 向oncallclient发出去一个消息包[%s] len[%ld] ip[%s] port[%d] line[%d]", __FUNCTION__, send.c_str(), send.length(), host_server_ip, PORT_UDP_BCENTER_TO_ONCALLCLIENT, __LINE__);
        writelog(log_buf_);
    }
    close(sock);

    return 0;
}

/**
 * @brief: 支付接口，接收pay消息
 * @param money
 * @param park_id
 * @param box_ip
 * @param plate
 * @param openid
 * @param flag
 * @return -1， 失败
 *          0, 成功
 */
int mongodb_process_wx_pay( char *money,char *park_id,char * box_ip,char *plate,char *openid, char *flag, char* userid)
{
    char time_now[64];
    time_t tm;
    time_printf(time(&tm),time_now);

    //  mongoc_client_t *mongodb_client;
    mongoc_collection_t *mongodb_table_wx;	//wx表
    //  mongodb_client = mongoc_client_new(str_con);
    mongodb_table_wx = mongoc_client_get_collection(mongodb_client,"boondb","wx");   //wx表

    mongoc_cursor_t *cursor;

    bson_t *query;
    bson_t result;
    bson_iter_t iter;
    const char *tmp;
    const bson_t *tmp1;
    bson_error_t error;
    unsigned int length;

    Json::Value json_msg; //发送给bled的json格式的信息
    query = bson_new();
    BSON_APPEND_UTF8(query, "wx_money",money);
    BSON_APPEND_UTF8(query, "wx_park_id",park_id);
    BSON_APPEND_UTF8(query, "wx_box_ip",box_ip);
    BSON_APPEND_UTF8(query, "wx_plate",plate);
    BSON_APPEND_UTF8(query, "wx_openid",openid);
    BSON_APPEND_UTF8(query, "wx_flag",flag);
    BSON_APPEND_UTF8(query, "wx_time",time_now);

    if(!mongoc_collection_insert (mongodb_table_wx, MONGOC_INSERT_NONE, query, NULL, &error))
    {
        fprintf(fp_log,"%s##mongodb_process_wx_pay 失败\n",time_now);
        bson_destroy(query);
        return -1;
    }
    bson_destroy(query);
    mongoc_collection_destroy(mongodb_table_wx);
    //  mongoc_client_destroy(mongodb_client);

    json_msg["cmd"] = Json::Value("pay");
    json_msg["park_id"] = Json::Value(park_id);
    json_msg["text"] = Json::Value("成功");
    json_msg["plate"] = Json::Value(plate); // 车牌号
    json_msg["channel_id"] = Json::Value(out_channel);// 通道id

    std::string send = json_msg.toStyledString();

    struct sockaddr_in addr;
    int sock;
    sock=socket(AF_INET, SOCK_DGRAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(6002);
    addr.sin_addr.s_addr = inet_addr(host_server_ip);
    int n = sendto(sock,send.c_str(),send.length(), 0, (struct sockaddr *)&addr, sizeof(addr));
    if(n < 0)
    {
        fprintf(fp_log,"%s##mongodb_process_wx_pay发送失败\n",time_now);
    }
    else
    {
        fprintf(fp_log,"%s##mongodb_process_wx_pay发送成功:\n",time_now);
        fprintf(fp_log,"%s\n",send.c_str());
    }
    close(sock);


    std::string recv_flag = flag;
    if(0 == recv_flag.compare("open")){ // 如果是open，说明是场外支付，即需要出场,此时需要发送太杆信号

        // 向bled发送抬杆信号
        mongodb_process_wx_fangxing();

        return 0;
    }
}

/**
 * @brief: 微信支付成功后，需要放行,向bled发送抬杆消息，并语音播报
 * @return -1, 失败
 *          0, 成功
 */
int mongodb_process_wx_fangxing()
{
    char time_now[64];
    time_t tm;
    time_printf(time(&tm),time_now);   //获取当前时间

    Json::Value json_send;
    json_send["cmd"] = Json::Value("open_door");
    json_send["channel_id"] = Json::Value(out_channel);
    json_send["in_out"] = Json::Value("出口"); //"入口"　“出口”

    // 车队模式标记
    if(out_fleet == true)
    {
        json_send["flag"] = Json::Value("keep");
    }
    else
    {
        json_send["flag"] = Json::Value("once");
    }

    std::string send = json_send.toStyledString();

    struct sockaddr_in addr;
    int sock;
    sock=socket(AF_INET, SOCK_DGRAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(5002);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    int n = sendto(sock,send.c_str(),send.length(), 0, (struct sockaddr *)&addr, sizeof(addr));
    if(n < 0)
    {
        fprintf(fp_log,"%s##send_bipc_open_door 发送失败\n",time_now);
    }
    else
    {
        fprintf(fp_log,"%s##send_bipc_open_door 发送成功:\n",time_now);
        fprintf(fp_log,"%s\n",send.c_str());
    }
    close(sock);

    return 0;
}


/**
 * @brief: 扫码枪的数据入库
 * @return
 */
bool mongodb_bgui_smq_insert(Json::Value& _in_json_value)
{
    std::string cmd = _in_json_value["cmd"].asString();
    if(cmd.empty() || cmd.length() <=0){
        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 命令类型异常[%s] return false; line[%d]", __FUNCTION__, cmd.c_str(), __LINE__);
        writelog(log_buf_);
        return false;
    }

    std::string plate = _in_json_value["plateid"].asString();
    std::string sum_money = _in_json_value["money"].asString();

    if(sum_money.compare("0") == 0){
        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 收到金额异常[%s] return false; line[%d]", __FUNCTION__, sum_money.c_str(), __LINE__);
        writelog(log_buf_);
        return false;
    }

    std::string park_id = _in_json_value["parkid"].asString();
    std::string id = _in_json_value["id"].asString();
    std::string time = _in_json_value["time"].asString();
    std::string flag = _in_json_value["flag"].asString();

    mongoc_collection_t *mongodb_table_wx_zfb = mongoc_client_get_collection(mongodb_client,"boondb","wx_zfb");   //wx表

    bson_t *query;
    bson_iter_t iter;
    bson_error_t error;

    query = bson_new();

    BSON_APPEND_UTF8(query, "plate", plate.c_str());
    BSON_APPEND_UTF8(query, "sum_money", sum_money.c_str());
    BSON_APPEND_UTF8(query, "park_id", park_id.c_str());
    BSON_APPEND_UTF8(query, "id", id.c_str()); // openid / userid
    BSON_APPEND_UTF8(query, "time", time.c_str()); //
    BSON_APPEND_UTF8(query, "cmd", flag.c_str()); // 微信刷卡或支付宝刷卡

    bool insert_ret = mongoc_collection_insert (mongodb_table_wx_zfb, MONGOC_INSERT_NONE, query, NULL, &error);

    bson_destroy(query);
    mongoc_collection_destroy(mongodb_table_wx_zfb);

    // step1 写入数据库
    if(!insert_ret)
    {
        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 插入wx_zfb表失败 line[%d]", __FUNCTION__, __LINE__);
        writelog(log_buf_);
    }else{
        snprintf(log_buf_, sizeof(log_buf_), "[%s] 写入wx_zfb表成功 plate[%s] sum_money[%s] park_id[%s] id[%s] time[%s] cmd[%s] line[%d]",
                 __FUNCTION__, plate.c_str(), sum_money.c_str(), park_id.c_str(), id.c_str(), time.c_str(), flag.c_str(), __LINE__);
        writelog(log_buf_);
    }

    return insert_ret;
}

/**
 * @brief: 扫码枪开闸
 * @return
 */
int mongodb_bgui_smq_opendoor()
{
    char time_now[64];
    time_t tm;
    time_printf(time(&tm),time_now);   //获取当前时间

    Json::Value json_send;
    json_send["cmd"] = Json::Value("open_door");
    json_send["channel_id"] = Json::Value(out_channel);
    json_send["in_out"] = Json::Value("出口"); //"入口"　“出口”

    // 车队模式标记
    if(out_fleet == true)
    {
        json_send["flag"] = Json::Value("keep");
    }
    else
    {
        json_send["flag"] = Json::Value("once");
    }

    if(false == g_has_cheduimoshi_){
        json_send["flag"] = Json::Value("once");
    }

    std::string send = json_send.toStyledString();

    char ip[] = "127.0.0.1";

    int ret = mogodbb_process_udp_send(send, ip, PORT_UDP_BCENTER_TO_BIPC);

    if(ret <= 0){
        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 发包失败 line[%d]", __FUNCTION__, __LINE__);
        writelog(log_buf_);
    }

    return ret;
}

/**
 * @brief : 查询　carinrec表，获取车牌最后一条记录，获取时间分钟，判断持续分钟
 * @param _out_dur_min : 输出持续的分钟数
 * @param _in_plate : 当前进入时间
 * @param _in_current_time
 * @return : false, 没有找记录
 *          true, 找到车牌
 * @attention: 第一次读取carinrec时候，应该是没有数据的；所以查询carinrec应该在insert carinrec前面　读取
 *             第二次读取carinrec时候，也应该在 insert carinrec前面 读取
 */
bool mongodb_jici_query_carinrec_duration_min(int& _out_dur_min, const char* _in_plate, const char* _in_current_time)
{
    if(!_in_current_time || !_in_plate) {
        snprintf(log_buf_, sizeof(log_buf_), "[%s] 传入参数异常, return false! line[%d]", __FUNCTION__, __LINE__);
        writelog(log_buf_);

        return false;
    }

    mongoc_collection_t *mongodb_table_carinrec = mongoc_client_get_collection(mongodb_client,"boondb","carinrec");

    mongoc_cursor_t *cursor;
    bson_t *query;
    bson_t result;
    bson_iter_t iter;
    bson_error_t error;

    const char *tmp;
    const bson_t *tmp1;
    unsigned int length;

    query = bson_new();

    BSON_APPEND_UTF8(query, "carinrec_plate_id", _in_plate);

    cursor = mongoc_collection_find (mongodb_table_carinrec, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
    if(mongoc_cursor_error(cursor, &error))
    {
        snprintf(log_buf_, sizeof(log_buf_), "[%s] error 查询mongdb失败 没有找到 plate[%s]{可能首次进入该停车场}  line[%d]", __FUNCTION__, _in_plate, __LINE__);
        writelog(log_buf_);

        bson_destroy(query);
        mongoc_cursor_destroy(cursor);

        return false;
    }

    bool find = false;

    char last_in_time[24] = {0};

    // 统计该车的进入次数
    int tmp_count = 0;

    while(!mongoc_cursor_error(cursor,&error) && mongoc_cursor_more(cursor))
    {
        if (mongoc_cursor_next (cursor, &tmp1))
        {
            bson_copy_to(tmp1, &result); //得到一条完整的记录
            if (bson_iter_init (&iter, &result) && bson_iter_find (&iter, "carinrec_in_time"))
            {
                find = true;
                tmp = bson_iter_utf8(&iter,&length);
                memcpy(last_in_time, tmp, length);

                tmp_count++;

                //snprintf(log_buf_, sizeof(log_buf_), "[%s] 找到车牌[%s]的入场时间[%s] line[%d]", __FUNCTION__, _in_plate, last_in_time, __LINE__);
                //writelog(log_buf_);
            }
            bson_destroy(&result);
        }
    }

    bson_destroy(query);
    mongoc_cursor_destroy(cursor);
    mongoc_collection_destroy(mongodb_table_carinrec);

    if(!find) {
        snprintf(log_buf_, sizeof(log_buf_), "[%s] error 查询carinrec_in_time失败 没有找到 plate[%s]的入场时间{可能首次进入该停车场}  line[%d]", __FUNCTION__, _in_plate, __LINE__);
        writelog(log_buf_);

        return false;
    }else{
        snprintf(log_buf_, sizeof(log_buf_), "[%s] 找到车牌[%s]的入场次数[%d]次 最近一次进入时间[%s] line[%d]", __FUNCTION__, _in_plate, tmp_count, last_in_time, __LINE__);
        writelog(log_buf_);
    }

    // 如果有记录，则统计持续分钟
    char current_time[24] = {0};
    memcpy(current_time, _in_current_time, sizeof(current_time));

    // 返回持续的分钟数
    _out_dur_min = (get_tick(current_time) - get_tick(last_in_time))/60;

    snprintf(log_buf_, sizeof(log_buf_), "[%s] 查询carinrec表，找到plate[%s]上次入场信息 last_in_time[%s] current_in_time[%s] 持续[%d]分钟 line[%d]", __FUNCTION__, _in_plate, last_in_time, current_time, _out_dur_min, __LINE__);
    writelog(log_buf_);

    return true;
}

/**
 * @brief : 查询car表，　设置计次次数减１操作
 * @param _in_plate :　车牌号
 * @param _in_step ：　步长，默认１
 * @return ：　>=0, 剩余计数次数
 *             <0, 失败
 */
int mongodb_jici_update_carinrec_jici_counts(const char* _in_plate)
{
    mongoc_collection_t *mongodb_table_car = mongoc_client_get_collection(mongodb_client,"boondb","car");

    mongoc_cursor_t *cursor_find;
    bson_t *query_find;
    bson_t result_find;
    bson_iter_t iter_find;
    bson_error_t error;

    const char *tmp;
    const bson_t *tmp1;
    unsigned int length;

    query_find = bson_new();

    // step1 先查询当前车牌的剩余次数
    BSON_APPEND_UTF8(query_find, "car_plate_id", _in_plate);

    cursor_find = mongoc_collection_find (mongodb_table_car, MONGOC_QUERY_NONE, 0, 0, 0, query_find, NULL, NULL);
    if(mongoc_cursor_error(cursor_find, &error))
    {
        snprintf(log_buf_, sizeof(log_buf_), "[%s] error 查询mongdb失败 没有在car表找到 plate[%s] line[%d]", __FUNCTION__, _in_plate, __LINE__);
        writelog(log_buf_);

        bson_destroy(query_find);
        mongoc_cursor_destroy(cursor_find);

        return -1;
    }

    char remain_counts[24] = {0};
    bool find = false;

    if(!mongoc_cursor_error(cursor_find,&error)&&mongoc_cursor_more(cursor_find))
    {
        if (mongoc_cursor_next(cursor_find, &tmp1))
        {
            bson_copy_to(tmp1, &result_find); //得到一条完整的记录
            if (bson_iter_init (&iter_find, &result_find) &&bson_iter_find (&iter_find, "car_remain_counts"))
            {
                find = true;
                tmp = bson_iter_utf8(&iter_find,&length);
                memcpy(remain_counts,tmp,length);

                snprintf(log_buf_, sizeof(log_buf_), "[%s] 查找car表 找到车牌plate[%s]的剩余次数[%s] line[%d]", __FUNCTION__, _in_plate, remain_counts, __LINE__);
                writelog(log_buf_);
            }
            bson_destroy(&result_find);
        }
    }
    bson_destroy(query_find);
    mongoc_cursor_destroy(cursor_find);

    if(!find){
        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 查找car表 没有找到车牌plate[%s]的剩余次数 line[%d]", __FUNCTION__, _in_plate, __LINE__);
        writelog(log_buf_);

        return -1;
    }

    // step2 剩余次数　减１
    int icounts = atoi(remain_counts);
    if(icounts >= 1){
        icounts -= 1;

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 剩余次数为减为[%d]次 line[%d]", __FUNCTION__, icounts, __LINE__);
        writelog(log_buf_);
    }else{
        icounts = 0;

        snprintf(log_buf_, sizeof(log_buf_), "[%s] 剩余次数为0次 line[%d]", __FUNCTION__, __LINE__);
        writelog(log_buf_);
    }

    // step3 更新表
    memset(remain_counts, 0, sizeof(remain_counts));
    snprintf(remain_counts, sizeof(remain_counts), "%d", icounts);

    bson_t *query_update = bson_new();
    bson_t *result_update = bson_new();

    BSON_APPEND_UTF8(query_update, "car_plate_id", _in_plate); //查询条件
    result_update = BCON_NEW("$set", "{", "car_remain_counts", BCON_UTF8 (remain_counts), "}");
    bool ret = mongoc_collection_update(mongodb_table_car, MONGOC_UPDATE_NONE, query_update, result_update, NULL, &error);
    if(!ret){
        snprintf(log_buf_, sizeof(log_buf_), "[%s] error! 更新car表 plate[%s] 剩余次数为[%s]次失败 line[%d]", __FUNCTION__, _in_plate, remain_counts, __LINE__);
        writelog(log_buf_);
    }else{
        snprintf(log_buf_, sizeof(log_buf_), "[%s] 更新car表 plate[%s] 剩余次数为[%s]次成功 line[%d]", __FUNCTION__, _in_plate, remain_counts, __LINE__);
        writelog(log_buf_);
    }

    bson_destroy(query_update);
    bson_destroy(result_update);

    mongoc_collection_destroy(mongodb_table_car);

    return icounts;
}
