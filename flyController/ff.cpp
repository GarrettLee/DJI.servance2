#include "ff.h"
#include <stdio.h>                      //�������롢��������ĵ���,printf, gets
#include <winsock2.h>                   //socketͷ�ļ�
#include <Windows.h>                    //Ϊ�˷�����ԣ����Լ����˵ȴ�2��Ž�������server�������õ���sleep����
#include <iostream>						//���������������std::cout<<String<<std::endl��
#include <time.h>						//��ȡϵͳʱ�䣬����ʱ���
#include "DynaLnkQueue.hpp"				//����

#pragma comment (lib, "ws2_32")         //socket���ļ�
#define sendBySocket 0					//Ϊ1ʱ������socket1���ѽ�����bitmapͼƬ���͵�����������Ӧ�ã�
#define ENABLE_WHOLENESS_CHECK 0;		//Ϊ1ʱ���������������Լ�飬����Ĭ����Ϊ������������
#define DECODE_NATIVE_CODE 0			//Ϊ1ʱ����ĿĿ¼�¶�ȡ��Ϊ�����õı���h.264�ļ������룬�������ܷ���������ĵ���
#define FLIGHT_CONTROL 0				//Ϊ1ʱ�����ô󽮵ķ��п���

//���뺯��decodeandDisplay���ܵ�������������ⲿ������н������˽⵽�����Ƿ����
#define ERROR_SIZE_0 0
#define FIRST_FRAME_DECODED 1
#define BUFFER_DECODED 2
#define ERROR_DECODEING 3

//using namespace std;

AVPixelFormat output_format = PIX_FMT_GRAY8; //���ͼ���ʽ��һ��ΪPIX_FMT_RGB24����PIX_FMT_GRAY8

//���߳�
DWORD WINAPI sendImageInQueue(LPVOID lpParameter);
DWORD WINAPI decodeImageThread(LPVOID lpParameter);
DWORD WINAPI flightControlThread(LPVOID lpParameter);

HANDLE ghMutex;
HANDLE imMutex;

Queue<ElemType> W;							//���У���˳���ŵȴ����͵�λͼ����
Queue<int> eventQueue;					//���У���ż����¼�
bool eventQueueBeingUsed = false;		//���д�������ռ��eventQueueʱ��Ϊ��

bool isAlive = true;					//�����߳̽����ı�־λ
bool send2Matlab = false;				//��Ϊ��ʱ����ͼ���MATLAB��

SDL_Window *window;



#define START_FLAG 0x11					//socket���ļ�ͷ�����Ǳ����Լ�Լ����һ���ļ�ͷ
//SDL---------------------------
int screen_w = 480, screen_h = 272;
int output_w = 640, output_h = 320;		//�����ͼƬ�Ĵ�С
SDL_Window *screen;						//SDL_Window����ʹ��SDL��ʱ�򵯳����Ǹ�����
SDL_Renderer* sdlRenderer;				//SDL_Renderer������ȾSDL_Texture��SDL_Window
SDL_Texture* sdlTexture;				//SDL_Texture������ʾYUV���ݡ�һ��SDL_Texture��Ӧһ֡YUV����
SDL_Rect sdlRect;						//SDL_Rect����ȷ��SDL_Texture��ʾ��λ��

//SDL End----------------------
struct SwsContext *img_convert_ctx;
AVCodec *pCodec;                         //ÿ�ֽ���������Ӧһ��AVCodec�ṹ
AVCodecContext *pCodecCtx = NULL;        //ÿ��AVCodecContext�ж�Ӧһ��AVCodec
AVCodecParserContext *pCodecParserCtx = NULL;
int frame_count;
FILE *fp_in;							//����������ļ���
FILE *fp_head;							//����Ĳ���
AVFrame	*pFrame, *pFrameYUV;			//���������
uint8_t *out_buffer;
const long in_buffer_size = 50000;
uint8_t in_buffer[in_buffer_size + FF_INPUT_BUFFER_PADDING_SIZE] = { 0 };
uint8_t *cur_ptr;
int cur_size;
int first_time = 1;

AVPacket packet;						//����ǰ����,ʹ��AVPacket���ݴ�⸴��֮�󡢽���֮ǰ��ý�����ݣ�һ����/��Ƶ֡��һ����Ļ���ȣ���������Ϣ������ʱ�������ʾʱ�����ʱ���ȣ�
int ret = 0;
int got_picture = 0;

