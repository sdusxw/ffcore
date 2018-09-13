/*********************************************************************************
  *Copyright(C):        sdboon.com
  *FileName:            common_def.h
  *Author:              diaoguangqiang
  *Version:             2.0
  *Date:                2017.09.16
  *Description:         配置文件，预定义变量
  *History:             (修改历史记录列表，每条修改记录应包含修改日期、修改者及修改内容简介)
     1. Date:           2017.09.16
        Author:         diaoguagnqiang
        Modification:   首次生成文件
**********************************************************************************/

#ifndef COMMON_DEF_H
#define COMMON_DEF_H

#include <cstring>

using namespace std;

#pragma once

#ifndef LOG_BUF_SIZE
#define LOG_BUF_SIZE (1024*8)
#endif

// 消息缓冲区大小
#define MSG_BUFF_LEN (1024 * 512)

// 服务端ip/port, 针对停车场, 给oncallclient提供服务
#define IP_ALI_YUN_SERVER ("47.93.19.77") // ("47.93.19.77")
#define PORT_ALI_YUN_SERVER (7666)  // todo 7666 或 3389

// 服务端ip/port, 针对手机端，给web提供服务
#define PORT_SERVER_WEIXIN (7667)

//提供给烽火的服务端口  by Tian
#define PORT_SERVER_FH (7668)

// bcenter所在的机器, 默认往88.10发送消息
#define IP_UDP_BCENTER_SERVER ("192.168.88.10")
#define PORT_UDP_BCENTER_SERVER (6001)

#define IP_UDP_LOCAL ("127.0.0.1")
#define PORT_UDP_LOCAL (6002)

#ifndef MAX
//#undef MAX
#define MAX(a,b)        ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
//#undef MIN
#define MIN(a,b)        ((a) < (b) ? (a) : (b))
#endif

// 频道列表 或 命令类型
#define C_HEARTBEAT                 ("beat")           // 心跳
#define C_REGISTER_PARK_ID          ("init_parkid")    // oncallclient与oncallserver之间消息
#define C_CMD_IN                    ("in")             // 入场扫码
#define C_CMD_OUT                   ("out")            // 出场查询费用
#define C_CMD_PAY                   ("pay")            // 支付，两种情况，一是场内支付，一是出场支付
#define C_CMD_BIND                  ("bind_openid")    // 场内绑定
#define C_CMD_QUERY_PAY_IN          ("query_pay")      // 场内查询支付费用
#define C_CMD_SMQ_PAY               ("smq_pay")        // 出场扫描枪支付

/**
 * @brief: 通讯消息头
 */
/*typedef struct msg_head_node{
    int type;
    int len;

    msg_head_node(){
        type = 1;
        len = 0;
    }
}msg_head, *pmsg_head;

#define MSG_HEAD_LEN (8)
*/
/**
     * 分配给客户端的缓冲区
     */
typedef struct tcp_buf_node
{
    char data[MSG_BUFF_LEN];
    int len;

    tcp_buf_node() {
        memset(data, 0, sizeof(data));
        len = 0;
    }
}tcp_buf;

/**
 * @brief : 系统配置表
 */
struct s_sysconfig_node{
    char device_sn[32];             // 设备序列号
    char device_type[32];           // 设备类型,动车/机车/地铁
    char device_location[128];      // 安装点位
    char device_longitude[32];      // 安装点位经度
    char device_latitude[32];       // 安装点位维度
    char device_mfd[32];            // 生产日期
    char deivce_sshport[32];        // 远程管理 ssh 端口号
    char device_ipaddr[32];         // 设备 ip 地址
    char deivce_tcpport[32];        // 设备 TCP 服务端口,默认 6000
    char deivce_enable_match[32];   // 是否启用内部车号匹配

    s_sysconfig_node(){
        memset(device_sn, 0, sizeof(device_sn));
        memset(device_type, 0, sizeof(device_type));
        memset(device_location, 0, sizeof(device_location));
        memset(device_longitude, 0, sizeof(device_longitude));
        memset(device_latitude, 0, sizeof(device_latitude));
        memset(device_mfd, 0, sizeof(device_mfd));
        memset(deivce_sshport, 0, sizeof(deivce_sshport));
        memset(device_ipaddr, 0, sizeof(device_ipaddr));
        memset(deivce_tcpport, 0, sizeof(deivce_tcpport));
        memset(deivce_enable_match, 0, sizeof(deivce_enable_match));
    }
};

/**
 * @brief: 工业相机表
 */
struct s_camera_node{
    char camera_ip[32];             // 工业相机 IP 地址
    char camera_index[32];          // 工业相机标识,只能是 A/B/C
    char camera_sn[128];            // 相机序列号
    char camera_model[32];          // 工业相机型号
    char camera_expo_min[32];       // 曝光时间下限
    char camera_expo_max[32];       // 曝光时间上限
    char camera_gain_min[32];       // 增益下限
    char camera_gain_max[32];       // 增益上限
    char camera_target_gray[32];    // 工业相机目标亮度
    char camera_fps[32];            // 工业相机帧率

    s_camera_node(){
        memset(camera_ip, 0, sizeof(camera_ip));
        memset(camera_index, 0, sizeof(camera_index));
        memset(camera_sn, 0, sizeof(camera_sn));
        memset(camera_model, 0, sizeof(camera_model));
        memset(camera_expo_min, 0, sizeof(camera_expo_min));
        memset(camera_expo_max, 0, sizeof(camera_expo_max));
        memset(camera_gain_min, 0, sizeof(camera_gain_min));
        memset(camera_gain_max, 0, sizeof(camera_gain_max));
        memset(camera_target_gray, 0, sizeof(camera_target_gray));
        memset(camera_fps, 0, sizeof(camera_fps));
    }
};

/**
 * @brief: 车号配置表
 */
struct s_train_label_cfg_node{
    char train_type[32];
    char label_type[32];
    char label_enable[32];

    s_train_label_cfg_node(){
        memset(train_type, 0, sizeof(train_type));
        memset(label_type, 0, sizeof(label_type));
        memset(label_enable, 0, sizeof(label_enable));
    }
};

#endif // COMMON_DEF_H