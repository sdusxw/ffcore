//
// Created by boon on 17-11-22.
//

#include "rapidjson_lib.h"
#include "boon_log.h"
#include "wx_def.h"
#include "mongodb_model.h"

using namespace BASE;
using namespace ONCALL;
using namespace mongocxx;


CMongoDbModel::CMongoDbModel()
        : port_(PORT_MONGODB)
{
    memset(ip_, 0, sizeof(ip_));
    memcpy(ip_, IP_MONGODB, sizeof(ip_));

    memset(log_buf_, 0, sizeof(log_buf_));

    init();
}

CMongoDbModel::~CMongoDbModel()
{

}

void CMongoDbModel::init()
{
    mongocxx::instance instance{};

    uri_str_ = MONGODB_URI;
}

/**
 * @brief: 查询停车场id
 * @param _vec : 容器
 * @return ： false, 查询失败， true, 查询成功
 */
bool CMongoDbModel::queryParkId(std::vector<string>& _vec)
{
    _vec.clear();

    bool ret = false;

    try {
        mongocxx::uri uri(uri_str_);
        mongocxx::client client(uri);
        mongocxx::database db = client.database(MONGODB_DB);

        auto cursor = db["park_set"].find({});

        for(auto&& it:cursor){

            std::string json = bsoncxx::to_json(it);

            Document doc;
            doc.Parse(json.c_str());

            if(doc.HasMember("park_ID") && doc["park_ID"] != "" ){
                _vec.push_back(doc["park_ID"].GetString());

                snprintf(log_buf_, sizeof(log_buf_) - 1, "[%s] 查询到停车场id[%s] line[%d]", __FUNCTION__, doc["park_ID"].GetString(), __LINE__);
                writelog(log_buf_);

                ret = true;
            }
        }
    }catch (const std::exception& xcp){
        ret = false;

        snprintf(log_buf_, sizeof(log_buf_) - 1, "[%s] error, 查询异常[%s] line[%d]", __FUNCTION__, xcp.what(), __LINE__);
        writelog(log_buf_);
    }

    if(!ret || _vec.empty() || _vec.size() <= 0){
        snprintf(log_buf_, sizeof(log_buf_) - 1, "[%s] error, 查询到停车场id信息失败 line[%d]", __FUNCTION__, __LINE__);
        writelog(log_buf_);
    }

    return ret;
}

/**
 * @brief: 根据表，字段，条件来查询记录
 * @param _table ：表
 * @param _field ：字段
 * @param _condition ：条件
 * @param _vec : 返回查询结果的记录集合
 * @return ：false, 查询失败
 *          true, 查询成功
 */
const bool CMongoDbModel::queryTableField(const char* _table, const char* _field, const char* _condition, std::vector<string>& _vec)
{
    if(!_table || !_field || !_condition)
        return false;

    _vec.clear();

    bool ret = false;

    try {
        mongocxx::uri uri(uri_str_);
        mongocxx::client client(uri);
        mongocxx::database db = client.database(MONGODB_DB);

        auto cursor = db[_table].find({});

        for(auto&& it:cursor){

            std::string json = bsoncxx::to_json(it);

            Document doc;
            doc.Parse(json.c_str());

            if(doc.HasMember(_field) && doc[_field] != "" ){
                _vec.push_back(doc[_field].GetString());

                snprintf(log_buf_, sizeof(log_buf_) - 1, "[%s] 查询到字段[%s]记录[%s] line[%d]", __FUNCTION__, _field, doc[_field].GetString(), __LINE__);
                writelog(log_buf_);

                ret = true;
            }
        }
    }catch (const std::exception& xcp){
        ret = false;

        snprintf(log_buf_, sizeof(log_buf_) - 1, "[%s] error, 查询字段[%s]异常[%s] {表[%s]} line[%d]", __FUNCTION__, _field, _table, xcp.what(), __LINE__);
        writelog(log_buf_);
    }

    if(!ret || _vec.empty() || _vec.size() <= 0){
        snprintf(log_buf_, sizeof(log_buf_) - 1, "[%s] error, 查询字段[%s]信息失败 {表[%s]} line[%d]", __FUNCTION__, _field, _table, __LINE__);
        writelog(log_buf_);
    }

    return ret;
}

/**
 * @brief: 查询车类型
 * @param _plate : 车牌
 * @return ：类型
 */
const bool CMongoDbModel::queryCarType(const std::string& _plate, std::string& _out_car_type)
{
    bool ret = false;

    try {
        mongocxx::uri uri(uri_str_);
        mongocxx::client client(uri);
        mongocxx::database db = client.database(MONGODB_DB);

        // 查询车牌对应的记录，判断车类型
        auto cursor = db["car"].find(make_document(kvp("car_plate_id", _plate.c_str())));

        // 如果找到数据
        for(auto&& it:cursor){

            std::string json = bsoncxx::to_json(it);

            Document doc;
            doc.Parse(json.c_str());

            _out_car_type = doc["car_type"].GetString();

            snprintf(log_buf_, sizeof(log_buf_) - 1, "[%s] 找到车牌[%s]对应的车类型[%s] line[%d]", __FUNCTION__, _plate.c_str(), _out_car_type.c_str(), __LINE__);
            writelog(log_buf_);

            ret = true;

            break;
        }

        // 没有找到数据，则为临时车
        if(!ret){
            _out_car_type = "临时车";

            snprintf(log_buf_, sizeof(log_buf_) - 1, "[%s] warn! 没有在car表中找到车牌[%s]对应的车类型, 则该车为[%s] line[%d]", __FUNCTION__, _plate.c_str(), _out_car_type.c_str(), __LINE__);
            writelog(log_buf_);
        }

        ret = true;

    }catch (const std::exception& xcp){
        ret = false;

        snprintf(log_buf_, sizeof(log_buf_) - 1, "[%s] error, 查询异常[%s] line[%d]", __FUNCTION__, xcp.what(), __LINE__);
        writelog(log_buf_);
    }

    return ret;
}

/**
 * @brief: 写日志，注意日志的文件目录
 * @param _buf :日志缓冲区
 * @attention: 注意日志缓冲区的的大小，写缓冲区时请用snprintf()安全函数写入
 */
void CMongoDbModel::writelog(const char* _buf)
{
    BLog::writelog(_buf, PKGDATA, LOG_DIR_NAME, LOG_FILE_NAME);

    //std::cout << "[" << printTime().c_str() << "] " << (char*)_buf << endl;
}