int y_size;

const AVCodecID codec_id = AV_CODEC_ID_H264;
const char filepath_in[] = "bigbuckbunny_480x272.h265";//bigbuckbunny_480x272.h264Ϊ�����ļ�
const char filepath_head[] = "myData.h264";//���������I֡

int wholeness_check = true;			//�����Լ���־����Ϊ��ʱ˵��������ļ�����������Ҫ����I֡

int wrightBytes2Stream(SOCKET s, char* data, unsigned int len) {
	// add buffered writer

	if (len <= 0xffff){
		uint8_t head[3];
		//��TCP�����ݼӸ�����ͷ������Ϊ�ö����ݵĳ���
		head[0] = START_FLAG;
		head[1] = len & 0x00ff;
		head[2] = len >> 8;					//�����ݵĳ��Ȳ�������ֽڷ���ȥ

		if ((head[1] == 0)&(head[2] == 0)){
			return -1;
		}
		if (send(s, (char *)head, 3, NULL) <= 0)
			return -1;
		int num = send(s, data, len, NULL);
		if (num <= 0)
			return -1;
	}
	else{
		int remain = len;
		int package_num = len / 0xffff + 1;
		char cmd[] = "big data";
		wrightBytes2Stream(s, cmd, sizeof(cmd) / sizeof(char));
		char num[] = { package_num };
		wrightBytes2Stream(s, num, 1);
		int j = 0;
		for (int i = 0; i < package_num - 1; i++){
			if (wrightBytes2Stream(s, data, 0xffff) == -1){
				i--;
				continue;
			}
			data = data + 0xffff;
			remain -= 0xffff;
			//Sleep(0);
		}
		wrightBytes2Stream(s, data, remain);
	}
	return 1;
}

//�ӻ����л�ȡһ������ͷ�����ظ������ݵĳ���
int getHead(SOCKET s){
	int dataLength = 0;
	uint8_t start[3] = { 0 };
	recv(s, (char *)start, 3, NULL);
	if (start[0] == START_FLAG){
		dataLength = (start[1] & (int)0xff) + ((start[2] << 8)&(int)0xffff);
	}
	else{
		printf("�ļ�ͷ����");
		return -1;
	}
	return dataLength;
}

//���ݳ��ȴӻ�����ȡ������ȵ����ݣ����ȡ������ʱȡ�ù����������������򷵻���������ݣ����ص����������Ҫ����free����
char* readByteFromStream(SOCKET s, char * out, int dataLength) {

	int data_left = dataLength;
	char * frame_ptr = out;
	char * overflow = NULL;
	while (data_left > 0)
	{
		char buffer[10000] = { 0 };
		if (dataLength > 10000){
			return NULL;
		}
		int rev_num = recv(s, buffer, data_left, NULL);
		//һֱ���տͻ���socket��send����
		if (rev_num == 0){
			continue;
		}
		if (rev_num <= data_left){
			memcpy(frame_ptr, buffer, rev_num * sizeof(char));
			frame_ptr += rev_num * sizeof(char);
			data_left -= rev_num;
		}
		else if (rev_num > data_left){
			printf("ERROR����ȡ���������");
			int overflow_num = rev_num - data_left;
			memcpy(frame_ptr, buffer, data_left * sizeof(char));
			overflow = (char *)malloc(overflow_num * sizeof(char));
			char * start_of_overflow = buffer + data_left;
			memcpy(overflow, start_of_overflow, overflow_num * sizeof(char));
			data_left = 0;
		}
	}
	return overflow;
}


//�������ַ�ת��UTF8������ֽ���
int createUTF8Byte(char *chinese_string, char *out, int out_len){
	wchar_t wc[256];
	// ��ANSI����Ķ��ֽ��ַ���ת���ɿ��ַ��ַ���  
	int n = MultiByteToWideChar(CP_ACP, 0, chinese_string, strlen(chinese_string), wc, 256);
	if (n > 0)
	{
		wc[n] = 0;

		// �����ַ��ַ���ת����UTF-8����Ķ��ֽ��ַ���  
		n = WideCharToMultiByte(CP_UTF8, 0, wc, wcslen(wc), out, out_len, NULL, NULL);
		if (n > 0)
		{
			out[n] = 0;
		}
	}
	return n;
}

#if sendBySocket
WSADATA wsaData;
SOCKET s;
sockaddr_in sockaddr;
#endif

