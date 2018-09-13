
#include <string.h>
#include "bswap.h"
#include "base_msg.h"

int base_pack_msg(char *dest,int cmd,char *src,int len)
{
    struct msg_hdr *hdr = (struct msg_hdr *)dest;
    hdr->cmd = cmd;
    hdr->len = MSG_HDR_LEN+len;//
    hdr->endin = BASE_MSG_ENDIN;
    hdr->ver = BASE_MSG_VER;
    if ( (len > 0) && (NULL != src) )
    {
        memcpy(dest+MSG_HDR_LEN,src,len);
    }
    return hdr->len;
}

struct msg_hdr *base_unpack_msg(char *src,int len)
{
    if(len < MSG_HDR_LEN )
    {
        return NULL;
    }
    //
    struct msg_hdr *hdr = (struct msg_hdr *)src;
    if( hdr->endin != BASE_MSG_ENDIN )
    {
        hdr->endin = byte_swap(hdr->endin);
        hdr->len = byte_swap(hdr->len);
    }
    return hdr;
}

int base_unpack_udpmsg(udpmsg *msg)
{
    if( msg->endin != BASE_MSG_ENDIN )
    {
        msg->endin = byte_swap(msg->endin);
        msg->totallen = byte_swap(msg->totallen);
        msg->len = byte_swap(msg->len);
        msg->totelcount = byte_swap(msg->totelcount);
        msg->seq = byte_swap(msg->seq);
        msg->id = byte_swap(msg->id);
    }
    return 0;
}