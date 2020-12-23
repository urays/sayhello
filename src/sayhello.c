/*<!--@author:urays @date:2018-01-17 @version:1.1 -->*/
#include "sayhello.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#ifndef uchar
#define uchar unsigned char   // 0 ~ 255    0xff
#endif
#ifndef ushort
#define ushort unsigned short //0 ~ 65535  0xffff
#endif
#define hello_true      (1)
#define hello_false     (0)
#define upper_wait      (50) /*如果主机函数循环上限,大于此值,则解除说话权限*/

typedef struct _pair { void* first, *second; }pair;
void Listen_Echo()/* 消息监听/响应*/

/*消息收发统计*/
MSG MsgStc = { U_MASTER,hello_true,0,0 }; /*默认主机*//*说话权限锁*//*发送统计*//*接受统计*/

/* 初始化地址参数表*//*所有参数值的范围: -120. ~ 120*/
#define FXL_SIZE        (8)

/*<---------------- sayhello 只需修改这里!! ------------------>*/
#include "app_speed_ctrl.h"//此处包含外部参数头文件
#include "MK60_uart.h" //k60串口底层 提供 getchar() 和 putchar()

static pair FixedLink[FXL_SIZE + 1] =/*这张表很关键哦!!!*/
{
{&big_spd,&small_spd},{NULL,NULL},{NULL,NULL},{NULL,NULL},  /*_MSG_DATA_INT2_1*/
{&motoP,NULL},{NULL,NULL},{NULL,NULL},{NULL,NULL},  /*_MSG_DATA_FLOAT_1*/
};

/*Listen_Echo() 消息监听/响应函数 需要一直启用*/
#define _USE_UART_PORT   (UART4) /*使用的串口号!!!*/
extern void say_hello_init(_USER_SG user)/*HELLO初始化!!!*/
{
	MsgStc.user = user; /*主从机初始化*/
	MsgStc.saylock = (user == U_MASTER) ? hello_true : hello_false;

	uart_init(UART4, 115200);/*初始化串口*/
	NVIC_SetPriority(UART4_RX_TX_IRQn, 1);/*串口中断优先级*/
	set_vector_handler(UART4_RX_TX_VECTORn, Listen_Echo);
	uart_rx_irq_en(UART4);  /*使能*/
	uart_tx_irq_dis(UART4);
}

/*发送一个字节 8bit*//*接收一个字节 8bit*/
void send_1_byte(char* byte) { uart_putchar(_USE_UART_PORT, &byte); }
void take_1_byte(char* byte) { uart_getchar(_USE_UART_PORT, byte); }

/*<------ 以下代码为应用层代码,请谨慎修改___enjoy__:) -------->*/

typedef enum _MSG_SIGN {/* 可以包含15个信息标志 */
	/* 数据溢出/或非法 */
	_MSG_ERROR = 121,

	/* 数据类*/
	_MSG_DATA_INT2_1 = -128,
	_MSG_DATA_INT2_2, /* 双整型数据*/
	_MSG_DATA_INT2_3,
	_MSG_DATA_INT2_4,

	_MSG_DATA_FLOAT_1,
	_MSG_DATA_FLOAT_2, /* 浮点型数据*/
	_MSG_DATA_FLOAT_3, /* 精度为小数点2位*/
	_MSG_DATA_FLOAT_4, /*-121 END*/

	/* 命令类*/
	_MSG_COMMAND = 122,/* 组合指令  <cmd1,cmd2>*/
}MSG_SIGN;
void __say_hello(MSG_SIGN _sg, ...);/*发送消息*/

