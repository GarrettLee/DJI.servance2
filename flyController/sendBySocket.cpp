//���͸�MATLAB
#include "SocketUtils.h"				//SOCKET����
#include "sendBySocket.h"				
#include "globalVar.h"					//ȫ�ֱ���
#include <stdio.h>
#include "ElemType.h"					//����Ԫ��

#if sendBySocket
	SOCKET socket_matlab;				
#endif
bool isAlive = true;					//�����߳̽����ı�־λ
HANDLE imMutex;

//���͸�MATLAB�߳�
#if sendBySocket
DWORD WINAPI sendImageInQueue(LPVOID lpParameter){
#if sendBySocket
	createShortConnection(MATLAB_IP, 30004, &socket_matlab);
	char buffer[MAXBYTE] = { 0 };		//ͨ�Ż���
	//recv(s, buffer, MAXBYTE, NULL);
	printf("***SERVER***%s", buffer);

#endif






	//char buffer[1000000];
	//int len;
	//while (isAlive){
	//	DWORD  dwWaitResult = WaitForSingleObject(		//WaitForSingleObject���������ȴ�Event��Job��Memory resource notification��
	//		//Mutex��Process��Semaphore��Thread��Waitable timer�ȶ���Ľ���
	//		imMutex,    // handle to mutex
	//		INFINITE);  // no time-out interval
	//	ElemType e;
	//	if (!W.DeQueue(&e)) {
	//		ReleaseMutex(imMutex);							//�ͷ��˻������
	//		continue;
	//	}
	//	memcpy(buffer, e.data, e.len);
	//	len = e.len;
	//	freeElemType(e);
	//	ReleaseMutex(imMutex);							//�ͷ��˻������
	//	//send(s, e.data, e.len, NULL);
	//	wrightBytes2Stream(s, buffer, len);
	//}
	return 1;
}
#endif

