//
// Created by boon on 17-11-22.
//

#ifndef BPARK_CMONGODBMODEL_H
#define BPARK_CMONGODBMODEL_H

#include "common_def.h"

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/stdx/make_unique.hpp>

#include <mongocxx/options/find.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/logger.hpp>
#include <mongocxx/options/client.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/exception/exception.hpp>

#include <string>
#include <vector>

using namespace std;

namespace ONCALL{

    class CMongoDbModel {
    public:
        CMongoDbModel();
        ~CMongoDbModel();

        // 查询车类型
        const bool queryCarType(const std::string& _plate, std::string& _out_car_type);

    private:
        // 初始化mongodb信息
        void init();

        // 根据车牌号在car表查询这辆车的车辆类型
        const bool queryTableField(const char* _table, const char* _field, const char* _condition, std::vector<string>& _vec);

        // 查询停车场信息
        bool queryParkId(std::vector<string>& _vec);

        // 写日志
        void writelog(const char* _buf);

    private:
        //
        std::string uri_str_;

        // mongodb的ip
        char ip_[32];
        // mongodb的端口号
        unsigned int port_;

        // 日志缓冲区
        char log_buf_[LOG_BUF_SIZE];
    };

};


#endif //BPARK_CMONGODBMODEL_H
