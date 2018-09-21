#ifndef BOON_MONGODB_H
#define BOON_MONGODB_H

#include "../global/boon_global.h"
#include <mongoc.h>

int mongodb_exit();
int mongodb_connect();      //mongodb连接 
int mongodb_get_ipc_config(int _dst_port); //获取ipc config
int mongodb_get_white_list(int _dst_port); // 获取白名单列表
int mongodb_get_ipc_config_bop(); //获取ipc config
int mongodb_get_ipc_config_bvs(); //获取ipc config
int mongodb_query_car(char * plate); //查询这辆车在car表的个数
int mongodb_delete_car_inpark(char *plate);//删除该车辆在在场表里的记录
int mongodb_mohu_match(char *pcolor,char *inplate,char *outplate);//mongodb的模糊匹配
int mongodb_mohu_match_out(char *pcolor,char *inplate,char *outplate);//mongodb的模糊匹配, 出口
int mongodb_query_channel(char * id,char *auth_type,char * open_door,char * one_way,char * mohu_match,char *park_id);//根据通道id号查询通道属性
int mongodb_query_cartype(char * plate,char *type);//根据车牌号在car表查询这辆车的车辆类型
int mongodb_query_car(char * plate);//查询这辆车在car表里的个数
int mongodb_delete_car_inpark(char *plate,char *park_id);//删除该车辆在在场表里的记录
int mongodb_auth_plate(char *id,char *plate,bool *auth);//按车牌号授权
int mongodb_auth_type(char *id,char *plate,bool *auth);//按车辆类型授权
int mongodb_validate_chewei(bool *flag,char *channel);//车位数验证是否开启
int mongodb_validate_chewei_num(char *plate,bool *auth);//车位数验证
int mongodb_validate_chewei_num_out(char *plate,bool *auth);//车位数验证, 20171213 11:35 针对huilongguagnchang 出口验证
int mongodb_validate_guoqi(char *flag,char *id,char *plate,char *type,bool *auth,int *remain_day,int *remain_hour,int * remain_money,char * charge_rule);//月租车，储值车，储时车过期验证
int mongodb_validate_full_fangxing(char *parkid);//车位数满时验证放行
int mongodb_validate_blacklist(char *plate,char *blacklistreason);    //验证黑名单
int mongodb_process_top_park(char * id,char *plate,char * charge_rule,char *car_type,car_msg *msg,char *open_door);//入口处理上级停车场
int mongodb_write_in_park(car_msg *in_msg,char *ori_plate,char *final_plate,char *park_id,char *global_car_type,bool *auth,char *open_door);//写数据到入口表和在场表
int mongodb_process_led_in(char *plate,char* show_type,char * show_text,char *msg);//根据数据库配置，拼接LED显示信息
int mongodb_send_bled(car_msg *msg,char *flag,char *car_type,char *plate,char *id,char *speak_send,int money,int park_time);//给bled发送led显示信息和语音信息
int mongodb_process_top_park_out(char * id,char *plate,char * charge_rule,char *car_type,char *park_parent_id);//出口处理上级停车场
int mongodb_query_carinpark(char * channel_id,char *plate,char *in_time,char *in_pic_path,char *in_channel_id,char *people_name,char *in_channel_name);//查询在场表
int mongodb_cal_fee(char *rule,char *in_time,char *out_time,char *car_type,char *id,char *plate);//计费
int mongodb_write_caroutrec(char *in_time,int pay_charge,int real_charge,char *plate_ori,char *plate_final,char *in_pic_path,char *in_channel_id,char *park_id,char *park_parent_id,char *car_type,char *operator_name,char *open_door,char *charge_type,car_msg *out);//写出场表
int mongodb_write_carinpark(char *plate_ori,char *plate_final,char *park_id,char *car_type,char *people_name,car_msg *msg);//写在场表
int mongodb_process_fee(char *channel_id,char *plate,char *in_time,char * out_time);//计费
int mongodb_send_bgui_start();//发送bgui 启动信息
int mongodb_bgui_login(char *name,char *password);//bgui 登录
int mongodb_bgui_process_door(char *type,char *op);//处理bgui 开闸信息
int mongodb_bgui_process_youhui(char *type,char *plate);//bgui 处理优惠
int mongodb_bgui_charge_pass(char *plate);//收费放行
int mongodb_bgui_free_pass(char *plate);//免费放行
int mongodb_bgui_modif_passwd(char *name,char *old_pass,char *new_pass);//gui 修改密码
int mongodb_process_chewei(char *channel_id,char *in_out,char *flag);//车位计算
int mongodb_bgui_modif_parkcount( char *count);//bgui 修改剩余车位
int mongodb_query_space_count(char *space_count);//查询剩余车位
int mongodb_query_remain_space_count(char *space_count);//查询剩余车位
int mongodb_get_welcome_msg(int _dst_port);
int mongodb_jinan_2017(char *id,char *plate,char *car_type,char *in_time,char *out_time);
int mongodb_speak_wannengyuyin(char *in_out, char *plate, char* car_type, int fee, car_msg *msg, char *speak_send);

