#ifndef FLIGHTCONTROL_H
#define FLIGHTCONTROL_H

#include <Windows.h>                    //Ϊ�˷�����ԣ����Լ����˵ȴ�2��Ž�������server�������õ���sleep����
#include "DynaLnkQueue.hpp"
extern HANDLE ghMutex;
extern Queue<int> eventQueue;							//���У���˳���ŵȴ����͵�λͼ����
DWORD WINAPI flightControlThread(LPVOID lpParameter);
DWORD WINAPI waitAndHandlePhoneEvent(LPVOID lpParameter);
#endif