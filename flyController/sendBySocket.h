#ifndef SENDBYSOCKET_H
#define SENDBYSOCKET_H
#include <Windows.h>                    //Ϊ�˷�����ԣ����Լ����˵ȴ�2��Ž�������server�������õ���sleep����
extern HANDLE imMutex;
extern SOCKET socket_matlab;

DWORD WINAPI sendImageInQueue(LPVOID lpParameter);
#endif