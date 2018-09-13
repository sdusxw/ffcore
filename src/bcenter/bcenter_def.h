//
// Created by boon on 17-11-21.
//

#ifndef BPARK_BCENTER_DEF_H
#define BPARK_BCENTER_DEF_H

#undef LOG_DIR_NAME
#undef LOG_FILE_NAME

#define LOG_DIR_NAME    "BoonCamera/log/bcenter"
#define LOG_FILE_NAME   "bcenter"

// 定义udp端口
//#define PORT_UDP_BLED_TO_BCENTER     (5001)
#define PORT_UDP_BCENTER_TO_BLED     (5002)
#define PORT_UDP_BGUI_TO_BCENTER     (5003)
#define PORT_UDP_BCENTER_TO_BGUI     (5004)
#define PORT_UDP_BCENTER_TO_BLED1    (5005)
//#define PORT_UDP_BCENTER_TO_BLED2    (5006)
#define PORT_UDP_BVS_TO_BCENTER      (5009)
#define PORT_UDP_BCENTER_TO_BVS      (5010)
#define PORT_UDP_BCENTER_TO_BOP      (5012)
#define PORT_UDP_BCENTER_TO_ONCALLCLIENT (6002)
#define PORT_UDP_BCENTER_TO_BIPC     (5021)
#define PORT_UDP_BIPC_TO_BCENTER     (5022)

#endif //BPARK_BCENTER_DEF_H
