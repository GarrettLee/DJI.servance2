//���п����߳�
//����֡��ʽ��
//==============================================================================
//ָ���ʽ	{0, 1, 3, 1/2/3/4/5, -1/1, 0~255, 0~255, 0/1/2, 0~255, 0~255}
//	         ��	�� ��    ��       ��     ��     ��	  ��      �������ة���ֻ���ڿ�����̨ʱ���ã���ʾ��̨�ƶ��ٶȣ�ռ�������ֽڡ�
//			 ��	�� ��    ��       ��     ��     ��    ��
//			 ��	�� ��    ��       ��     ��     ��    ��������������������ֻ���ڴ�ֱ�ƶ�ʱ��Ч
//           ��	�� ��  	 ��       ��     ��		��		                   0����ֱ��ֹ
//			 ��	�� ��	 ��	      ��	 ��		��		                   1����ֱ����
//			 ��	�� ��	 ��	      ��	 ��		��		                   2����ֱ�½�
//		 	 �� �� ��    ��       ��     ������ �ة�����������������������ֻ����ǰ���ƶ�/�����ƶ�/ˮƽ��תʱ���ã��������ƶ����ٶ�
//		   	 �� �� ��    ��       ��							                  ��Ϊ�Ǹ�����255��������������Ҫռ�������ֽڡ�
//			 �� �� ��    ��       ������������������������������������������̨�ƶ�����ǰ���ƶ�/�����ƶ�/ˮƽ��ת��������
//			 �� �� ��    ���������������������������������� ��������������ָʾ�˶���������
//			 �� �� ��                                                      1����ֱ�ƶ�
//			 �� �� ��                                                      2��ǰ���ƶ�
//			 �� �� ��                                                      3�������ƶ�
//			 �� �� ��                                                      4��ˮƽ��ת
//		     �� �� ��                                                      5����̨����ת��
//		     �� �� �ة�����������������������������������������������������֡ͷ��־
//============================================================================
extern "C"{										//PID������
	#include "fuzzyPIDController.h"					
	#include "fuzzyPIDController_for_x_axis.h"
	#include <SDL2\SDL.h>
}
#include "SocketUtils.h"						//SOCKET����
#include "FlightControl.h"						
#include "globalVar.h"							//ȫ�ֱ���
#include "sendBySocket.h"						//��MATLABͨ��ʱ��Ҫ��ȡ��Ӧ��SOCKET
#include <stdio.h>
#include "H264Decoder.h"						//���룬����MATLABͨ��ʱ��Ҫͨ����H264Decoder�������Ӧ�������в�����֪ͨ�����̷߳���ͼƬ

#define NO_EVENT 0
#define NEW_TRACKING_TASK 1

Queue<int> eventQueue;							//���У���˳���ŵȴ����͵�λͼ����
HANDLE ghMutex;									//������

bool quit_falg = false;
int event_falg = NO_EVENT;

unsigned char  cmd2Matlab[15] = { 0 };

bool start_flag = false;

#if FLIGHT_CONTROL
SOCKET* clientsocket_heartbreak_flight_control;	
SOCKET* clientsocket_flight_control;				//�����е����������ֻ��˵�h264�����SOCKET
#endif

