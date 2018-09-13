#ifndef BOON_MYSQL_H
#define BOON_MYSQL_H

#include "../global/boon_global.h"
#include <mysql/mysql.h>


int mysql_connect();//连接mysql
int mysql_query_host(char * host_ip,char * host_server_ip);//查询主机ip和服务器ip
int mysql_get_ipc_config(); //给bipc发送 ipc config 信息
int mysql_get_ipc_config_bop(); //给bop发送 ipc config 信息
int mysql_get_ipc_config_bvs(); //给bvs发送 ipc config 信息
int mysql_query_car(char *flag,char * plate);//入口线程查询这辆车在car表里的个数
int mysql_query_channel(char *flag,char * id,char *auth_type,char * open_door,char * one_way,char * mohu_match,char *park_id);//入口线程查询通道属性
int mysql_mohu_match(char *flag,char *pcolor,char *inplate,char *outplate);//入口线程mysql的模糊匹配
int mysql_delete_car_inpark(char *flag,char * plate,char *park_id);// 入口线程删除在场车辆
int mysql_auth_plate(char *flag,char *id,char *plate,bool *auth);//入口线程按车牌号授权
int mysql_auth_type(char *flag,char *id,char *plate,bool *auth);//入口线程按车辆类型授权
int mysql_validate_chewei_num(char *plate,bool *auth);//入口线程车位数验证
int mysql_validate_guoqi(char *flag,char *id,char *plate,char *type,bool *auth,int *remain_day,int *remain_hour,int * remain_money,char * charge_rule);//入口线程月租车，储值车，储时车过期验证
int mysql_process_top_park(char * id,char *plate,char * charge_rule,char *car_type,car_msg *msg,char *open_door);//处理上级停车场
int mysql_write_in_park(car_msg *in_msg,char *ori_plate,char *final_plate,char *park_id,char *global_car_type,bool *auth,char *open_door);//写数据到入口表和在场表
int mysql_process_led(char *flag,char *plate,char* show_type,char * show_text,char *msg);//根据数据库配置，拼接LED显示信息
int mysql_send_bled(char *flag,char *car_type,char *plate,char *id,char *speak_send);//给bled发送led显示信息和语音信息
int mysql_process_top_park_out(char * id,char *plate,char * charge_rule,char * car_type,char *park_id);//出口处理上级停车场
int mysql_query_carinpark(char *flag,char *channel_id,char * plate,char *in_time,char *in_pic_path,char *in_channel_id,char *people_name,char *in_channel_name);//查询在场表
int mysql_cal_fee(char *rule,char *in_time,char *out_time,char *car_type,char *id,char *plate);//计费
int mysql_write_caroutrec(char *in_time,int pay_charge,int real_charge,char *plate_ori,char *plate_final,char *in_pic_path,char *in_channel_id,char *park_id,char *park_parent_id,char *car_type,char *operator_name,char *open_door,char *charge_type,car_msg *out);//写出场表
int mysql_write_carinpark(char *plate_ori,char *plate_final,char *park_id,char *car_type,char *people_name,car_msg *msg);//写在场表
int mysql_process_fee(char *channel_id,char *plate);//计费
int mysql_send_bgui_start();//发送bgui 启动信息
int mysql_bgui_login(char *name,char *password);//bgui 登录
int mysql_bgui_process_door(char *type,char *op);//处理bgui 开闸信息
int mysql_bgui_process_youhui(char *type,char *plate);//bgui 处理优惠
int mysql_bgui_charge_pass(char *plate);//收费放行
int mysql_bgui_free_pass(char *plate);//免费放行
int mysql_bgui_modif_passwd(char *name,char *old_pass,char *new_pass);//gui 修改密码
int mysql_process_chewei(char *channel_id,char *in_out,char *flag);//车位计算
int mysql_bgui_modif_parkcount( char *count);//bgui 修改剩余车位
int mysql_query_space_count( char *flag,char *space_count);//查询剩余车位
int mysql_get_welcome_msg();
#endif
