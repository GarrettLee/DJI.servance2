//SOCKET工具
//这里约定的数据帧格式为
/*
	┏━━━━━┯━━━┯━━━┱──┬─……─┬───┐
    ┃START_FLAG│ 帧长1│ 帧长2┃ 控制帧/H264码流帧    │
	┗━━━━━┷━━━┷━━━┹──┴─……─┴───┘
	其中控制帧的格式见FlightControl.cpp，H264码流帧则是裸流，帧长1和帧长2反映后面控制帧/H264码流帧的长度
	getHead函数获取的是钱三个字节，也就是START_FLAG，帧长1，帧长2,
*/

#include <stdio.h>                      //用于输入、输出函数的调用,printf, gets
#include "SocketUtils.h"



#define START_FLAG 0x11					//socket的文件头，这是本人自己约定的一个文件头

//把字节写到STREAM中去
int wrightBytes2Stream(SOCKET s, char* data, unsigned int len) {
	if (len <= 0xffff){
		unsigned char head[3];
		//给TCP的数据加个数据头，内容为该段数据的长度
		head[0] = START_FLAG;
		head[1] = len & 0x00ff;
		head[2] = len >> 8;					//把数据的长度拆成两个字节发出去

		if ((head[1] == 0)&(head[2] == 0)){
			return -1;
		}
		if (send(s, (char *)head, 3, NULL) <= 0)
			return -1;
		int num = send(s, data, len, NULL);
		if (num <= 0)
			return -1;
	}
	else{
		int remain = len;
		int package_num = len / 0xffff + 1;
		char cmd[] = "big data";
		wrightBytes2Stream(s, cmd, sizeof(cmd) / sizeof(char));
		char num[] = { package_num };
		wrightBytes2Stream(s, num, 1);
		int j = 0;
		for (int i = 0; i < package_num - 1; i++){
			if (wrightBytes2Stream(s, data, 0xffff) == -1){
				i--;
				continue;
			}
			data = data + 0xffff;
			remain -= 0xffff;
			//Sleep(0);
		}
		wrightBytes2Stream(s, data, remain);
	}
	return 1;
}

//从缓存中获取一个数据头，返回该组数据的长度
int getHead(SOCKET s){
	int dataLength = 0;
	unsigned char start[3] = { 0 };
	int result = recv(s, (char *)start, 3, NULL);
	if (result == 0){
		return  -2;
	}
	if (start[0] == START_FLAG){
		dataLength = (start[1] & (int)0xff) + ((start[2] << 8)&(int)0xffff);
	}
	else{
		printf("文件头出错");
		return -1;
	}
	return dataLength;
}

//根据长度从缓存中取出额定长度的数据，如果取出数据时取得过多数据造成溢出，则返回溢出的数据，返回的这个数据需要就行free操作
char* readByteFromStream(SOCKET s, char * out, int dataLength) {

	int data_left = dataLength;
	char * frame_ptr = out;
	char * overflow = NULL;
	while (data_left > 0)
	{
		char buffer[10000] = { 0 };
		if (dataLength > 10000){
			return NULL;
		}
		int rev_num = recv(s, buffer, data_left, NULL);
		//一直接收客户端socket的send操作
		if (rev_num == 0){
			continue;
		}
		if (rev_num <= data_left){
			memcpy(frame_ptr, buffer, rev_num * sizeof(char));
			frame_ptr += rev_num * sizeof(char);
			data_left -= rev_num;
		}
		else if (rev_num > data_left){
			printf("ERROR：读取数据溢出！");
			int overflow_num = rev_num - data_left;
			memcpy(frame_ptr, buffer, data_left * sizeof(char));
			overflow = (char *)malloc(overflow_num * sizeof(char));
			char * start_of_overflow = buffer + data_left;
			memcpy(overflow, start_of_overflow, overflow_num * sizeof(char));
			data_left = 0;
		}
	}
	return overflow;
}


//将中文字符转成UTF8编码的字节流
int createUTF8Byte(char *chinese_string, char *out, int out_len){
	wchar_t wc[256];
	// 将ANSI编码的多字节字符串转换成宽字符字符串  
	int n = MultiByteToWideChar(CP_ACP, 0, chinese_string, strlen(chinese_string), wc, 256);
	if (n > 0)
	{
		wc[n] = 0;

		// 将宽字符字符串转换成UTF-8编码的多字节字符串  
		n = WideCharToMultiByte(CP_UTF8, 0, wc, wcslen(wc), out, out_len, NULL, NULL);
		if (n > 0)
		{
			out[n] = 0;
		}
	}
	return n;
}

