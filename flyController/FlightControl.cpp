//飞行控制线程
//控制帧格式：
//==============================================================================
//指令格式	{0, 1, 3, 1/2/3/4/5, -1/1, 0~255, 0~255, 0/1/2, 0~255, 0~255}
//	         │	│ │    │       │     │     │	  │      └──┴──只有在控制云台时有用，表示云台移动速度，占据两个字节。
//			 │	│ │    │       │     │     │    │
//			 │	│ │    │       │     │     │    └─────────只有在垂直移动时有效
//           │	│ │  	 │       │     │		│		                   0：垂直静止
//			 │	│ │	 │	      │	 │		│		                   1：垂直上升
//			 │	│ │	 │	      │	 │		│		                   2：垂直下降
//		 	 │ │ │    │       │     └── ┴────────────只有在前后移动/左右移动/水平旋转时有用，代表了移动的速度
//		   	 │ │ │    │       │							                  因为是个大于255的整数，在这里要占据两个字节。
//			 │ │ │    │       └───────────────────云台移动或者前后移动/左右移动/水平旋转的正负号
//			 │ │ │    └──────────────── ───────指示了动作的类型
//			 │ │ │                                                      1：垂直移动
//			 │ │ │                                                      2：前后移动
//			 │ │ │                                                      3：左右移动
//			 │ │ │                                                      4：水平旋转
//		     │ │ │                                                      5：云台上下转动
//		     └ ┴ ┴───────────────────────────帧头标志
//============================================================================
extern "C"{										//PID控制器
	#include "fuzzyPIDController.h"					
	#include "fuzzyPIDController_for_x_axis.h"
	#include <SDL2\SDL.h>
}
#include "SocketUtils.h"						//SOCKET工具
#include "FlightControl.h"						
#include "globalVar.h"							//全局变量
#include "sendBySocket.h"						//与MATLAB通信时需要获取相应的SOCKET
#include <stdio.h>
#include "H264Decoder.h"						//解码，在与MATLAB通信时需要通过对H264Decoder里面的相应变量进行操作以通知解码线程发送图片

#define NO_EVENT 0
#define NEW_TRACKING_TASK 1

Queue<int> eventQueue;							//队列，按顺序存放等待发送的位图数据
HANDLE ghMutex;									//互斥体

bool quit_falg = false;
int event_falg = NO_EVENT;

unsigned char  cmd2Matlab[15] = { 0 };

bool start_flag = false;

#if FLIGHT_CONTROL
SOCKET* clientsocket_heartbreak_flight_control;	
SOCKET* clientsocket_flight_control;				//用来承担接收来自手机端的h264编码的SOCKET
#endif