/*
 @请求(主机发出):
       <'x','x'> —— 权限交换请求
	   <'i',1> —— 双整数1请求   <'f',1>  —— 偏差1请求
	   <'i',2> —— 双整数2请求   <'f',2>  —— 偏差2请求
	   <'i',3> —— 双整数3请求   <'f',3>  —— 偏差3请求
	   <'i',4> —— 双整数4请求   <'f',4>  —— 偏差4请求
	   
 @消息(从机发出):
       <-120,-120> —— 返回数据源错误消息
	   <-120,-121> —— 命令错误/不存在消息
	   <'x','k'>   —— 权限交换成功消息
	   <'c','k'>   —— 自定义命令执行成功消息
*/
void echo_cmd_msg(char _cmd1, char _cmd2)
{
	/* _cmd1,cmd2 范围 -120 ~ +120 种类数:241*241*/
	if (MsgStc.user == U_SLAVE)
	{
		MsgStc.saylock = hello_true; /* 主机需要数据,给从机说话的权限*/
		if (_cmd1 == 'x' && _cmd2 == 'x') {
			MsgStc.user = U_MASTER;
			__say_hello(_MSG_COMMAND, 'x', 'k');
			MsgStc.saylock = hello_true; //变为主机后就有了说话权限
		}
		else if (_cmd1 == 'f' && _cmd2 == 1) __say_hello(_MSG_DATA_FLOAT_1);
		else if (_cmd1 == 'i' && _cmd2 == 1) __say_hello(_MSG_DATA_INT2_1);
		else if (_cmd1 == 'f' && _cmd2 == 2) __say_hello(_MSG_DATA_FLOAT_2);
		else if (_cmd1 == 'i' && _cmd2 == 2) __say_hello(_MSG_DATA_INT2_2);
		else if (_cmd1 == 'f' && _cmd2 == 3) __say_hello(_MSG_DATA_FLOAT_3);
		else if (_cmd1 == 'i' && _cmd2 == 3) __say_hello(_MSG_DATA_INT2_3);
		else if (_cmd1 == 'f' && _cmd2 == 4) __say_hello(_MSG_DATA_FLOAT_4);
		else if (_cmd1 == 'i' && _cmd2 == 4) __say_hello(_MSG_DATA_INT2_4);
		//else if (_cmd1 == 'a' && _cmd2 == 'b') {/*自定义命令 从机执行!!!*/
		//	//这里写从机做的事情(FUNCTION)
		//	__say_hello(_MSG_COMMAND, 'c', 'k');/*这句话一定要加*/
		//}
		else { __say_hello(_MSG_COMMAND, -120, -121); }/*执行错误 返回命令不存在*/
	}
	else if (MsgStc.user == U_MASTER) {
		if (_cmd1 == 'x' && _cmd2 == 'k') {
			MsgStc.saylock = hello_false;
			MsgStc.user = U_SLAVE;
		}
		//if (_cmd1 == -120 && _cmd2 == -120)/*数据申请 数据源错误*/
		//if (_cmd1 == -120 && _cmd2 == -121)/*命令执行 从机中该命令不存在*/
		//if (_cmd1 == 'c' && _cmd2 == 'k')  /*命令执行 从机执行完成命令*/
	}
}

//#define _CRC16_BUILD_TABLE /* crc16_create_table() 函数使能标志符*/

/* 最高位定为1，此处省略*/
//#define CRC16_8005    (0x8005) /* X16 + X15 + X2 + X0 */
//#define CRC16_1021    (0x1021) /* X16 + X12 + X5 + X0 */

const ushort CRC16_TAB[256] = {  /* 0x1021 标准*/
0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0,
};

#ifdef _CRC16_BUILD_TABLE
void crc16_create_table(ushort aPoly)
{
	ushort i, j;
	ushort nData, nAccum;

	for (i = 0; i < 256; i++)
	{
		nData = (ushort)(i << 8);
		nAccum = 0;
		for (j = 0; j < 8; j++)
		{
			if ((nData ^ nAccum) & 0x8000)
				nAccum = (nAccum << 1) ^ aPoly;
			else
				nAccum <<= 1;
			nData <<= 1;
		}
		if (i & 7 == 0) printf("\n");
		printf("0x%04x, ", nAccum);
	}
}
#endif

/* crc-16 冗余校验 一般来说 校验2个字节*/
ushort _crc16_check(char* _data, uchar cc)
{
	ushort crc = 0;
	while (cc--)
		crc = (crc << 8) ^ CRC16_TAB[(crc >> 8) ^ (0x80 + *_data++)];
	return crc;
}

/* 异或校验*/
uchar _xor_check(uchar* _data)
{
	uchar res = 0x1b;
	while (*_data) { res ^= *_data++; }
	return res;
}

