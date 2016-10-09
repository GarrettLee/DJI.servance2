#ifndef GLOBALVAR_H
#define GLOBALVAR_H
#define sendBySocket 1					//为1时会启动socket1，把解码后的bitmap图片发送到本机的其他应用，
#define ENABLE_WHOLENESS_CHECK 0		//为1时会启动码流完整性检查，否则默认认为码流是完整的
#define DECODE_NATIVE_CODE 0			//为1时从项目目录下读取作为例子用的本地h.264文件做解码，这样就能方便解码器的调试
#define FLIGHT_CONTROL 1				//为1时会启用大疆的飞行控制
//#define SERVANCE_IP "192.168.0.114" 
#define SERVANCE_IP "172.20.10.9"		//服务器（本机）ip地址
//#define SERVANCE_IP "192.168.0.152"		//服务器（本机）ip地址
#define MATLAB_IP "127.0.0.1"
#endif