//飞行控制线程
#if FLIGHT_CONTROL
DWORD WINAPI flightControlThread(LPVOID lpParameter){

	//建立长连接
#if FLIGHT_CONTROL
	while (1){
		SOCKET s1;
		SOCKET s2;
		clientsocket_flight_control = &s1;
		clientsocket_heartbreak_flight_control = &s2;
		createLongConnection(SERVANCE_IP, 30002, 30003, clientsocket_heartbreak_flight_control, clientsocket_flight_control);
		start_flag = true;
#endif

		//如果不需要和MATLAB通信，直接通过键盘相应控制飞行器，否则与MATLAB通信
#if !sendBySocket
		int done = 0;						//退出循环的标志位
		char cmd[] = { 0, 1, 3, 1, 0, 0, 0, 1, 0, 0 };		//与飞行器通信的指令
		while (!done)
		{
			int  key;						//键值
			DWORD  dwWaitResult = WaitForSingleObject(		//等待互斥体被其他线程解除占用
				ghMutex,    // handle to mutex
				INFINITE);  // no time-out interval
			if (dwWaitResult == WAIT_ABANDONED) continue;
			if (eventQueue.DeQueue(&key) == false){			//从事件队列中获取键值
				ReleaseMutex(ghMutex);
				continue;
			}
			ReleaseMutex(ghMutex);
			
			//判断键值，并根据键值对命令进行修改
			switch (key){									
			case SDLK_DOWN:
				cmd[3] = 1;
				cmd[7] = 2;
				printf("↓");
				break;
			case SDLK_UP:
				cmd[3] = 1;
				cmd[7] = 1;
				printf("↑");
				break;
			case SDLK_LEFT:
				cmd[3] = 4;
				cmd[4] = -1;
				cmd[5] = 1000;
				cmd[6] = 1000 >> 8;
				printf("左旋");
				break;
			case SDLK_RIGHT:
				cmd[3] = 4;
				cmd[4] = 1;
				cmd[5] = 1000;
				cmd[6] = 1000 >> 8;
				printf("右旋");
				break;
			case SDLK_w:
				cmd[3] = 2;
				cmd[4] = 1;
				cmd[5] = 1000;
				cmd[6] = 1000 >> 8;
				printf("×");
				break;
			case SDLK_s:
				cmd[3] = 2;
				cmd[4] = -1;
				cmd[5] = 1000;
				cmd[6] = 1000 >> 8;
				printf("⊙");
				break;
			case SDLK_a:
				cmd[3] = 3;
				cmd[4] = -1;
				cmd[5] = 1000;
				cmd[6] = 1000 >> 8;
				printf("←");
				break;
			case SDLK_d:
				cmd[3] = 3;
				cmd[4] = 1;
				cmd[5] = 1000;
				cmd[6] = 1000 >> 8;
				printf("→");
				break;
			case SDLK_i:
				cmd[3] = 5;
				cmd[4] = 1;
				cmd[8] = 1000;
				cmd[9] = 1000 >> 8;
				printf("云台上旋");
				break;
			case SDLK_k:
				cmd[3] = 5;
				cmd[4] = -1;
				cmd[8] = 1000;
				cmd[9] = 1000 >> 8;
				printf("云台下旋");
				break;
			case SDLK_SPACE:
				cmd[3] = 5;
				cmd[4] = 1;
				cmd[8] = 0;
				cmd[9] = 0 >> 8;
				printf("云台停止");
				break;

			case SDL_KEYUP:
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
				continue;
				break;
			}

			wrightBytes2Stream(*clientsocket_flight_control, cmd, 10);//把命令发送给手机端

			//从手机端获取应答
			/*int cur_size = getHead(clientSocketLong);
			char * overflow = readByteFromStream(clientSocketLong, (char *)in_buffer, cur_size);
			if (overflow != NULL){
			free(overflow);
			return 0;
			}
			if ((in_buffer[0] == 0) && (in_buffer[1] == 1) && (in_buffer[2] == 4)){
			for (int i = 3; i <= 9; i++){
			if (in_buffer[i] != cmd[i]){
			return 0;
			}
			}
			}*/

			//Sleep(50);
		}

#else
//if (start_flag == false){
	//Sleep(200);
	//continue;
//}
int cursize = getHead(*clientsocket_flight_control);
char buffer[50] = { 0 };
char * overflow = readByteFromStream(*clientsocket_flight_control, buffer, cursize);

if (overflow != NULL){
	free(overflow);
	return 0;
}
if ((buffer[0] == 0)&(buffer[1] == 2)&(buffer[2] == 1)){
	event_falg = NEW_TRACKING_TASK;
	int x = encodeOneInt(1, buffer[3], buffer[4]) * output_w / encodeOneInt(1, buffer[11], buffer[12]);
	int y = encodeOneInt(1, buffer[5], buffer[6]) * output_h / encodeOneInt(1, buffer[13], buffer[14]);
	int w = encodeOneInt(1, buffer[7], buffer[8]) * output_w / encodeOneInt(1, buffer[11], buffer[12]);
	int h = encodeOneInt(1, buffer[9], buffer[10]) * output_h / encodeOneInt(1, buffer[13], buffer[14]);
	DWORD  dwWaitResult = WaitForSingleObject(		//WaitForSingleObject可以用来等待Event、Job、Memory resource notification、
		//Mutex、Process、Semaphore、Thread、Waitable timer等对象的接收
		rectMutex,    // handle to mutex
		100);  // no time-out interval
	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = h;
	ReleaseMutex(rectMutex);


	cmd2Matlab[0] = 0;
	cmd2Matlab[1] = 3;
	cmd2Matlab[2] = 2;
	int cordinate[4] = { x, y, w, h };
	decodeInts(cordinate, 4, cmd2Matlab, 3, 15);
	event_falg = NEW_TRACKING_TASK;
	Sleep(100);
	//break;
}
	if (event_falg == NO_EVENT){
		char u_cmd[] = { 0, 3, 1, NO_EVENT };
		wrightBytes2Stream(socket_matlab, u_cmd, 4);
	}
	if (event_falg == NEW_TRACKING_TASK){
char u_cmd[] = { 0, 3, 2 };
//wrightBytes2Stream(socket_matlab, u_cmd, 3);
		wrightBytes2Stream(socket_matlab, (char *)cmd2Matlab, 15);
		event_falg = NO_EVENT;
	}

		char in_buffer[2000] = { 0 };		//SOCKET通信的缓存

		int maxdx = 0;						//循环中PID输出的微分最大值
		int mindx = 0;						//循环中PID输出的微分最小值

		//PID控制器初始化
		fuzzyPIDController_initialize();
		fuzzyPIDController_for_x_axis_initialize();

		/*-------------云台垂直运动的PID参数-------------------*/
		I_PARAMETER_Y = 0.0294;
		P_PARAMETER_Y = 2.1;
		D_PARAMETER_Y = 4;
		//这两个参数设为0比设为以下数值的表现还要好，模糊pid的作用存疑
		KP_PARAMETER_Y = 0;//0.0267;
		KD_PARAMETER_Y = 0;// 0.0690;
		/*-----------------------------------------------------*/

		/*-------------飞行器水平旋转的PID参数-----------------*/
		I_PARAMETER_X = 0;
		P_PARAMETER_X = 1;
		D_PARAMETER_X = 0;
		KP_PARAMETER_X = 0;
		KD_PARAMETER_X = 0;
		/*-----------------------------------------------------*/

		while (1){

			//从MATLAB获取命令
			int cursize = getHead(socket_matlab);
			char cmd[20];
			char * overflow = readByteFromStream(socket_matlab, cmd, cursize);

			if (overflow != NULL){
				free(overflow);
				return 0;
			}

			//通过命令头的格式判断命令类型

			//if ((cmd[0] == 0)&(cmd[1] == 3)&(cmd[2] == 1)){
			//	if (event_falg == NO_EVENT){
			//		char u_cmd[] = { 0, 3, 1, NO_EVENT };
			//		wrightBytes2Stream(socket_matlab, u_cmd, 4);
			//	}
			//	if (event_falg == NEW_TRACKING_TASK){
					//char u_cmd[] = { 0, 3, 2 };
					//wrightBytes2Stream(socket_matlab, u_cmd, 3);
			//		wrightBytes2Stream(socket_matlab, (char *)cmd2Matlab, 15);
			//		event_falg = NO_EVENT;
			//	}
			//}

			/*--------------------------------------云台垂直运动的处理程序start-----------------------------------------------*/
			if ((cmd[0] == 0)&(cmd[1] == 1)&(cmd[2] == 5)){
				int error_y = cmd[3] * (int)(((unsigned char)cmd[4] & 0xff) + (((unsigned char)cmd[5] << 8) & 0xffff));//还原误差值
				CONTROLLER_INPUT_Y = error_y;			//向PID输入误差值
				fuzzyPIDController_step();				//PID执行

				//更新dx
				if (maxdx < FUZZY_DX_Y){
					maxdx = FUZZY_DX_Y;
				}
				if (mindx > FUZZY_DX_Y){
					mindx = FUZZY_DX_Y;
				}

				int controller_u = CONTROLLER_OUTPUT_Y;	//获取PID输出

				//判断输出的符号，并且求输出的绝对值
				char character = 1;						//PID输出的符号位
				if (controller_u < 0){
					character = -1;
					controller_u *= -1;
				}

				//往手机端发送控制指令
				char con_cmd[] = { 0, 1, 3, 5, character, 0, 0, 0, controller_u, controller_u >> 8 };
				wrightBytes2Stream(*clientsocket_flight_control, con_cmd, 10);

				//往MATLAB回送PID控制量
				char u_cmd[] = { 0, 1, 5, 0, 0, 0 };
				u_cmd[3] = character;
				u_cmd[4] = controller_u;
				u_cmd[5] = controller_u >> 8;
				wrightBytes2Stream(socket_matlab, u_cmd, 6);
				printf("控制量为：%d", controller_u);

				continue;
			}
			/*--------------------------------------云台垂直运动的处理程序end-----------------------------------------------*/

			/*--------------------------------------飞行器水平旋转的处理程序start-----------------------------------------------*/
			//与云台的处理程序类似，省略注释
			if ((cmd[0] == 0)&(cmd[1] == 1)&(cmd[2] == 7)){
				int error_x = cmd[3] * (int)(((unsigned char)cmd[4] & 0xff) + (((unsigned char)cmd[5] << 8) & 0xffff));
				CONTROLLER_INPUT_X = error_x;
				fuzzyPIDController_for_x_axis_step();

				//更新dx
				if (maxdx < FUZZY_DX_X){
					maxdx = FUZZY_DX_X;
				}
				if (mindx > FUZZY_DX_X){
					mindx = FUZZY_DX_X;
				}

				int controller_u = CONTROLLER_OUTPUT_X;

				char character = 1;
				if (controller_u < 0){
					character = -1;
					controller_u *= -1;
				}

				char u_cmd[] = { 0, 1, 5, 0, 0, 0 };
				u_cmd[3] = character;
				u_cmd[4] = controller_u;
				u_cmd[5] = controller_u >> 8;
				wrightBytes2Stream(socket_matlab, u_cmd, 6);
				printf("控制量为：%d", controller_u);
				char con_cmd[] = { 0, 1, 3, 4, character, controller_u, controller_u >> 8, 0, 0, 0 };
				wrightBytes2Stream(*clientsocket_flight_control, con_cmd, 10);
				continue;
			}
			if ((cmd[0] == 0)&(cmd[1] == 1)&(cmd[2] == 4)){
				send2Matlab = true;
				continue;
			}
			/*--------------------------------------飞行器水平旋转的处理程序end-----------------------------------------------*/


			/*--------------------------------------处理MATLAB请求照片信号start-----------------------------------------------*/
			//当接收到此信号时，说明MATLAB已经准备好接收下一帧图片了，通过修改send2Matlab来通知解码程序发送图片
			if ((cmd[0] == 0)&(cmd[1] == 1)&(cmd[2] == 4)){
				send2Matlab = true;
				continue;
			}
			/*--------------------------------------处理MATLAB请求照片信号end-----------------------------------------------*/

			/*--------------------------------------从MATLAB获取目标位置start-----------------------------------------------*/
			if ((cmd[0] == 0)&(cmd[1] == 1) & (cmd[2] == 6)){
				DWORD  dwWaitResult = WaitForSingleObject(		//WaitForSingleObject可以用来等待Event、Job、Memory resource notification、
					//Mutex、Process、Semaphore、Thread、Waitable timer等对象的接收
					rectMutex,    // handle to mutex
					100);  // no time-out interval
				rect.x = encodeOneInt(cmd[3], cmd[4], cmd[5]);
				rect.y = encodeOneInt(cmd[6], cmd[7], cmd[8]);
				rect.w = encodeOneInt(cmd[9], cmd[10], cmd[11]);
				rect.h = encodeOneInt(cmd[12], cmd[13], cmd[14]);
				ReleaseMutex(rectMutex);
			}
			wrightBytes2Stream(*clientsocket_flight_control, cmd, cursize);	//把框发送给手机端？
			/*--------------------------------------飞行器水平旋转的处理程序end-----------------------------------------------*/

			/*int cur_size = getHead(clientSocketLong);
			overflow = readByteFromStream(clientSocketLong, (char *)in_buffer, cur_size);
			if (overflow != NULL){
			free(overflow);
			return 0;
			}
			if ((in_buffer[0] == 0) && (in_buffer[1] == 1) && (in_buffer[2] == 4)){
			for (int i = 3; i <= 9; i++){
			if (in_buffer[i] != cmd[i]){
			return 0;
			}
			}
			}*/

		}

		//结束循环，关闭PID
		fuzzyPIDController_terminate();
		fuzzyPIDController_for_x_axis_terminate();
#endif
		closesocket(*clientsocket_flight_control);
		closesocket(*clientsocket_heartbreak_flight_control);
		clientsocket_flight_control = 0;
		clientsocket_heartbreak_flight_control = 0;
		WSACleanup();
		Sleep(200);
		//return (0);
	}
	
	eventQueue.DestroyQueue();		//清理队列

}

#endif

DWORD WINAPI waitAndHandlePhoneEvent(LPVOID lpParameter){
	while (1){
		
	}


}

