/*********************************************************************************
  *Copyright(C):        sdboon.com
  *FileName:            unit_client.cpp
  *Author:              diaoguangqiang
  *Version:             2.0
  *Date:                2017.09.15
  *Description:         单元测试，模拟发包
  *History:             (修改历史记录列表，每条修改记录应包含修改日期、修改者及修改内容简介)
     1. Date:           2017.09.15
        Author:         diaoguagnqiang
        Modification:   首次生成文件
     2. Data:
        Author:
        Modification:
**********************************************************************************/

#include "common_def.h"
#include "my_client.h"

int test_web(int argc, char** argv);

int main(int argc, char** argv)
{
    test_web(argc, argv);
}

int test_web(int argc, char** argv)
{
    if(argc < 3){
        printf("./unit_client ip port\n");
        return -1;
    }

    printf("begin\n");

    char ip[32] = {0};
    memcpy(ip, argv[1], sizeof(ip));
    short port = atoi(argv[2]);

    base_cb_cli_attr cli_cfg;
    cli_cfg.recv_buf_len = MSG_BUFF_LEN;
    cli_cfg.send_buf_len = MSG_BUFF_LEN;

    my_client* cli = new my_client(&cli_cfg);

    if(cli->connect(ip, port)){
        printf("[%s] 连接oncallserver成功 ip[%s] port[%d] line[%d]\n", printTime().c_str(), ip, port, __LINE__);
    }else{
        printf("[%s] error, 连接oncallserver失败 ip[%s] port[%d] line[%d]\n", printTime().c_str(), ip, port, __LINE__);
        return -1;
    }

    cli->start();

    while(1){

        cli->sendIn();

        sleep(60);

        cli->sendPay();

        sleep(60);

        cli->sendOut();

        sleep(60);
    }

    delete cli;

    printf("end\n");
}