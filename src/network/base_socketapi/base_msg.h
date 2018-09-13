//
// Created by boon on 17-11-6.
//

#ifndef TLRS_BASE_MSG_H
#define TLRS_BASE_MSG_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <stdint.h>//

#define BASE_MSG_VER 1
#define BASE_MSG_ENDIN 1

#define BASE_MSG_DATA_NOTIFY 0
#define BASE_HEARTBEAT_REQ 1
#define BASE_HEARTBEAT_RSP 2
#define BASE_MSG_DATA_REQ 3
#define BASE_MSG_DATA_RSP 4

//
#pragma pack(1)
struct msg_hdr
{
    uint8_t cmd;//
    uint8_t ver; //
    uint16_t  endin;//
    uint32_t len;//
};
#pragma pack()
#define MSG_HDR_LEN sizeof(struct msg_hdr)
extern  int base_pack_msg(char *dest,int cmd,char *src,int len);
extern struct msg_hdr *base_unpack_msg(char *src,int len);
#pragma pack(1)
struct udpmsg
{
    uint64_t seq;//
    uint32_t totallen;//
    uint32_t len;//
    uint32_t totelcount;//
    uint32_t id;//
    uint8_t checksum[16];//
    uint16_t endin;//
    uint8_t  cmd; //
    char data[0];
};
#pragma pack()
extern  int base_unpack_udpmsg(udpmsg *msg);
#define UDPMSG_HDR_LEN sizeof(udpmsg)

#endif //TLRS_BASE_MSG_H