/* MSG_SIGN + 信息 + 信息 + crc校验字节 + crc校验字节*/
#define MSG_LEN                (5) /* 报文总长*/
#define MSG_DATA_LEN           (2) /* 数据位长度*/

short _msg_crc_check(char* msg)
{
	ushort a = _crc16_check(msg + 1, MSG_DATA_LEN);
	return (!(msg[3] ^ ((a >> 8) - 0x80)) && !(msg[4] ^ (a & 0xff - 0x80)));
}

/* 针对 FixedLink 固定地址的操作 宏*/
#define _fxl_fst(idx)    (FixedLink[idx].first)
#define _fxl_sec(idx)    (FixedLink[idx].second)

#define _fxl_fstv(type,idx,errv)   \
((idx < FXL_SIZE && _fxl_fst(idx)) ? *(type*)_fxl_fst(idx) : errv)

#define _fxl_secv(type,idx,errv)   \
((idx < FXL_SIZE && _fxl_sec(idx)) ? *(type*)_fxl_sec(idx) : errv)

#define _fxl_fstvs(type,idx,setv)  \
{if(idx < FXL_SIZE && _fxl_fst(idx)) *(type*)_fxl_fst(idx) = setv;}

#define _fxl_secvs(type,idx,setv)  \
{if(idx < FXL_SIZE && _fxl_sec(idx)) *(type*)_fxl_sec(idx) = setv;}

/* 信息包装组合 */
#define _msg_data_chk(x)       (x >= -120. && x <= 120.)  /* -120 ~ +120*/
/* 15个预留作为标志位 -128 ~ -121  121 ~ 127 */

#define _msg_float_sign(x)     (x >= _MSG_DATA_FLOAT_1 && x <= _MSG_DATA_FLOAT_4)
#define _msg_float_chk(x)      (x >= -99 && x <= 99)  /* -99 ~ +99*//* 小数位*/

static int mkbufc[2] = { -128,-128 }; /*本地命令缓存*/
/* 编写信息*/ /* 如果信息_MSG_ERROR 则不发送*/
void make_message(char* _msg, MSG_SIGN sg)
{
	int va1 = 0, va2 = 0;
	float va3 = 0.f;

	_msg[0] = _MSG_ERROR;
	_msg[1] = '\0';
	if (_msg_data_chk(sg)) { return; }

	if (_MSG_COMMAND == sg) {/* 组合命令*/
		_msg[1] = _msg_data_chk(mkbufc[0]) ? (char)mkbufc[0] : _MSG_ERROR;
		_msg[2] = _msg_data_chk(mkbufc[1]) ? (char)mkbufc[1] : _MSG_ERROR;
		mkbufc[0] = -128, mkbufc[1] = -128;/*缓存初始化*/
	}
	else if (_msg_float_sign(sg)) { /* 浮点数据*/
		va3 = _fxl_fstv(float, sg + 0x80, 127.f);
		_msg[1] = _msg_data_chk(va3) ? (char)va3 : _MSG_ERROR;
		_msg[2] = (char)((short)(va3 * 100) % 100);
	}
	else { /* 双整数数据*/
		va1 = _fxl_fstv(int, sg + 0x80, 127);
		va2 = _fxl_secv(int, sg + 0x80, 127);
		_msg[1] = _msg_data_chk(va1) ? (char)va1 : _MSG_ERROR;
		_msg[2] = _msg_data_chk(va2) ? (char)va2 : _MSG_ERROR;
	}

	if (_msg[1] ^ _MSG_ERROR &&
		_msg[2] ^ _MSG_ERROR) {
		_msg[0] = sg;
	}
	ushort ck = _crc16_check(_msg + 1, MSG_DATA_LEN);
	_msg[3] = (char)((ck >> 8) - 0x80);
	_msg[4] = (char)(ck & 0xff - 0x80);
	_msg[5] = '\0';
}