//从字节命令中返回一个带符号的int16数
int encodeOneInt(char character, char pre, char next){
	return character * (int)(((unsigned char)pre & 0xff) + (((unsigned char)next << 8) & 0xffff));
}

int decodeInts(int * int_array, int len_in,  unsigned char * bytes, int start, int len){
	for (int i = 0; i < len_in; i++){
		if (i * 3 <= len - start){
			if (int_array[i] >= 0){
				bytes[start + 3 * i] = 1;
			}
			else{
				bytes[start + 3 * i] = 255;
			}
			bytes[start + 3 * i + 1] = int_array[i] & 0x00ff;
			bytes[start + 3 * i + 2] = int_array[i] >> 8;
		}
	}
	return -1;
}

//创建短连接
void createShortConnection(char* ip, int port, SOCKET * socket_short){
	WSADATA wsaData;
	sockaddr_in sockaddr;

	char cmd[] = "允许开始长连接";
	char chinese_cmd[100];
	//socket1：用来把解码后得到的bitmap图片发出去，这里就发回本机的其他程序
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	*socket_short = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	sockaddr.sin_family = PF_INET;
	sockaddr.sin_addr.S_un.S_addr = inet_addr(ip);
	sockaddr.sin_port = htons(port);

	connect(*socket_short, (SOCKADDR*)&sockaddr, sizeof(SOCKADDR));
}

//创建长连接
int createLongConnection(char* ip, int port, int port_long, SOCKET * socket_heartbreak_ptr, SOCKET * socket_long_ptr){

	char cmd[] = "允许开始长连接";		//建立长连接的命令
	char chinese_cmd[100];				//用来存放建立长连接中需用用到的指令
	WSADATA wsaData_in;
	SOCKET s_in;
	sockaddr_in sockaddr_input;
	SOCKET clientsocket_h264code = *socket_long_ptr;


	WSADATA wsaData_long;
	SOCKET socket_long;
	sockaddr_in sockaddr_long;
	//socket2：用来为与手机端的长链接的建立和退出服务
	WSAStartup(MAKEWORD(2, 2), &wsaData_in);
	s_in = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_input.sin_family = PF_INET;
	sockaddr_input.sin_addr.S_un.S_addr = inet_addr(ip);   //需要绑定到本地的哪个IP地址
	sockaddr_input.sin_port = htons(port);                          //需要监听的端口
	bind(s_in, (SOCKADDR*)&sockaddr_input, sizeof(SOCKADDR));        //进行绑定动作

	listen(s_in, 1);

	SOCKADDR clientAddr;
	int size = sizeof(SOCKADDR);

	SOCKET clientsocket = *socket_heartbreak_ptr;

	//进行通信，确认可以建立长链接
	*socket_heartbreak_ptr = accept(s_in, &clientAddr, &size);               //阻塞，直到有新tcp客户端连接
	closesocket(s_in);
	int len = getHead(*socket_heartbreak_ptr);
	if (len == -1) {
		return -1;
	}
	char * buffer_in = (char *)malloc(len * sizeof(char));
	char * overflow = readByteFromStream(*socket_heartbreak_ptr, buffer_in, len);
	if (overflow != NULL){
		free(overflow);
		return -1;
	}
	if (strcmp(buffer_in, "请求长连接") == 1){
		printf("开始长连接");
	}
	free(buffer_in);

	int n = createUTF8Byte(cmd, chinese_cmd, 100);	//把中文字符串转成UTF编码以便发送

	wrightBytes2Stream(*socket_heartbreak_ptr, chinese_cmd, n);
	//socket3：长链接的socket，用来接受视频码流
	WSAStartup(MAKEWORD(2, 2), &wsaData_long);
	socket_long = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_long.sin_family = PF_INET;
	sockaddr_long.sin_addr.S_un.S_addr = inet_addr(ip);   //需要绑定到本地的哪个IP地址
	sockaddr_long.sin_port = htons(port_long);                          //需要监听的端口
	bind(socket_long, (SOCKADDR*)&sockaddr_long, sizeof(SOCKADDR));        //进行绑定动作

	listen(socket_long, 1);

	SOCKADDR clientAddr_long;
	size = sizeof(SOCKADDR);

	*socket_long_ptr = accept(socket_long, &clientAddr_long, &size);   //阻塞，直到有新tcp客户端连接      //阻塞，直到有新tcp客户端连接 
	//socket_heartbreak_ptr = &clientsocket_h264code;
	closesocket(socket_long);
	return 1;
}