#if !DECODE_NATIVE_CODE
WSADATA wsaData_in;
SOCKET s_in;
sockaddr_in sockaddr_input;


WSADATA wsaData_long;
SOCKET socket_long;
sockaddr_in sockaddr_long;

SOCKET clientsocket_long;
#endif

#if FLIGHT_CONTROL
WSADATA wsaDataControl;
SOCKET sControl;
sockaddr_in sockaddrControl;
SOCKET clientSocketControl;


WSADATA wsaDataControlLong;
SOCKET sControlLong;
sockaddr_in sockaddrControlLong;
SOCKET clientSocketLong;

#endif

void socketInitial(){
	Sleep(2000);                        //��˯2��������server
	char cmd[] = "����ʼ������";
	char chinese_cmd[100];
#if sendBySocket
	//socket1�������ѽ����õ���bitmapͼƬ����ȥ������ͷ��ر�������������
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	sockaddr.sin_family = PF_INET;
	sockaddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	sockaddr.sin_port = htons(30004);

	connect(s, (SOCKADDR*)&sockaddr, sizeof(SOCKADDR));

	char buffer[MAXBYTE] = { 0 };
	//recv(s, buffer, MAXBYTE, NULL);
	printf("***SERVER***%s", buffer);


#endif

#if !DECODE_NATIVE_CODE
	//socket2������Ϊ���ֻ��˵ĳ����ӵĽ������˳�����
	WSAStartup(MAKEWORD(2, 2), &wsaData_in);
	s_in = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_input.sin_family = PF_INET;
	sockaddr_input.sin_addr.S_un.S_addr = inet_addr("172.20.10.9");   //��Ҫ�󶨵����ص��ĸ�IP��ַ
	sockaddr_input.sin_port = htons(30000);                          //��Ҫ�����Ķ˿�
	bind(s_in, (SOCKADDR*)&sockaddr_input, sizeof(SOCKADDR));        //���а󶨶���

	listen(s_in, 1);

	SOCKADDR clientAddr;
	int size = sizeof(SOCKADDR);

	SOCKET clientsocket;
	//����ͨ�ţ�ȷ�Ͽ��Խ���������
	clientsocket = accept(s_in, &clientAddr, &size);               //������ֱ������tcp�ͻ�������
	int len = getHead(clientsocket);
	char * buffer_in = (char *)malloc(len * sizeof(char));
	char * overflow = readByteFromStream(clientsocket, buffer_in, len);
	if (overflow != NULL){
		free(overflow);
		return;
	}
	if (strcmp(buffer_in, "��������") == 1){
		printf("��ʼ������");
	}
	free(buffer_in);

	int n = createUTF8Byte(cmd, chinese_cmd, 100);

	wrightBytes2Stream(clientsocket, chinese_cmd, n);
	//socket3�������ӵ�socket������������Ƶ����
	WSAStartup(MAKEWORD(2, 2), &wsaData_long);
	socket_long = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_long.sin_family = PF_INET;
	sockaddr_long.sin_addr.S_un.S_addr = inet_addr("172.20.10.9");   //��Ҫ�󶨵����ص��ĸ�IP��ַ
	sockaddr_long.sin_port = htons(30001);                          //��Ҫ�����Ķ˿�
	bind(socket_long, (SOCKADDR*)&sockaddr_long, sizeof(SOCKADDR));        //���а󶨶���

	listen(socket_long, 1);

	SOCKADDR clientAddr_long;
	size = sizeof(SOCKADDR);

	clientsocket_long = accept(socket_long, &clientAddr_long, &size);               //������ֱ������tcp�ͻ�������
#endif

#if FLIGHT_CONTROL

	//socket2������Ϊ���ֻ��˵ĳ����ӵĽ������˳�����
	WSAStartup(MAKEWORD(2, 2), &wsaDataControl);
	sControl = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddrControl.sin_family = PF_INET;
	sockaddrControl.sin_addr.S_un.S_addr = inet_addr("172.20.10.9");   //��Ҫ�󶨵����ص��ĸ�IP��ַ
	sockaddrControl.sin_port = htons(30002);                          //��Ҫ�����Ķ˿�
	bind(sControl, (SOCKADDR*)&sockaddrControl, sizeof(SOCKADDR));        //���а󶨶���

	listen(sControl, 1);
	SOCKADDR clientControlAddr;
	int sizeControl = sizeof(SOCKADDR);
	//����ͨ�ţ�ȷ�Ͽ��Խ���������
	clientSocketControl = accept(sControl, &clientControlAddr, &sizeControl);               //������ֱ������tcp�ͻ�������
	int lenControl = getHead(clientSocketControl);
	char * buffer_inControl = (char *)malloc(lenControl * sizeof(char));
	char * overflowControl = readByteFromStream(clientSocketControl, buffer_inControl, lenControl);
	if (overflowControl != NULL){
		free(overflowControl);
		return;
	}
	if (strcmp(buffer_inControl, "��������") == 1){
		printf("��ʼ������");
	}
	free(buffer_inControl);
	int nControl = createUTF8Byte(cmd, chinese_cmd, 100);

	wrightBytes2Stream(clientSocketControl, chinese_cmd, nControl);

	//socket3�������ӵ�socket������������Ƶ����
	WSAStartup(MAKEWORD(2, 2), &wsaDataControlLong);
	sControlLong = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddrControlLong.sin_family = PF_INET;
	sockaddrControlLong.sin_addr.S_un.S_addr = inet_addr("172.20.10.9");   //��Ҫ�󶨵����ص��ĸ�IP��ַ
	sockaddrControlLong.sin_port = htons(30003);                          //��Ҫ�����Ķ˿�
	bind(sControlLong, (SOCKADDR*)&sockaddrControlLong, sizeof(SOCKADDR));        //���а󶨶���

	listen(sControlLong, 1);

	SOCKADDR clientAddr_long4Con;
	sizeControl = sizeof(SOCKADDR);

	clientSocketLong = accept(sControlLong, &clientAddr_long4Con, &sizeControl);               //������ֱ������tcp�ͻ�������
#endif
}

