#ifndef SENDBYSOCKET_H
#define SENDBYSOCKET_H
#include <Windows.h>                    //为了方便调试，所以加入了等待2秒才进行连接server，这里用到了sleep函数
extern HANDLE imMutex;
extern SOCKET socket_matlab;

DWORD WINAPI sendImageInQueue(LPVOID lpParameter);
#endif