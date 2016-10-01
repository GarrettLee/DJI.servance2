//SOCKET����
//����Լ��������֡��ʽΪ
/*
	�������������ө������ө������թ����Щ��������Щ�������
    ��START_FLAG�� ֡��1�� ֡��2�� ����֡/H264����֡    ��
	�������������۩������۩������ݩ����ة��������ة�������
	���п���֡�ĸ�ʽ��FlightControl.cpp��H264����֡����������֡��1��֡��2��ӳ�������֡/H264����֡�ĳ���
	getHead������ȡ����Ǯ�����ֽڣ�Ҳ����START_FLAG��֡��1��֡��2,
*/

#include <stdio.h>                      //�������롢��������ĵ���,printf, gets
#include "SocketUtils.h"



#define START_FLAG 0x11					//socket���ļ�ͷ�����Ǳ����Լ�Լ����һ���ļ�ͷ

//���ֽ�д��STREAM��ȥ
int wrightBytes2Stream(SOCKET s, char* data, unsigned int len) {
	if (len <= 0xffff){
		unsigned char head[3];
		//��TCP�����ݼӸ�����ͷ������Ϊ�ö����ݵĳ���
		head[0] = START_FLAG;
		head[1] = len & 0x00ff;
		head[2] = len >> 8;					//�����ݵĳ��Ȳ�������ֽڷ���ȥ

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

//�ӻ����л�ȡһ������ͷ�����ظ������ݵĳ���
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
		printf("�ļ�ͷ����");
		return -1;
	}
	return dataLength;
}

//���ݳ��ȴӻ�����ȡ������ȵ����ݣ����ȡ������ʱȡ�ù����������������򷵻���������ݣ����ص����������Ҫ����free����
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
		//һֱ���տͻ���socket��send����
		if (rev_num == 0){
			continue;
		}
		if (rev_num <= data_left){
			memcpy(frame_ptr, buffer, rev_num * sizeof(char));
			frame_ptr += rev_num * sizeof(char);
			data_left -= rev_num;
		}
		else if (rev_num > data_left){
			printf("ERROR����ȡ���������");
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


//�������ַ�ת��UTF8������ֽ���
int createUTF8Byte(char *chinese_string, char *out, int out_len){
	wchar_t wc[256];
	// ��ANSI����Ķ��ֽ��ַ���ת���ɿ��ַ��ַ���  
	int n = MultiByteToWideChar(CP_ACP, 0, chinese_string, strlen(chinese_string), wc, 256);
	if (n > 0)
	{
		wc[n] = 0;

		// �����ַ��ַ���ת����UTF-8����Ķ��ֽ��ַ���  
		n = WideCharToMultiByte(CP_UTF8, 0, wc, wcslen(wc), out, out_len, NULL, NULL);
		if (n > 0)
		{
			out[n] = 0;
		}
	}
	return n;
}

//���ֽ������з���һ�������ŵ�int16��
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

//����������
void createShortConnection(char* ip, int port, SOCKET * socket_short){
	WSADATA wsaData;
	sockaddr_in sockaddr;

	char cmd[] = "����ʼ������";
	char chinese_cmd[100];
	//socket1�������ѽ����õ���bitmapͼƬ����ȥ������ͷ��ر�������������
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	*socket_short = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	sockaddr.sin_family = PF_INET;
	sockaddr.sin_addr.S_un.S_addr = inet_addr(ip);
	sockaddr.sin_port = htons(port);

	connect(*socket_short, (SOCKADDR*)&sockaddr, sizeof(SOCKADDR));
}

//����������
int createLongConnection(char* ip, int port, int port_long, SOCKET * socket_heartbreak_ptr, SOCKET * socket_long_ptr){

	char cmd[] = "����ʼ������";		//���������ӵ�����
	char chinese_cmd[100];				//������Ž����������������õ���ָ��
	WSADATA wsaData_in;
	SOCKET s_in;
	sockaddr_in sockaddr_input;
	SOCKET clientsocket_h264code = *socket_long_ptr;


	WSADATA wsaData_long;
	SOCKET socket_long;
	sockaddr_in sockaddr_long;
	//socket2������Ϊ���ֻ��˵ĳ����ӵĽ������˳�����
	WSAStartup(MAKEWORD(2, 2), &wsaData_in);
	s_in = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_input.sin_family = PF_INET;
	sockaddr_input.sin_addr.S_un.S_addr = inet_addr(ip);   //��Ҫ�󶨵����ص��ĸ�IP��ַ
	sockaddr_input.sin_port = htons(port);                          //��Ҫ�����Ķ˿�
	bind(s_in, (SOCKADDR*)&sockaddr_input, sizeof(SOCKADDR));        //���а󶨶���

	listen(s_in, 1);

	SOCKADDR clientAddr;
	int size = sizeof(SOCKADDR);

	SOCKET clientsocket = *socket_heartbreak_ptr;

	//����ͨ�ţ�ȷ�Ͽ��Խ���������
	*socket_heartbreak_ptr = accept(s_in, &clientAddr, &size);               //������ֱ������tcp�ͻ�������
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
	if (strcmp(buffer_in, "��������") == 1){
		printf("��ʼ������");
	}
	free(buffer_in);

	int n = createUTF8Byte(cmd, chinese_cmd, 100);	//�������ַ���ת��UTF�����Ա㷢��

	wrightBytes2Stream(*socket_heartbreak_ptr, chinese_cmd, n);
	//socket3�������ӵ�socket������������Ƶ����
	WSAStartup(MAKEWORD(2, 2), &wsaData_long);
	socket_long = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_long.sin_family = PF_INET;
	sockaddr_long.sin_addr.S_un.S_addr = inet_addr(ip);   //��Ҫ�󶨵����ص��ĸ�IP��ַ
	sockaddr_long.sin_port = htons(port_long);                          //��Ҫ�����Ķ˿�
	bind(socket_long, (SOCKADDR*)&sockaddr_long, sizeof(SOCKADDR));        //���а󶨶���

	listen(socket_long, 1);

	SOCKADDR clientAddr_long;
	size = sizeof(SOCKADDR);

	*socket_long_ptr = accept(socket_long, &clientAddr_long, &size);   //������ֱ������tcp�ͻ�������      //������ֱ������tcp�ͻ������� 
	//socket_heartbreak_ptr = &clientsocket_h264code;
	closesocket(socket_long);
	return 1;
}