/* 执行消息内容*/
void carry_message(char* _msg)
{
	/* _msg已检测为合法
	 * 长度为 MSG_LEN
	 * _msg_data_chk() !_msg_data_chk()
	 * 若_msg_float_sign(msg[0]) = 1 则_msg_float_chk(msg[2]) = 1
	 *  _msg_crc_check(msg) = 1
	 */
	float n = 0.0f;
	/* 发出信息一定是有地址链接的*/
	if (_MSG_COMMAND == _msg[0]) {/* 组合命令*/
		echo_cmd_msg(_msg[1], _msg[2]);/* 执行命令*/
	}
	else if (_msg_float_sign(_msg[0])) { /* 浮点数据*/
		n = _msg[1] * 1.f + _msg[2] / 100.f;
		_fxl_fstvs(float, _msg[0] + 0x80, n);
	}
	else { /* 双整型数据*/
		_fxl_fstvs(int, _msg[0] + 128, _msg[1]);
		_fxl_secvs(int, _msg[0] + 0x80, _msg[2]);
	}
	if (MsgStc.user == U_MASTER) {/* 前提是保证数据可靠传输*/
		MsgStc.saylock = hello_true; /* 只要是主机收到消息 就有说话的权限*/
	}
}

/* 编写消息 并 发送信息*/
void __say_hello(MSG_SIGN _sg, ...)
{
	char msg[6] = { 0 };
	va_list va;

	if (MsgStc.saylock == hello_false) { return; } /* 没有权限说话*/
	if (_sg == _MSG_COMMAND) {
		va_start(va, _sg); /* 写入到本地命令缓存*/
		mkbufc[0] = va_arg(va, int);
		mkbufc[1] = va_arg(va, int);
		va_end(va);
	}
	make_message(msg, _sg);

	if (msg[0] ^ _MSG_ERROR) {
		MsgStc.saylock = hello_false;/*说完话就禁止再说*/
		send_1_byte(&msg[0]), send_1_byte(&msg[1]);
		send_1_byte(&msg[2]), send_1_byte(&msg[3]);
		send_1_byte(&msg[4]);
		MsgStc.send_amo += 5;
	}
	else if (MsgStc.user == U_SLAVE) {
		__say_hello(_MSG_COMMAND, -120, -120);/* 返回数据源错误消息*/
	}
}

static short RecSta = 0; /*主机消息接收状态*/
static ushort wait_cnt = 0;/*主机消息等待计数*/
extern void say_hello(char _cmd1, char _cmd2) {
	/*外部默认只被主机调用*/
	if (MsgStc.saylock == hello_false)
	{
		if (wait_cnt >= upper_wait) { /*主机等待超过了计数上限,则解除说话权限*/
			RecSta = 0;
			MsgStc.saylock = hello_true;
			wait_cnt = 0;
		}
		else { ++wait_cnt; }
	}
	else { wait_cnt = 0; }
	__say_hello(_MSG_COMMAND, _cmd1, _cmd2);
}

static char recMsg[MSG_LEN + 1] = { 0 }; /* 消息缓存*/
void echo_message(char byte)/*消息接收回应状态机*/
{
	switch (RecSta) {
	case(0): {
		if (!_msg_data_chk(byte)) {
			recMsg[RecSta++] = byte;
		}
	}break;
	case(1): {
		RecSta = _msg_data_chk(byte) ? 2 : 1;
		recMsg[RecSta - 1] = byte;
	}break;
	case(2): {
		RecSta = _msg_data_chk(byte) ?
			(_msg_float_sign(recMsg[0]) ?
			(_msg_float_chk(byte) ? 3 : 0) : 3) : 1;
		if (RecSta > 0) {
			recMsg[RecSta - 1] = byte;
		}
	}break;
	case(3): {
		recMsg[RecSta++] = byte;
	}break;
	case(4): {
		recMsg[RecSta] = byte;
		recMsg[RecSta + 1] = '\0';
		if (_msg_crc_check(recMsg)) {
			carry_message(recMsg);
		}
		RecSta = 0;
	}break;
	}
}

void Listen_Echo()/* 消息监听/响应*/
{
	char recc = '\0';
	take_1_byte(&recc); /*接收一个字节*/
	echo_message(recc); /*分析/响应消息*/
	++MsgStc.rec_amo;
}
