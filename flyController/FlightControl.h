#ifndef FLIGHTCONTROL_H
#define FLIGHTCONTROL_H

#include <Windows.h>                    //为了方便调试，所以加入了等待2秒才进行连接server，这里用到了sleep函数
#include "DynaLnkQueue.hpp"
extern HANDLE ghMutex;
extern Queue<int> eventQueue;							//队列，按顺序存放等待发送的位图数据
DWORD WINAPI flightControlThread(LPVOID lpParameter);
DWORD WINAPI waitAndHandlePhoneEvent(LPVOID lpParameter);
#endif