//********* 微信支付逻辑 ********
int mongodb_process_wx_carout(char *name,char *park_id,char *box_ip,char *plate1,char *openid, char *outime, char* userid);
int mongodb_process_wx_tcp_carout(char *name,char *park_id,char *box_ip,char *plate1,char *openid, char *outime, char* userid, int sock);
int mongodb_process_wx_pay_open( char *money,char *park_id,char * box_ip,char *plate,char *openid, char *flag, char* userid); // 出口，支付完成，开闸
int mongodb_process_wx_pay_open( char *money,char *park_id,char * box_ip,char *plate, char *openid, char *flag, char* userid, const char* _in_dis_money, const char* _in_fact_money); // 出口，支付完成，开闸
int mongodb_process_wx_tcp_pay_open( char *money,char *park_id,char * box_ip,char *plate,
                                    char *_in_openid, char *flag, char* _in_userid,
                                    const char* _in_dis_money, const char* _in_fact_money, int sock);
int mongodb_process_wx_pay( char *money,char *park_id,char * box_ip,char *plate,char *openid, char *flag, char* userid);
int mongodb_process_wx_pay_in(const char *money, const char *park_id, const char *plate,  const char *openid, const char* userid, const char *flag, const char* _in_dis_money, const char* _in_fact_money); // 场内 支付完成
int mongodb_process_wx_tcp_pay_in(const char *_in_sum_money, const char *park_id, const char *plate,
                                  const char *_in_openid, const char* _in_userid, const char *flag,
                                  const char* _in_dis_money, const char* _in_fact_money, int sock);
int mongodb_process_wx_fangxing(); // 微信支付成功后，需要放行,并语音播报
int mongodb_process_wx_opendoor(); // 由wx向bled发送消息 (微信支付成功后，需要放行,并语音播报)
int mogodbb_process_wx_sendto_bled(const std::string& _msg); // udp消息
int mogodbb_process_wx_udp_send(const std::string& _msg, const std::string& _ip, const int& _port); // udp消息
int mogodbb_process_wx_tcp_send(const std::string& _msg, int sock); // tcp消息
int mongodb_process_wx_query_fee_in(const char* _in_cmd, const char *park_id, const char *plate, const char *openid, const char* userid, const char *outime); // 场内查询支付费用
int mongodb_process_wx_tcp_query_fee_in(const char* _in_cmd, const char *_in_park_id, const char *_in_plate, const char *_in_openid, const char* _in_userid, const char *_in_outime, int sock);//TCP场内查询支付费用

//******* 扫码枪业务逻辑 *********
bool mongodb_bgui_smq_insert(Json::Value& _in_json_value); // 扫码枪的数据入库
int mongodb_bgui_smq_opendoor(); // 扫码枪开闸
int mogodbb_process_udp_send(const std::string& _msg, const std::string& _ip, const int& _port); // udp消息
bool mongodb_get_jsondata(const std::string& _cmd, std::string& _rsp, bool flag = true);

//********* 洗车逻辑 *********
bool mongodb_jici_query_carinrec_duration_min(int& _out_dur_min, const char* _in_plate, const char* _in_current_time) ;  // 查询　carinrec表，获取车牌最后一条记录，获取时间分钟，判断持续分钟
int mongodb_jici_update_carinrec_jici_counts(const char* _in_plate) ; // 查询car表，　设置计次次数减１操作

#endif