//���п����߳�
#if FLIGHT_CONTROL
DWORD WINAPI flightControlThread(LPVOID lpParameter){

	//����������
#if FLIGHT_CONTROL
	while (1){
		SOCKET s1;
		SOCKET s2;
		clientsocket_flight_control = &s1;
		clientsocket_heartbreak_flight_control = &s2;
		createLongConnection(SERVANCE_IP, 30002, 30003, clientsocket_heartbreak_flight_control, clientsocket_flight_control);
		start_flag = true;
#endif

		//�������Ҫ��MATLABͨ�ţ�ֱ��ͨ��������Ӧ���Ʒ�������������MATLABͨ��
#if !sendBySocket
		int done = 0;						//�˳�ѭ���ı�־λ
		char cmd[] = { 0, 1, 3, 1, 0, 0, 0, 1, 0, 0 };		//�������ͨ�ŵ�ָ��
		while (!done)
		{
			int  key;						//��ֵ
			DWORD  dwWaitResult = WaitForSingleObject(		//�ȴ������屻�����߳̽��ռ��
				ghMutex,    // handle to mutex
				INFINITE);  // no time-out interval
			if (dwWaitResult == WAIT_ABANDONED) continue;
			if (eventQueue.DeQueue(&key) == false){			//���¼������л�ȡ��ֵ
				ReleaseMutex(ghMutex);
				continue;
			}
			ReleaseMutex(ghMutex);
			
			//�жϼ�ֵ�������ݼ�ֵ����������޸�
			switch (key){									
			case SDLK_DOWN:
				cmd[3] = 1;
				cmd[7] = 2;
				printf("��");
				break;
			case SDLK_UP:
				cmd[3] = 1;
				cmd[7] = 1;
				printf("��");
				break;
			case SDLK_LEFT:
				cmd[3] = 4;
				cmd[4] = -1;
				cmd[5] = 1000;
				cmd[6] = 1000 >> 8;
				printf("����");
				break;
			case SDLK_RIGHT:
				cmd[3] = 4;
				cmd[4] = 1;
				cmd[5] = 1000;
				cmd[6] = 1000 >> 8;
				printf("����");
				break;
			case SDLK_w:
				cmd[3] = 2;
				cmd[4] = 1;
				cmd[5] = 1000;
				cmd[6] = 1000 >> 8;
				printf("��");
				break;
			case SDLK_s:
				cmd[3] = 2;
				cmd[4] = -1;
				cmd[5] = 1000;
				cmd[6] = 1000 >> 8;
				printf("��");
				break;
			case SDLK_a:
				cmd[3] = 3;
				cmd[4] = -1;
				cmd[5] = 1000;
				cmd[6] = 1000 >> 8;
				printf("��");
				break;
			case SDLK_d:
				cmd[3] = 3;
				cmd[4] = 1;
				cmd[5] = 1000;
				cmd[6] = 1000 >> 8;
				printf("��");
				break;
			case SDLK_i:
				cmd[3] = 5;
				cmd[4] = 1;
				cmd[8] = 1000;
				cmd[9] = 1000 >> 8;
				printf("��̨����");
				break;
			case SDLK_k:
				cmd[3] = 5;
				cmd[4] = -1;
				cmd[8] = 1000;
				cmd[9] = 1000 >> 8;
				printf("��̨����");
				break;
			case SDLK_SPACE:
				cmd[3] = 5;
				cmd[4] = 1;
				cmd[8] = 0;
				cmd[9] = 0 >> 8;
				printf("��ֹ̨ͣ");
				break;

			case SDL_KEYUP:
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
				continue;
				break;
			}

			wrightBytes2Stream(*clientsocket_flight_control, cmd, 10);//������͸��ֻ���

			//���ֻ��˻�ȡӦ��
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
	DWORD  dwWaitResult = WaitForSingleObject(		//WaitForSingleObject���������ȴ�Event��Job��Memory resource notification��
		//Mutex��Process��Semaphore��Thread��Waitable timer�ȶ���Ľ���
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

		char in_buffer[2000] = { 0 };		//SOCKETͨ�ŵĻ���

		int maxdx = 0;						//ѭ����PID�����΢�����ֵ
		int mindx = 0;						//ѭ����PID�����΢����Сֵ

		//PID��������ʼ��
		fuzzyPIDController_initialize();
		fuzzyPIDController_for_x_axis_initialize();

		/*-------------��̨��ֱ�˶���PID����-------------------*/
		I_PARAMETER_Y = 0.0294;
		P_PARAMETER_Y = 2.1;
		D_PARAMETER_Y = 4;
		//������������Ϊ0����Ϊ������ֵ�ı��ֻ�Ҫ�ã�ģ��pid�����ô���
		KP_PARAMETER_Y = 0;//0.0267;
		KD_PARAMETER_Y = 0;// 0.0690;
		/*-----------------------------------------------------*/

		/*-------------������ˮƽ��ת��PID����-----------------*/
		I_PARAMETER_X = 0;
		P_PARAMETER_X = 1;
		D_PARAMETER_X = 0;
		KP_PARAMETER_X = 0;
		KD_PARAMETER_X = 0;
		/*-----------------------------------------------------*/

		while (1){

			//��MATLAB��ȡ����
			int cursize = getHead(socket_matlab);
			char cmd[20];
			char * overflow = readByteFromStream(socket_matlab, cmd, cursize);

			if (overflow != NULL){
				free(overflow);
				return 0;
			}

			//ͨ������ͷ�ĸ�ʽ�ж���������

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

			/*--------------------------------------��̨��ֱ�˶��Ĵ������start-----------------------------------------------*/
			if ((cmd[0] == 0)&(cmd[1] == 1)&(cmd[2] == 5)){
				int error_y = cmd[3] * (int)(((unsigned char)cmd[4] & 0xff) + (((unsigned char)cmd[5] << 8) & 0xffff));//��ԭ���ֵ
				CONTROLLER_INPUT_Y = error_y;			//��PID�������ֵ
				fuzzyPIDController_step();				//PIDִ��

				//����dx
				if (maxdx < FUZZY_DX_Y){
					maxdx = FUZZY_DX_Y;
				}
				if (mindx > FUZZY_DX_Y){
					mindx = FUZZY_DX_Y;
				}

				int controller_u = CONTROLLER_OUTPUT_Y;	//��ȡPID���

				//�ж�����ķ��ţ�����������ľ���ֵ
				char character = 1;						//PID����ķ���λ
				if (controller_u < 0){
					character = -1;
					controller_u *= -1;
				}

				//���ֻ��˷��Ϳ���ָ��
				char con_cmd[] = { 0, 1, 3, 5, character, 0, 0, 0, controller_u, controller_u >> 8 };
				wrightBytes2Stream(*clientsocket_flight_control, con_cmd, 10);

				//��MATLAB����PID������
				char u_cmd[] = { 0, 1, 5, 0, 0, 0 };
				u_cmd[3] = character;
				u_cmd[4] = controller_u;
				u_cmd[5] = controller_u >> 8;
				wrightBytes2Stream(socket_matlab, u_cmd, 6);
				printf("������Ϊ��%d", controller_u);

				continue;
			}
			/*--------------------------------------��̨��ֱ�˶��Ĵ������end-----------------------------------------------*/

			/*--------------------------------------������ˮƽ��ת�Ĵ������start-----------------------------------------------*/
			//����̨�Ĵ���������ƣ�ʡ��ע��
			if ((cmd[0] == 0)&(cmd[1] == 1)&(cmd[2] == 7)){
				int error_x = cmd[3] * (int)(((unsigned char)cmd[4] & 0xff) + (((unsigned char)cmd[5] << 8) & 0xffff));
				CONTROLLER_INPUT_X = error_x;
				fuzzyPIDController_for_x_axis_step();

				//����dx
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
				printf("������Ϊ��%d", controller_u);
				char con_cmd[] = { 0, 1, 3, 4, character, controller_u, controller_u >> 8, 0, 0, 0 };
				wrightBytes2Stream(*clientsocket_flight_control, con_cmd, 10);
				continue;
			}
			if ((cmd[0] == 0)&(cmd[1] == 1)&(cmd[2] == 4)){
				send2Matlab = true;
				continue;
			}
			/*--------------------------------------������ˮƽ��ת�Ĵ������end-----------------------------------------------*/


			/*--------------------------------------����MATLAB������Ƭ�ź�start-----------------------------------------------*/
			//�����յ����ź�ʱ��˵��MATLAB�Ѿ�׼���ý�����һ֡ͼƬ�ˣ�ͨ���޸�send2Matlab��֪ͨ���������ͼƬ
			if ((cmd[0] == 0)&(cmd[1] == 1)&(cmd[2] == 4)){
				send2Matlab = true;
				continue;
			}
			/*--------------------------------------����MATLAB������Ƭ�ź�end-----------------------------------------------*/

			/*--------------------------------------��MATLAB��ȡĿ��λ��start-----------------------------------------------*/
			if ((cmd[0] == 0)&(cmd[1] == 1) & (cmd[2] == 6)){
				DWORD  dwWaitResult = WaitForSingleObject(		//WaitForSingleObject���������ȴ�Event��Job��Memory resource notification��
					//Mutex��Process��Semaphore��Thread��Waitable timer�ȶ���Ľ���
					rectMutex,    // handle to mutex
					100);  // no time-out interval
				rect.x = encodeOneInt(cmd[3], cmd[4], cmd[5]);
				rect.y = encodeOneInt(cmd[6], cmd[7], cmd[8]);
				rect.w = encodeOneInt(cmd[9], cmd[10], cmd[11]);
				rect.h = encodeOneInt(cmd[12], cmd[13], cmd[14]);
				ReleaseMutex(rectMutex);
			}
			wrightBytes2Stream(*clientsocket_flight_control, cmd, cursize);	//�ѿ��͸��ֻ��ˣ�
			/*--------------------------------------������ˮƽ��ת�Ĵ������end-----------------------------------------------*/

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

		//����ѭ�����ر�PID
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
	
	eventQueue.DestroyQueue();		//�������

}

#endif

DWORD WINAPI waitAndHandlePhoneEvent(LPVOID lpParameter){
	while (1){
		
	}


}