#if sendBySocket
void sendBytes(char * mymsg, int len){
	send(s, mymsg, len, NULL);
}
#endif

//��ʼ��������SDL��ffmpeg�ĳ�ʼ�����Լ�һЩ������ȫ�ֱ���
int initail(){
	//SDL��ʼ��
	sdlRenderer = NULL;
	sdlTexture = NULL;
	pFrameYUV = NULL;

	//��������ʼ��
	first_time = 1;
	pCodecCtx = NULL;
	pCodecParserCtx = NULL;
	ret = 0;
	got_picture = 0;
	pFrame = av_frame_alloc();//�����ڴ档AVFrameͨ������av_frame_alloc()��ʼ��,pFrame�������������������Ƶ֡
	av_init_packet(&packet);

	img_convert_ctx = NULL;

	avcodec_register_all();//ע�����еı������

	pCodec = avcodec_find_decoder(codec_id);//���ҽ�����codec_id = AV_CODEC_ID_H264

	if (!pCodec) {
		return -1;
	}
	pCodecCtx = avcodec_alloc_context3(pCodec);//ΪAVCodecContext�����ڴ�
	if (!pCodecCtx){
		return -1;
	}

	pCodecParserCtx = av_parser_init(codec_id);//��ʼ��AVCodecParserContext
	if (!pCodecParserCtx){
		return -1;
	}

	if (pCodec->capabilities&CODEC_CAP_TRUNCATED)
		pCodecCtx->flags |= CODEC_FLAG_TRUNCATED; /* we do not send complete frames ������û�з���������֡��*/

	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
		return -1;
	}

	wholeness_check = false;
}

//��������Ե�һ�����裬���������Զ���ȫ�ֱ���wholeness_check���false
int checkWholeness(uint8_t * in_buf, size_t size){
	if (size == 0)
		return ERROR_SIZE_0;
	cur_ptr = in_buf;
	while (size>0){
		int len = av_parser_parse2(  //�������һ��Packet:ʹ��AVCodecParser��������������з����һ֡һ֡��ѹ����������
			pCodecParserCtx, pCodecCtx,
			&packet.data, &packet.size,

			cur_ptr, size,                  //cur_ptrΪһ�ν��յ����ݰ�, cur_size���ν������ݰ��ĳ���
			AV_NOPTS_VALUE, AV_NOPTS_VALUE, AV_NOPTS_VALUE);

		cur_ptr += len;
		size -= len;
		if (packet.size == 0)
			continue;       //��������ѭ��ִ����һ��ѭ��

		ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, &packet);//����һ֡����
		av_free_packet(&packet);

		if (ret < 0) {//retС����˵�����������Ϊ�ļ�ͷ����ȷ
			//AfxMessageBox("Decode Error.(�������)\n");
			wholeness_check = false;
			return FIRST_FRAME_DECODED;
		}
		else{
			wholeness_check = true;
			return FIRST_FRAME_DECODED;
		}

	}
	return BUFFER_DECODED;
}

