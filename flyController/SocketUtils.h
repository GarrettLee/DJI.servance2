
#ifndef SOCKETUTILS_H
#define SOCKETUTILS_H
#include <winsock2.h>                   //socket头文件
#pragma comment (lib, "ws2_32")         //socket库文件


int wrightBytes2Stream(SOCKET s, char* data, unsigned int len);
int getHead(SOCKET s);
char* readByteFromStream(SOCKET s, char * out, int dataLength);
int createUTF8Byte(char *chinese_string, char *out, int out_len);
int encodeOneInt(char character, char pre, char next);
int createLongConnection(char* ip, int port, int port_long, SOCKET * socket_heartbreak, SOCKET * socket_long);
void createShortConnection(char* ip, int port, SOCKET * socket_short);
int decodeInts(int * int_array, int len_in, unsigned char * bytes, int start, int len);
#endif