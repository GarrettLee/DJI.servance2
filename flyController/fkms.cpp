//��������ڣ���


#include "fkms.h"
#include <stdio.h>                      //�������롢��������ĵ���,printf, gets
#include <iostream>						//���������������std::cout<<String<<std::endl��
#include "H264Decoder.h"				//���������̺߳����Լ���صı���
#include "FlightControl.h"				//�������п����̺߳����Լ���ر���
#include "globalVar.h"					//������һЩȫ�ֵĺ궨��
#include "sendBySocket.h"				//������MATLAB����ͨ�ŵ��̺߳�������ر�����Ŀǰû��ʵ��


bool eventQueueBeingUsed = false;		//���д�������ռ��eventQueueʱ��Ϊ�棬Ŀǰû��ʹ��


/*�˳�����֮ǰ������ SDL subsystem */
static void
quit(int rc)
{
	SDL_Quit();
	exit(rc);
}




int main(int argc, char* argv[]){

	
	eventQueue = Queue<int>();//һ��������Ŵ����͵�MATLAB�ϵ�ͼƬ�Ķ��У�Ŀǰû��ʹ��

	//������ĳ�ʼ��
	ghMutex = CreateMutex(			//�ҳ���ǰϵͳ�Ƿ��Ѿ�����ָ�����̵�ʵ�������û���򴴽�һ��������
		NULL,              // default security attributes
		FALSE,             // initially not owned
		NULL);             // unnamed mutex;
	imMutex = CreateMutex(			//�ҳ���ǰϵͳ�Ƿ��Ѿ�����ָ�����̵�ʵ�������û���򴴽�һ��������
		NULL,              // default security attributes
		FALSE,             // initially not owned
		NULL);             // unnamed mutex;
	rectMutex = CreateMutex(			//�ҳ���ǰϵͳ�Ƿ��Ѿ�����ָ�����̵�ʵ�������û���򴴽�һ��������
		NULL,              // default security attributes
		FALSE,             // initially not owned
		NULL);             // unnamed mutex;

	//����SDLwindow

	/* ����Ӧ�ó�����־����*/
	SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

	/* ��ʼ��SDL��ϵͳ*/
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s\n", SDL_GetError());
		return (1);
	}

	/* ����SDL 2d��Ⱦ���� */
	window = SDL_CreateWindow("CheckKeys Test",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,

		output_w, output_h, 0);
	//sdl 2d��Ⱦ���ڴ���ʧ��
	if (!window) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create 640x480 window: %s\n",
			SDL_GetError());
		quit(2);
	}

	//���������̣߳������߳�ͬʱ�а�ͼƬ���͸�MATLAB�Ĺ��ܣ�
	HANDLE handleDecode = CreateThread(NULL, 0, decodeImageThread, NULL, 0, NULL);
	if (NULL == handleDecode)
	{
		std::cout << "Create Thread failed !" << std::endl;
	}
	CloseHandle(handleDecode);

/*
#if sendBySocket
	//��������ͼƬ�߳�
	HANDLE handle;
	handle = CreateThread(NULL, 0, sendImageInQueue, NULL, 0, NULL);
	if (NULL == handle)
	{
		std::cout << "Create Thread failed !" << std::endl;
	}
	CloseHandle(handle);
#endif
	*/

//�������п��ƺ���̨�����̣߳����̹߳�����ʽ�ǲ��ϵغ�MATLAB�˽������ݣ����Ҹ���MATLAB������������PID����
//#if FLIGHT_CONTROL
	HANDLE handleControl = CreateThread(NULL, 0, flightControlThread, NULL, 0, NULL);
	if (NULL == handleControl)
	{
		std::cout << "Create Thread failed !" << std::endl;
	}
	CloseHandle(handleControl);
//#endif

#if sendBySocket
	HANDLE handlesend = CreateThread(NULL, 0, sendImageInQueue, NULL, 0, NULL);
	if (NULL == handlesend)
	{
		std::cout << "Create Thread failed !" << std::endl;
	}
	CloseHandle(handlesend);

	HANDLE handlewaitforphone = CreateThread(NULL, 0, waitAndHandlePhoneEvent, NULL, 0, NULL);
	if (NULL == handlewaitforphone)
	{
		std::cout << "Create Thread failed !" << std::endl;
	}
	CloseHandle(handlewaitforphone);
#endif

	//���ϼ����̺���꣬���ɾ����һ���ִ�������SDL�����޷��ƶ�������
	SDL_StartTextInput();//��ʼ����Unicode�ı������¼�  ���֧�ֵĻ�������ʾ��Ļ����
	int done = 0;
	while (!done){
		//sdl�¼�����
		SDL_Event event;
		/* �¼����ѭ����� �¼� */
		SDL_PollEvent(&event);
		//event.type�洢�˵�ǰ���¼�����
		//����޼������ ������� ��ô Ĭ���� 0x200 
		switch (event.type)
		{
		case SDL_KEYDOWN:{

			//Ϊ�˱�֤���߳�֮�����Դ��������ң���������Mutex����ͬ����
			//����Ķ���eventQueue��һ�������˼�ֵ�Ķ���
			DWORD  dwWaitResult = WaitForSingleObject(		//WaitForSingleObject���������ȴ�Event��Job��Memory resource notification��
				//Mutex��Process��Semaphore��Thread��Waitable timer�ȶ���Ľ���
				ghMutex,    // handle to mutex
				INFINITE);  // no time-out interval
			if (dwWaitResult == WAIT_ABANDONED) continue;
			if (eventQueue.QueueLength() >= 2){
				eventQueue.DeQueue(NULL);
			}
			eventQueue.EnQueue(event.key.keysym.sym);
			ReleaseMutex(ghMutex);							//�ͷ��˻������
			break;
		}
		case SDL_KEYUP:
			//��ӡϵͳ����Ϣ
			break;
		case SDL_TEXTINPUT:
			//������ı����� ����ı�
			break;
		case SDL_MOUSEBUTTONDOWN:
			/* ������κ��������Ϣ ����SDL_QUIT��Ϣ ��ô���˳����� */
		case SDL_QUIT:
			done = 1;
			break;
		default:
			break;
		}

		event.key.keysym.sym = 0;

	}

	//�ȴ������߳̽���
	WaitForSingleObject(handleDecode, INFINITE);
	//WaitForSingleObject(handleControl, INFINITE);
	//SDL_DestroyWindow(screen);
	SDL_Quit();
	return 0;
}