//��fp_head���ṩ��h.264I֡�Ķ������ļ��л�ȡI֡�����������뵽�������У�Ϊ����֡���з���
int decodeFileHead(){
	fp_head = fopen(filepath_head, "rb+");
	while (1) {
		//size_t fread(void * ptr,size_t size,size_t count,FILE * stream);//��ȡfp_in�ļ������ݣ�
		cur_size = fread(in_buffer, 1, 400, fp_head);//in_buffer������ݵĻ�����,fread����ʵ�ʶ�ȡ����count��Ŀ
		//cur_size = size;
		if (cur_size == 0)
			break;
		cur_ptr = in_buffer;
		while (cur_size>0){
			int len = av_parser_parse2(  //�������һ��Packet:ʹ��AVCodecParser��������������з����һ֡һ֡��ѹ����������
				pCodecParserCtx, pCodecCtx,
				&packet.data, &packet.size,

				cur_ptr, cur_size,                  //cur_ptrΪһ�ν��յ����ݰ�, cur_size���ν������ݰ��ĳ���
				AV_NOPTS_VALUE, AV_NOPTS_VALUE, AV_NOPTS_VALUE);

			cur_ptr += len;
			cur_size -= len;
			if (packet.size == 0)
				continue;       //��������ѭ��ִ����һ��ѭ��
			ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, &packet);//����һ֡����
			av_free_packet(&packet);
		}

		//av_free_packet(&packet);//�ͷű��ζ�ȡ��֡�ڴ�
	}
	fclose(fp_head);
	return 0;
}

//��������֡��������ʾ����ͨ��socket1���͸���������
int decodeandDisplay(uint8_t * in_buf, size_t size){
	if (size == 0)
		return ERROR_SIZE_0;
	cur_ptr = in_buf;
	while (size>0){
		int len = av_parser_parse2(  //�������һ��Packet:ʹ��AVCodecParser��������������з����һ֡һ֡��ѹ����������
			pCodecParserCtx, pCodecCtx,
			&packet.data, &packet.size,

			cur_ptr, size,                  //cur_ptrΪһ�ν��յ����ݰ�, cur_size���ν������ݰ��ĳ���
			AV_NOPTS_VALUE, AV_NOPTS_VALUE, AV_NOPTS_VALUE);

		cur_ptr += len;
		size -= len;
		if (packet.size == 0)
			continue;       //��������ѭ��ִ����һ��ѭ��
		ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, &packet);//����һ֡����
		av_free_packet(&packet);
		if (ret < 0) {
			//AfxMessageBox("Decode Error.(�������)\n");
			return ERROR_DECODEING;
		}


		if (got_picture) {
			if (first_time){//���������ò�����ֻ���ڵ�һ��ִ��ʱ����Ҫ���ã���󽫰�����������һֱִ����ȥ
				//sws_getContext���ݱ�����Ϣ������Ⱦ��ʽ,��һ�ڶ������������ֱ�������Դ�Ŀ�ߺ͸�ʽ�����ĵ�������������ֱ�������Ŀ�ߺ͸�ʽ
				//����һ��ֻ��������һ��ת�����ã���û����ʽ��ʼת��
				//����������ʽ����Ϊ��PIX_FMT_GRAY8��Ҳ����RGB��ʽ��������YUV����Ϊ�����ͼ����
				img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
					output_w, output_h, output_format, SWS_BICUBIC, NULL, NULL, NULL);


				//��ʼ��SDL---------------------------
				//SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

				///* ��ʼ��SDL��ϵͳ*/
				//if (SDL_Init(SDL_INIT_VIDEO) < 0) {
				//	SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s\n", SDL_GetError());
				//	return (1);
				//}
				////��������SDL 2.0 Support for multiple windows
				//screen = SDL_CreateWindow("ffmpeg player's Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
				//	output_w, output_h,
				//	SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

				//if (!screen) {
				//	//AfxMessageBox(SDL_GetError());
				//	return -1;
				//}

				sdlRenderer = SDL_CreateRenderer(window, -1, 0);

				//�ڶ������������Texture����ĸ�ʽ
				sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, output_w, output_h);

				sdlRect.x = 0;
				sdlRect.y = 0;
				sdlRect.w = output_w;
				sdlRect.h = output_h;
				//SDL ��ʼ�����----------------------

				pFrameYUV = av_frame_alloc();//ΪYUV֡�����ڴ�
				//����������һ�����buffer�����Ұ�����pFrameYUV����������ִ������������󣬵�pFrameYUV�ı�ʱout_bufferҲ���Զ��ı�
				out_buffer = (uint8_t *)av_malloc(avpicture_get_size(output_format, output_w, output_h));
				avpicture_fill((AVPicture *)pFrameYUV, out_buffer, output_format, output_w, output_h);

				y_size = pCodecCtx->width*pCodecCtx->height;

				first_time = 0;
			}

			//printf("Succeed to decode 1 frame!\n");
			//ʵ�����ź͸�ʽ�任
			sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height,
				pFrameYUV->data, pFrameYUV->linesize);
