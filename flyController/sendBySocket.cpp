//发送给MATLAB
#include "SocketUtils.h"				//SOCKET工具
#include "sendBySocket.h"				
#include "globalVar.h"					//全局变量
#include <stdio.h>
#include "ElemType.h"					//队列元素

#if sendBySocket
	SOCKET socket_matlab;				
#endif
bool isAlive = true;					//控制线程结束的标志位
HANDLE imMutex;

//发送给MATLAB线程
#if sendBySocket
DWORD WINAPI sendImageInQueue(LPVOID lpParameter){
#if sendBySocket
	createShortConnection(MATLAB_IP, 30004, &socket_matlab);
	char buffer[MAXBYTE] = { 0 };		//通信缓存
	//recv(s, buffer, MAXBYTE, NULL);
	printf("***SERVER***%s", buffer);

#endif






	//char buffer[1000000];
	//int len;
	//while (isAlive){
	//	DWORD  dwWaitResult = WaitForSingleObject(		//WaitForSingleObject可以用来等待Event、Job、Memory resource notification、
	//		//Mutex、Process、Semaphore、Thread、Waitable timer等对象的接收
	//		imMutex,    // handle to mutex
	//		INFINITE);  // no time-out interval
	//	ElemType e;
	//	if (!W.DeQueue(&e)) {
	//		ReleaseMutex(imMutex);							//释放了互斥对象
	//		continue;
	//	}
	//	memcpy(buffer, e.data, e.len);
	//	len = e.len;
	//	freeElemType(e);
	//	ReleaseMutex(imMutex);							//释放了互斥对象
	//	//send(s, e.data, e.len, NULL);
	//	wrightBytes2Stream(s, buffer, len);
	//}
	return 1;
}
#endif

