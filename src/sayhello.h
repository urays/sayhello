/*
 * @brief: 串行通信协议.半双工.主从权限交换(单处理机)
 * @author: urays
 * @date:2018-01-17
 * @github:https://github.com/urays
 */
#ifndef _UART_SAY_HELLO_
#define _UART_SAY_HELLO_

typedef enum __USER_SG {
	U_SLAVE, /* 从机*/ U_MASTER, /* 主机*/
}_USER_SG;

typedef struct _MSG_ {
	short user,/*主机or从机*/saylock;/*说话权限锁*/
	int send_amo,/*发送字节数*/rec_amo;/*接受字节数*/
}MSG;
/*
  @组合命令说明:可扩展命令:241*241-12 = 58068种
  @系统保留辅助消息命令(4种)详见源文件注释
  @系统保留请求命令(9种)
  <'x','x'> —— 权限交换请求
  <'i',1> —— 双整数1请求   <'f',1>  —— 偏差1请求
  <'i',2> —— 双整数2请求   <'f',2>  —— 偏差2请求
  <'i',3> —— 双整数3请求   <'f',3>  —— 偏差3请求
  <'i',4> —— 双整数4请求   <'f',4>  —— 偏差4请求
  @组合命令示例:say_hello('f',1);
  @添加拓展命令:只需在在源文件echo_cmd_msg()函数中按示例添加自定义命令即可。
  主机直接发送该命令,从机就能响应执行该命令:)
*/

/*<!-- @串口硬件/软件初始化 -->*/
extern void say_hello_init(_USER_SG user);

/*<!-- @主机发送请求 -->*/
extern void say_hello(char _cmd1, char _cmd2);

/*<!-- @消息统计参数 -->*/
extern MSG MsgStc;/* 测试验证用*/


#endif