#if sendBySocket
			//�������е�ͼ����۵�̫��ʱ��ʡ��һЩ֡�������ڴ治��
			/*if (W.QueueLength() == 0){
			ElemType e = newElemType(output_w * output_h, (char*)out_buffer);
			W.EnQueue(e);
			}*/
			//DWORD  dwWaitResult = WaitForSingleObject(		//WaitForSingleObject���������ȴ�Event��Job��Memory resource notification��
			//	//Mutex��Process��Semaphore��Thread��Waitable timer�ȶ���Ľ���
			//	imMutex,    // handle to mutex
			//	INFINITE);  // no time-out interval
			//if (dwWaitResult != WAIT_TIMEOUT){
			//	W.ClearQueue();
			//	ElemType e = newElemType(output_w * output_h, (char*)out_buffer);
			//	W.EnQueue(e);
			//}
			//ReleaseMutex(imMutex);							//�ͷ��˻������
			//send(s, (char*)out_buffer, output_w * output_h * 3, NULL);
			//wrightBytes2Stream(s, (char*)out_buffer, output_w * output_h * 3);
			if (send2Matlab == true){
				wrightBytes2Stream(s, (char*)out_buffer, output_w * output_h);
				send2Matlab = false;
			}

#endif

			////SDL---------------------------
			SDL_UpdateTexture(sdlTexture, NULL, out_buffer, pFrameYUV->linesize[0]); //SDL_Texture������ʾYUV���ݡ�һ��SDL_Texture��Ӧһ֡YUV���� 
			SDL_RenderClear(sdlRenderer);
			SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, &sdlRect);
			SDL_RenderPresent(sdlRenderer);
			//return 0;//����ѭ��
		}
	}
	return BUFFER_DECODED;
}

//�������п�������ʣ�����ݣ�ȫ���������
int flushRemainFrame(){
	packet.data = NULL;
	packet.size = 0;
	while (1){
		ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, &packet);
		av_free_packet(&packet);
		if (ret < 0) {
			return ret;
		}
		if (!got_picture)
			break;
		if (got_picture) {
			//printf("Flush Decoder: Succeed to decode 1 frame!\n");
			//�Ѹ�֡ת������Ⱦ����YUV
			sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height,
				pFrameYUV->data, pFrameYUV->linesize);

			//SDL---------------------------
			SDL_UpdateTexture(sdlTexture, &sdlRect, pFrameYUV->data[0], pFrameYUV->linesize[0]); //SDL_Texture������ʾYUV���ݡ�һ��SDL_Texture��Ӧһ֡YUV���� 
			SDL_RenderClear(sdlRenderer);
			SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, &sdlRect);
			SDL_RenderPresent(sdlRenderer);
			//SDL End-----------------------
			////Delay 40ms
			//SDL_Delay(40);
		}
	}
}

//�ڴ��ͷţ�����SDL�ĺ�ffmpeg��
int recycle(){
	while (1);
	SDL_DestroyRenderer(sdlRenderer);

	SDL_DestroyTexture(sdlTexture);


	sws_freeContext(img_convert_ctx);
	av_parser_close(pCodecParserCtx);

	av_free_packet(&packet);
	av_frame_free(&pFrameYUV);
	av_frame_free(&pFrame);
	avcodec_close(pCodecCtx);
	av_free(pCodecCtx);
	av_free(pCodec);
	return 0;
}

