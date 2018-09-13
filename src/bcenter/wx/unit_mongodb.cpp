//
// Created by boon on 17-11-25.
//

#include <mongoc.h>
#include <stdio.h>

mongoc_client_t *mongodb_client;
char str_con[128] = {0};

int test_connect();

int main(int argc, char** argv)
{
    test_connect();
}

int test_connect()
{
    char host_server_ip[] = {"127.0.0.1"};

    char str[128] = {0};
    sprintf(str,"nc -z  -w 1 %s 27017",host_server_ip);
    if(system(str) != 0)
    {
        return -1;
    }

    // mongodb://127.0.0.1:27017
    sprintf(str_con,"mongodb://boon:boon123456@%s:27017/?authSource=boondb",host_server_ip); //数据库连接地址
    //sprintf(str_con,"mongodb://boon:boon123456@%s:27017/?authSource=boondb&socketTimeoutMS=30&connectTimeoutMS=30",host_server_ip); //数据库连接地址
    //mongodb_uri = mongoc_uri_new(str);
    //mongodb_pool = mongoc_client_pool_new(mongodb_uri);

    printf("%s\n", str_con);

    mongoc_init();
    mongodb_client = mongoc_client_new(str_con);
}