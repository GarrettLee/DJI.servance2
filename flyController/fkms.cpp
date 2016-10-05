//主函数入口！！


#include "fkms.h"
#include <stdio.h>                      //用于输入、输出函数的调用,printf, gets
#include <iostream>						//用来做输入输出（std::cout<<String<<std::endl）
#include "H264Decoder.h"				//包含解码线程函数以及相关的变量
#include "FlightControl.h"				//包含飞行控制线程函数以及相关变量
#include "globalVar.h"					//定义了一些全局的宏定义
#include "sendBySocket.h"				//包含与MATLAB进行通信的线程函数和相关变量，目前没有实用


bool eventQueueBeingUsed = false;		//当有代码正被占用eventQueue时置为真，目前没有使用


/*退出程序之前先清理 SDL subsystem */
static void
quit(int rc)
{
	SDL_Quit();
	exit(rc);
}




int main(int argc, char* argv[]){

	
	eventQueue = Queue<int>();//一个用来存放待发送到MATLAB上的图片的队列，目前没有使用

	//互斥体的初始化
	ghMutex = CreateMutex(			//找出当前系统是否已经存在指定进程的实例。如果没有则创建一个互斥体
		NULL,              // default security attributes
		FALSE,             // initially not owned
		NULL);             // unnamed mutex;
	imMutex = CreateMutex(			//找出当前系统是否已经存在指定进程的实例。如果没有则创建一个互斥体
		NULL,              // default security attributes
		FALSE,             // initially not owned
		NULL);             // unnamed mutex;
	rectMutex = CreateMutex(			//找出当前系统是否已经存在指定进程的实例。如果没有则创建一个互斥体
		NULL,              // default security attributes
		FALSE,             // initially not owned
		NULL);             // unnamed mutex;

	//创建SDLwindow

	/* 设置应用程序日志级别*/
	SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

	/* 初始化SDL子系统*/
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s\n", SDL_GetError());
		return (1);
	}

	/* 创建SDL 2d渲染窗口 */
	window = SDL_CreateWindow("CheckKeys Test",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,

		output_w, output_h, 0);
	//sdl 2d渲染窗口创建失败
	if (!window) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create 640x480 window: %s\n",
			SDL_GetError());
		quit(2);
	}

	//启动解码线程（解码线程同时有把图片发送给MATLAB的功能）
	HANDLE handleDecode = CreateThread(NULL, 0, decodeImageThread, NULL, 0, NULL);
	if (NULL == handleDecode)
	{
		std::cout << "Create Thread failed !" << std::endl;
	}
	CloseHandle(handleDecode);

/*
#if sendBySocket
	//启动发送图片线程
	HANDLE handle;
	handle = CreateThread(NULL, 0, sendImageInQueue, NULL, 0, NULL);
	if (NULL == handle)
	{
		std::cout << "Create Thread failed !" << std::endl;
	}
	CloseHandle(handle);
#endif
	*/

//启动飞行控制和云台控制线程，该线程工作方式是不断地和MATLAB端交换数据，并且根据MATLAB发来的误差进行PID控制
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

	//不断检测键盘和鼠标，如果删除这一部分代码会出现SDL窗口无法移动的问题
	SDL_StartTextInput();//开始接受Unicode文本输入事件  如果支持的话将会显示屏幕键盘
	int done = 0;
	while (!done){
		//sdl事件处理
		SDL_Event event;
		/* 事件监测循环检测 事件 */
		SDL_PollEvent(&event);
		//event.type存储了当前的事件类型
		//如果无键盘鼠标 触摸点击 那么 默认是 0x200 
		switch (event.type)
		{
		case SDL_KEYDOWN:{

			//为了保证多线程之间的资源共享不会混乱，这里用了Mutex来做同步锁
			//这里的队列eventQueue是一个保存了键值的队列
			DWORD  dwWaitResult = WaitForSingleObject(		//WaitForSingleObject可以用来等待Event、Job、Memory resource notification、
				//Mutex、Process、Semaphore、Thread、Waitable timer等对象的接收
				ghMutex,    // handle to mutex
				INFINITE);  // no time-out interval
			if (dwWaitResult == WAIT_ABANDONED) continue;
			if (eventQueue.QueueLength() >= 2){
				eventQueue.DeQueue(NULL);
			}
			eventQueue.EnQueue(event.key.keysym.sym);
			ReleaseMutex(ghMutex);							//释放了互斥对象
			break;
		}
		case SDL_KEYUP:
			//打印系统键信息
			break;
		case SDL_TEXTINPUT:
			//如果是文本输入 输出文本
			break;
		case SDL_MOUSEBUTTONDOWN:
			/* 如果有任何鼠标点击消息 或者SDL_QUIT消息 那么将退出窗口 */
		case SDL_QUIT:
			done = 1;
			break;
		default:
			break;
		}

		event.key.keysym.sym = 0;

	}

	//等待解码线程结束
	WaitForSingleObject(handleDecode, INFINITE);
	//WaitForSingleObject(handleControl, INFINITE);
	//SDL_DestroyWindow(screen);
	SDL_Quit();
	return 0;
}