#if sendBySocket
DWORD WINAPI sendImageInQueue(LPVOID lpParameter){
	char buffer[1000000];
	int len;
	while (isAlive){
		DWORD  dwWaitResult = WaitForSingleObject(		//WaitForSingleObject���������ȴ�Event��Job��Memory resource notification��
			//Mutex��Process��Semaphore��Thread��Waitable timer�ȶ���Ľ���
			imMutex,    // handle to mutex
			INFINITE);  // no time-out interval
		ElemType e;
		if (!W.DeQueue(&e)) {
			ReleaseMutex(imMutex);							//�ͷ��˻������
			continue;
		}
		memcpy(buffer, e.data, e.len);
		len = e.len;
		freeElemType(e);
		ReleaseMutex(imMutex);							//�ͷ��˻������
		//send(s, e.data, e.len, NULL);
		wrightBytes2Stream(s, buffer, len);
	}
	return 1;
}
#endif

/*�˳�����֮ǰ������ SDL subsystem */
static void
quit(int rc)
{
	SDL_Quit();
	exit(rc);
}

#if FLIGHT_CONTROL
DWORD WINAPI flightControlThread(LPVOID lpParameter){

#if !sendBySocket

	int done = 0;
	//������ windows ��Ϣѭ�� ʵ���ϵײ���windowsƽ̨Ҳ�Ƿ�װ�� windows��Ϣѭ������
	char cmd[] = { 0, 1, 3, 1, 0, 0, 0, 1, 0, 0 };
	char in_buffer[2000] = { 0 };
	while (!done)
	{
		int  key;

		DWORD  dwWaitResult = WaitForSingleObject(
			ghMutex,    // handle to mutex
			INFINITE);  // no time-out interval
		if (dwWaitResult == WAIT_ABANDONED) continue;
		if (eventQueue.DeQueue(&key) == false){
			ReleaseMutex(ghMutex);
			continue;
		}
		ReleaseMutex(ghMutex);
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
			continue;
			break;
		}

		wrightBytes2Stream(clientSocketLong, cmd, 10);

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

	//����Ѿ���ʼ������ϵͳ ��Ӧ���ڳ����˳���ʱ�����
	eventQueue.DestroyQueue();
#else
	while (1){
		int cursize = getHead(s);
		char cmd[10];
		char * overflow = readByteFromStream(s, cmd, cursize);
		if (overflow != NULL){
			free(overflow);
			return 0;
		}
		if ((cmd[0] == 0)&(cmd[1] == 1)&(cmd[2] == 4)){
			send2Matlab = true;
			continue;
		}
		wrightBytes2Stream(clientSocketLong, cmd, cursize);

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
#endif
	return (0);
}
#endif


DWORD WINAPI decodeImageThread(LPVOID lpParameter){

	initail();
#if DECODE_NATIVE_CODE
	fp_in = fopen(filepath_in, "rb+");
#endif
	int result;
#if ENABLE_WHOLENESS_CHECK
	while (1) {
		cur_size = fread(in_buffer, 1, 40000, fp_in);//in_buffer������ݵĻ�����,fread����ʵ�ʶ�ȡ����count��Ŀ
		result = checkWholeness(in_buffer, cur_size);

		if (result == ERROR_SIZE_0){
			printf("�ļ���ȡ����");
			break;
		}
		if (result == FIRST_FRAME_DECODED){
			break;
		}
		if (result == BUFFER_DECODED){
			continue;
		}
	}
#endif

	//��������Ϊ��ʱ�����в����ļ�ͷ�Ľ��룬�Ѳ����ļ�ͷ���뵽��������
	if (wholeness_check == false){
		decodeFileHead();
	}

	int ft = true;			//������ոտ�ʼ������߳����ڶ�ȡ��һ�δ��������֮�󣬿��ǵ����ݵĶ�ȡ�����Ѿ�����
	//�����¿�ʼ��ȡ����ʱ�õ������ݲ�һ����h.264�����е�֡ͷ��ʼ����������Щʱ�̳�����ftΪ��
	//�������ڽ�������л�������������л�ȡ��h.264��׼֡ͷ��ʼ����һ������
	//�����䣬h.264��֡ͷһ������[0, 0, 0]��ͷ��������һ��������һ֡�����͵�����
	uint8_t * start_pos;
	time_t start = NULL, end = NULL;
	int last_cur_size = 0;
	while (1) {
		//size_t fread(void * ptr,size_t size,size_t count,FILE * stream);//��ȡfp_in�ļ������ݣ�

#if DECODE_NATIVE_CODE
		cur_size = fread(in_buffer, 1, 2000, fp_in);//in_buffer������ݵĻ�����,fread����ʵ�ʶ�ȡ����count��Ŀ
#else
		int cur_size = getHead(clientsocket_long);
		char * overflow = readByteFromStream(clientsocket_long, (char *)in_buffer, cur_size);
		/*if (cur_size != -1){
		for (int i = cur_size; i < last_cur_size; i++){
		in_buffer[i] = 0;
		}


		last_cur_size = cur_size;
		}*/
		if (overflow != NULL){
			free(overflow);
			return 0;
		}
#endif

		//��ftΪ��ʱ�����������ȡ������֡ͷ����һ������
		start_pos = in_buffer;
		if (ft){
			for (int i = 0; i < cur_size; i++){
				if (in_buffer[i] == 0){
					if (in_buffer[i + 1] == 0){
						if (in_buffer[i + 2] == 0){
							if (in_buffer[i + 3] == 1){
								if (in_buffer[i + 4] == 97){
									ft = false;
									start_pos = in_buffer + i;
									cur_size = cur_size - i;
									printf("֡ͷΪ��0 0 0 %d %d \n", start_pos[3], start_pos[4]);
								}
							}
						}
					}
				}
			}
		}

		//���ft��δ�棬˵��ǰ��Ѱ����֡ͷ����û��Ѱ�ҳɹ���������һ�����ݶ���Ϊ��Ч���ݣ�������ȡ��һ�Ρ�
		if (ft == true){
			continue;
		}

		//��cur_sizeΪ-1˵��getHead�м������ݶε�ͷ����
		if (cur_size == -1){
			ft = true;
			continue;
		}
		result = decodeandDisplay(start_pos, cur_size);
		if (result == ERROR_SIZE_0){
			printf("���ļ�\n");
			if (start == NULL){
				time(&start);
			}
			else{
				time(&end);
				if (difftime(end, start) > 10){
					break;
				}
			}
			continue;
		}
		if (result == ERROR_DECODEING){
			printf("������ڴ�����һ֡��ʼ���ý���\n");
			ft = true;
		}
		//һ֡���ݽ�����ϣ�������һ֡
		if (result == BUFFER_DECODED){
			if (start != NULL){
				start = NULL;
			}
			continue;
		}
	}
	flushRemainFrame();

#if DECODE_NATIVE_CODE
	rewind(fp_in);
	fclose(fp_in);
#endif

	recycle();
	return 0;
}


int main(int argc, char* argv[]){
	eventQueue = Queue<int>();
	W = Queue<ElemType>();

	ghMutex = CreateMutex(			//�ҳ���ǰϵͳ�Ƿ��Ѿ�����ָ�����̵�ʵ�������û���򴴽�һ��������
		NULL,              // default security attributes
		FALSE,             // initially not owned
		NULL);             // unnamed mutex;
	imMutex = CreateMutex(			//�ҳ���ǰϵͳ�Ƿ��Ѿ�����ָ�����̵�ʵ�������û���򴴽�һ��������
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
	socketInitial();


	HANDLE handleDecode = CreateThread(NULL, 0, decodeImageThread, NULL, 0, NULL);
	if (NULL == handleDecode)
	{
		std::cout << "Create Thread failed !" << std::endl;
	}
	CloseHandle(handleDecode);

#if sendBySocket
	HANDLE handle;
	handle = CreateThread(NULL, 0, sendImageInQueue, NULL, 0, NULL);
	if (NULL == handle)
	{
		std::cout << "Create Thread failed !" << std::endl;
	}
	CloseHandle(handle);
#endif
#if FLIGHT_CONTROL
	HANDLE handleControl = CreateThread(NULL, 0, flightControlThread, NULL, 0, NULL);
	if (NULL == handleControl)
	{
		std::cout << "Create Thread failed !" << std::endl;
	}
	CloseHandle(handleControl);
#endif
	//��ʼ����Unicode�ı������¼�  ���֧�ֵĻ�������ʾ��Ļ����
	SDL_StartTextInput();
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

			//Ϊ�˷�ֹ�������������̶�д��������ɻ��ң���������Mutex����ͬ����
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

	WaitForSingleObject(handleDecode, INFINITE);
	//WaitForSingleObject(handleControl, INFINITE);
	SDL_DestroyWindow(screen);
	SDL_Quit();
	return 0;
}
