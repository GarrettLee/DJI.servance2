#include "ff.h"
#include <stdio.h>                      //用于输入、输出函数的调用,printf, gets
#include <winsock2.h>                   //socket头文件
#include <Windows.h>                    //为了方便调试，所以加入了等待2秒才进行连接server，这里用到了sleep函数
#include <iostream>						//用来做输入输出（std::cout<<String<<std::endl）
#include <time.h>						//获取系统时间，并求时间差
#include "DynaLnkQueue.hpp"				//队列

#pragma comment (lib, "ws2_32")         //socket库文件
#define sendBySocket 0					//为1时会启动socket1，把解码后的bitmap图片发送到本机的其他应用，
#define ENABLE_WHOLENESS_CHECK 0;		//为1时会启动码流完整性检查，否则默认认为码流是完整的
#define DECODE_NATIVE_CODE 0			//为1时从项目目录下读取作为例子用的本地h.264文件做解码，这样就能方便解码器的调试
#define FLIGHT_CONTROL 0				//为1时会启用大疆的飞行控制

//解码函数decodeandDisplay可能的输出，用来和外部代码进行交互以了解到解码是否出错
#define ERROR_SIZE_0 0
#define FIRST_FRAME_DECODED 1
#define BUFFER_DECODED 2
#define ERROR_DECODEING 3

//using namespace std;

AVPixelFormat output_format = PIX_FMT_GRAY8; //输出图像格式，一般为PIX_FMT_RGB24或者PIX_FMT_GRAY8

//多线程
DWORD WINAPI sendImageInQueue(LPVOID lpParameter);
DWORD WINAPI decodeImageThread(LPVOID lpParameter);
DWORD WINAPI flightControlThread(LPVOID lpParameter);

HANDLE ghMutex;
HANDLE imMutex;

Queue<ElemType> W;							//队列，按顺序存放等待发送的位图数据
Queue<int> eventQueue;					//队列，存放键盘事件
bool eventQueueBeingUsed = false;		//当有代码正被占用eventQueue时置为真

bool isAlive = true;					//控制线程结束的标志位
bool send2Matlab = false;				//当为真时发送图像给MATLAB端

SDL_Window *window;



#define START_FLAG 0x11					//socket的文件头，这是本人自己约定的一个文件头
//SDL---------------------------
int screen_w = 480, screen_h = 272;
int output_w = 640, output_h = 320;		//输出的图片的大小
SDL_Window *screen;						//SDL_Window就是使用SDL的时候弹出的那个窗口
SDL_Renderer* sdlRenderer;				//SDL_Renderer用于渲染SDL_Texture至SDL_Window
SDL_Texture* sdlTexture;				//SDL_Texture用于显示YUV数据。一个SDL_Texture对应一帧YUV数据
SDL_Rect sdlRect;						//SDL_Rect用于确定SDL_Texture显示的位置

//SDL End----------------------
struct SwsContext *img_convert_ctx;
AVCodec *pCodec;                         //每种解码器都对应一个AVCodec结构
AVCodecContext *pCodecCtx = NULL;        //每个AVCodecContext中对应一个AVCodec
AVCodecParserContext *pCodecParserCtx = NULL;
int frame_count;
FILE *fp_in;							//输入二进制文件流
FILE *fp_head;							//输入的补充
AVFrame	*pFrame, *pFrameYUV;			//解码后数据
uint8_t *out_buffer;
const long in_buffer_size = 50000;
uint8_t in_buffer[in_buffer_size + FF_INPUT_BUFFER_PADDING_SIZE] = { 0 };
uint8_t *cur_ptr;
int cur_size;
int first_time = 1;

AVPacket packet;						//解码前数据,使用AVPacket来暂存解复用之后、解码之前的媒体数据（一个音/视频帧、一个字幕包等）及附加信息（解码时间戳、显示时间戳、时长等）
int ret = 0;
int got_picture = 0;

int y_size;

const AVCodecID codec_id = AV_CODEC_ID_H264;
const char filepath_in[] = "bigbuckbunny_480x272.h265";//bigbuckbunny_480x272.h264为裸流文件
const char filepath_head[] = "myData.h264";//补充的码流I帧

int wholeness_check = true;			//完整性检查标志，当为真时说明输入的文件完整，不需要补充I帧

int wrightBytes2Stream(SOCKET s, char* data, unsigned int len) {
	// add buffered writer

	if (len <= 0xffff){
		uint8_t head[3];
		//给TCP的数据加个数据头，内容为该段数据的长度
		head[0] = START_FLAG;
		head[1] = len & 0x00ff;
		head[2] = len >> 8;					//把数据的长度拆成两个字节发出去

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

//从缓存中获取一个数据头，返回该组数据的长度
int getHead(SOCKET s){
	int dataLength = 0;
	uint8_t start[3] = { 0 };
	recv(s, (char *)start, 3, NULL);
	if (start[0] == START_FLAG){
		dataLength = (start[1] & (int)0xff) + ((start[2] << 8)&(int)0xffff);
	}
	else{
		printf("文件头出错");
		return -1;
	}
	return dataLength;
}

//根据长度从缓存中取出额定长度的数据，如果取出数据时取得过多数据造成溢出，则返回溢出的数据，返回的这个数据需要就行free操作
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
		//一直接收客户端socket的send操作
		if (rev_num == 0){
			continue;
		}
		if (rev_num <= data_left){
			memcpy(frame_ptr, buffer, rev_num * sizeof(char));
			frame_ptr += rev_num * sizeof(char);
			data_left -= rev_num;
		}
		else if (rev_num > data_left){
			printf("ERROR：读取数据溢出！");
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


//将中文字符转成UTF8编码的字节流
int createUTF8Byte(char *chinese_string, char *out, int out_len){
	wchar_t wc[256];
	// 将ANSI编码的多字节字符串转换成宽字符字符串  
	int n = MultiByteToWideChar(CP_ACP, 0, chinese_string, strlen(chinese_string), wc, 256);
	if (n > 0)
	{
		wc[n] = 0;

		// 将宽字符字符串转换成UTF-8编码的多字节字符串  
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
	Sleep(2000);                        //沉睡2秒再连接server
	char cmd[] = "允许开始长连接";
	char chinese_cmd[100];
#if sendBySocket
	//socket1：用来把解码后得到的bitmap图片发出去，这里就发回本机的其他程序
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
	//socket2：用来为与手机端的长链接的建立和退出服务
	WSAStartup(MAKEWORD(2, 2), &wsaData_in);
	s_in = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_input.sin_family = PF_INET;
	sockaddr_input.sin_addr.S_un.S_addr = inet_addr("172.20.10.9");   //需要绑定到本地的哪个IP地址
	sockaddr_input.sin_port = htons(30000);                          //需要监听的端口
	bind(s_in, (SOCKADDR*)&sockaddr_input, sizeof(SOCKADDR));        //进行绑定动作

	listen(s_in, 1);

	SOCKADDR clientAddr;
	int size = sizeof(SOCKADDR);

	SOCKET clientsocket;
	//进行通信，确认可以建立长链接
	clientsocket = accept(s_in, &clientAddr, &size);               //阻塞，直到有新tcp客户端连接
	int len = getHead(clientsocket);
	char * buffer_in = (char *)malloc(len * sizeof(char));
	char * overflow = readByteFromStream(clientsocket, buffer_in, len);
	if (overflow != NULL){
		free(overflow);
		return;
	}
	if (strcmp(buffer_in, "请求长连接") == 1){
		printf("开始长连接");
	}
	free(buffer_in);

	int n = createUTF8Byte(cmd, chinese_cmd, 100);

	wrightBytes2Stream(clientsocket, chinese_cmd, n);
	//socket3：长链接的socket，用来接受视频码流
	WSAStartup(MAKEWORD(2, 2), &wsaData_long);
	socket_long = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_long.sin_family = PF_INET;
	sockaddr_long.sin_addr.S_un.S_addr = inet_addr("172.20.10.9");   //需要绑定到本地的哪个IP地址
	sockaddr_long.sin_port = htons(30001);                          //需要监听的端口
	bind(socket_long, (SOCKADDR*)&sockaddr_long, sizeof(SOCKADDR));        //进行绑定动作

	listen(socket_long, 1);

	SOCKADDR clientAddr_long;
	size = sizeof(SOCKADDR);

	clientsocket_long = accept(socket_long, &clientAddr_long, &size);               //阻塞，直到有新tcp客户端连接
#endif

#if FLIGHT_CONTROL

	//socket2：用来为与手机端的长链接的建立和退出服务
	WSAStartup(MAKEWORD(2, 2), &wsaDataControl);
	sControl = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddrControl.sin_family = PF_INET;
	sockaddrControl.sin_addr.S_un.S_addr = inet_addr("172.20.10.9");   //需要绑定到本地的哪个IP地址
	sockaddrControl.sin_port = htons(30002);                          //需要监听的端口
	bind(sControl, (SOCKADDR*)&sockaddrControl, sizeof(SOCKADDR));        //进行绑定动作

	listen(sControl, 1);
	SOCKADDR clientControlAddr;
	int sizeControl = sizeof(SOCKADDR);
	//进行通信，确认可以建立长链接
	clientSocketControl = accept(sControl, &clientControlAddr, &sizeControl);               //阻塞，直到有新tcp客户端连接
	int lenControl = getHead(clientSocketControl);
	char * buffer_inControl = (char *)malloc(lenControl * sizeof(char));
	char * overflowControl = readByteFromStream(clientSocketControl, buffer_inControl, lenControl);
	if (overflowControl != NULL){
		free(overflowControl);
		return;
	}
	if (strcmp(buffer_inControl, "请求长连接") == 1){
		printf("开始长连接");
	}
	free(buffer_inControl);
	int nControl = createUTF8Byte(cmd, chinese_cmd, 100);

	wrightBytes2Stream(clientSocketControl, chinese_cmd, nControl);

	//socket3：长链接的socket，用来接受视频码流
	WSAStartup(MAKEWORD(2, 2), &wsaDataControlLong);
	sControlLong = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddrControlLong.sin_family = PF_INET;
	sockaddrControlLong.sin_addr.S_un.S_addr = inet_addr("172.20.10.9");   //需要绑定到本地的哪个IP地址
	sockaddrControlLong.sin_port = htons(30003);                          //需要监听的端口
	bind(sControlLong, (SOCKADDR*)&sockaddrControlLong, sizeof(SOCKADDR));        //进行绑定动作

	listen(sControlLong, 1);

	SOCKADDR clientAddr_long4Con;
	sizeControl = sizeof(SOCKADDR);

	clientSocketLong = accept(sControlLong, &clientAddr_long4Con, &sizeControl);               //阻塞，直到有新tcp客户端连接
#endif
}

#if sendBySocket
void sendBytes(char * mymsg, int len){
	send(s, mymsg, len, NULL);
}
#endif

//初始化，包括SDL和ffmpeg的初始化，以及一些功能性全局变量
int initail(){
	//SDL初始化
	sdlRenderer = NULL;
	sdlTexture = NULL;
	pFrameYUV = NULL;

	//解码器初始化
	first_time = 1;
	pCodecCtx = NULL;
	pCodecParserCtx = NULL;
	ret = 0;
	got_picture = 0;
	pFrame = av_frame_alloc();//分配内存。AVFrame通过函数av_frame_alloc()初始化,pFrame保留解码器中输出的视频帧
	av_init_packet(&packet);

	img_convert_ctx = NULL;

	avcodec_register_all();//注册所有的编解码器

	pCodec = avcodec_find_decoder(codec_id);//查找解码器codec_id = AV_CODEC_ID_H264

	if (!pCodec) {
		return -1;
	}
	pCodecCtx = avcodec_alloc_context3(pCodec);//为AVCodecContext分配内存
	if (!pCodecCtx){
		return -1;
	}

	pCodecParserCtx = av_parser_init(codec_id);//初始化AVCodecParserContext
	if (!pCodecParserCtx){
		return -1;
	}

	if (pCodec->capabilities&CODEC_CAP_TRUNCATED)
		pCodecCtx->flags |= CODEC_FLAG_TRUNCATED; /* we do not send complete frames （我们没有发送完整的帧）*/

	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
		return -1;
	}

	wholeness_check = false;
}

//检查完整性的一个步骤，如果出错会自动把全局变量wholeness_check设成false
int checkWholeness(uint8_t * in_buf, size_t size){
	if (size == 0)
		return ERROR_SIZE_0;
	cur_ptr = in_buf;
	while (size>0){
		int len = av_parser_parse2(  //解析获得一个Packet:使用AVCodecParser从输入的数据流中分离出一帧一帧的压缩编码数据
			pCodecParserCtx, pCodecCtx,
			&packet.data, &packet.size,

			cur_ptr, size,                  //cur_ptr为一次接收的数据包, cur_size本次接收数据包的长度
			AV_NOPTS_VALUE, AV_NOPTS_VALUE, AV_NOPTS_VALUE);

		cur_ptr += len;
		size -= len;
		if (packet.size == 0)
			continue;       //结束本次循环执行下一次循环

		ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, &packet);//解码一帧数据
		av_free_packet(&packet);

		if (ret < 0) {//ret小于零说明解码出错，视为文件头不正确
			//AfxMessageBox("Decode Error.(解码错误)\n");
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

//从fp_head所提供的h.264I帧的二进制文件中获取I帧，并把它输入到解码器中，为其后的帧进行服务
int decodeFileHead(){
	fp_head = fopen(filepath_head, "rb+");
	while (1) {
		//size_t fread(void * ptr,size_t size,size_t count,FILE * stream);//读取fp_in文件的内容，
		cur_size = fread(in_buffer, 1, 400, fp_head);//in_buffer存放数据的缓冲区,fread返回实际读取到的count数目
		//cur_size = size;
		if (cur_size == 0)
			break;
		cur_ptr = in_buffer;
		while (cur_size>0){
			int len = av_parser_parse2(  //解析获得一个Packet:使用AVCodecParser从输入的数据流中分离出一帧一帧的压缩编码数据
				pCodecParserCtx, pCodecCtx,
				&packet.data, &packet.size,

				cur_ptr, cur_size,                  //cur_ptr为一次接收的数据包, cur_size本次接收数据包的长度
				AV_NOPTS_VALUE, AV_NOPTS_VALUE, AV_NOPTS_VALUE);

			cur_ptr += len;
			cur_size -= len;
			if (packet.size == 0)
				continue;       //结束本次循环执行下一次循环
			ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, &packet);//解码一帧数据
			av_free_packet(&packet);
		}

		//av_free_packet(&packet);//释放本次读取的帧内存
	}
	fclose(fp_head);
	return 0;
}

//解码数据帧，并且显示或者通过socket1发送给其他程序
int decodeandDisplay(uint8_t * in_buf, size_t size){
	if (size == 0)
		return ERROR_SIZE_0;
	cur_ptr = in_buf;
	while (size>0){
		int len = av_parser_parse2(  //解析获得一个Packet:使用AVCodecParser从输入的数据流中分离出一帧一帧的压缩编码数据
			pCodecParserCtx, pCodecCtx,
			&packet.data, &packet.size,

			cur_ptr, size,                  //cur_ptr为一次接收的数据包, cur_size本次接收数据包的长度
			AV_NOPTS_VALUE, AV_NOPTS_VALUE, AV_NOPTS_VALUE);

		cur_ptr += len;
		size -= len;
		if (packet.size == 0)
			continue;       //结束本次循环执行下一次循环
		ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, &packet);//解码一帧数据
		av_free_packet(&packet);
		if (ret < 0) {
			//AfxMessageBox("Decode Error.(解码错误)\n");
			return ERROR_DECODEING;
		}


		if (got_picture) {
			if (first_time){//这里是配置操作，只有在第一次执行时才需要配置，其后将按照这种配置一直执行下去
				//sws_getContext根据编码信息设置渲染格式,第一第二第三个参数分别是输入源的宽高和格式，第四第五第六个参数分别是输出的宽高和格式
				//在这一步只是设置了一种转换配置，并没有正式开始转换
				//这里的输出格式被设为了PIX_FMT_GRAY8，也就是RGB格式，而不死YUV，因为这便于图像处理
				img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
					output_w, output_h, output_format, SWS_BICUBIC, NULL, NULL, NULL);


				//初始化SDL---------------------------
				//SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

				///* 初始化SDL子系统*/
				//if (SDL_Init(SDL_INIT_VIDEO) < 0) {
				//	SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s\n", SDL_GetError());
				//	return (1);
				//}
				////创建窗口SDL 2.0 Support for multiple windows
				//screen = SDL_CreateWindow("ffmpeg player's Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
				//	output_w, output_h,
				//	SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

				//if (!screen) {
				//	//AfxMessageBox(SDL_GetError());
				//	return -1;
				//}

				sdlRenderer = SDL_CreateRenderer(window, -1, 0);

				//第二个参数是这个Texture输入的格式
				sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, output_w, output_h);

				sdlRect.x = 0;
				sdlRect.y = 0;
				sdlRect.w = output_w;
				sdlRect.h = output_h;
				//SDL 初始化完成----------------------

				pFrameYUV = av_frame_alloc();//为YUV帧分配内存
				//这里声明了一个输出buffer，并且把它和pFrameYUV连接起来，执行完这个操作后，当pFrameYUV改变时out_buffer也会自动改变
				out_buffer = (uint8_t *)av_malloc(avpicture_get_size(output_format, output_w, output_h));
				avpicture_fill((AVPicture *)pFrameYUV, out_buffer, output_format, output_w, output_h);

				y_size = pCodecCtx->width*pCodecCtx->height;

				first_time = 0;
			}

			//printf("Succeed to decode 1 frame!\n");
			//实行缩放和格式变换
			sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height,
				pFrameYUV->data, pFrameYUV->linesize);
#if sendBySocket
			//当队列中的图像积累得太多时，省略一些帧，以免内存不足
			/*if (W.QueueLength() == 0){
			ElemType e = newElemType(output_w * output_h, (char*)out_buffer);
			W.EnQueue(e);
			}*/
			//DWORD  dwWaitResult = WaitForSingleObject(		//WaitForSingleObject可以用来等待Event、Job、Memory resource notification、
			//	//Mutex、Process、Semaphore、Thread、Waitable timer等对象的接收
			//	imMutex,    // handle to mutex
			//	INFINITE);  // no time-out interval
			//if (dwWaitResult != WAIT_TIMEOUT){
			//	W.ClearQueue();
			//	ElemType e = newElemType(output_w * output_h, (char*)out_buffer);
			//	W.EnQueue(e);
			//}
			//ReleaseMutex(imMutex);							//释放了互斥对象
			//send(s, (char*)out_buffer, output_w * output_h * 3, NULL);
			//wrightBytes2Stream(s, (char*)out_buffer, output_w * output_h * 3);
			if (send2Matlab == true){
				wrightBytes2Stream(s, (char*)out_buffer, output_w * output_h);
				send2Matlab = false;
			}

#endif

			////SDL---------------------------
			SDL_UpdateTexture(sdlTexture, NULL, out_buffer, pFrameYUV->linesize[0]); //SDL_Texture用于显示YUV数据。一个SDL_Texture对应一帧YUV数据 
			SDL_RenderClear(sdlRenderer);
			SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, &sdlRect);
			SDL_RenderPresent(sdlRenderer);
			//return 0;//跳出循环
		}
	}
	return BUFFER_DECODED;
}

//解码器中可能仍有剩余数据，全部解码出来
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
			//把该帧转换（渲染）成YUV
			sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height,
				pFrameYUV->data, pFrameYUV->linesize);

			//SDL---------------------------
			SDL_UpdateTexture(sdlTexture, &sdlRect, pFrameYUV->data[0], pFrameYUV->linesize[0]); //SDL_Texture用于显示YUV数据。一个SDL_Texture对应一帧YUV数据 
			SDL_RenderClear(sdlRenderer);
			SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, &sdlRect);
			SDL_RenderPresent(sdlRenderer);
			//SDL End-----------------------
			////Delay 40ms
			//SDL_Delay(40);
		}
	}
}

//内存释放，包括SDL的和ffmpeg的
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
		DWORD  dwWaitResult = WaitForSingleObject(		//WaitForSingleObject可以用来等待Event、Job、Memory resource notification、
			//Mutex、Process、Semaphore、Thread、Waitable timer等对象的接收
			imMutex,    // handle to mutex
			INFINITE);  // no time-out interval
		ElemType e;
		if (!W.DeQueue(&e)) {
			ReleaseMutex(imMutex);							//释放了互斥对象
			continue;
		}
		memcpy(buffer, e.data, e.len);
		len = e.len;
		freeElemType(e);
		ReleaseMutex(imMutex);							//释放了互斥对象
		//send(s, e.data, e.len, NULL);
		wrightBytes2Stream(s, buffer, len);
	}
	return 1;
}
#endif

/*退出程序之前先清理 SDL subsystem */
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
	//类似与 windows 消息循环 实际上底层在windows平台也是封装的 windows消息循环机制
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

	//清除已经初始化的子系统 你应该在程序退出的时候调用
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
		cur_size = fread(in_buffer, 1, 40000, fp_in);//in_buffer存放数据的缓冲区,fread返回实际读取到的count数目
		result = checkWholeness(in_buffer, cur_size);

		if (result == ERROR_SIZE_0){
			printf("文件读取出错");
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

	//当完整性为假时，进行补充文件头的解码，把补充文件头加入到解码器中
	if (wholeness_check == false){
		decodeFileHead();
	}

	int ft = true;			//当程序刚刚开始解码或者程序在读取到一次错误的数据之后，考虑到数据的读取可能已经出错
	//，重新开始读取数据时得到的数据不一定从h.264编码中的帧头开始，所以在这些时刻程序让ft为真
	//，这样在解码过程中会遍历码流，从中获取以h.264标准帧头开始的那一段码流
	//。补充，h.264的帧头一般是由[0, 0, 0]开头，其后跟随一个决定这一帧的类型的数。
	uint8_t * start_pos;
	time_t start = NULL, end = NULL;
	int last_cur_size = 0;
	while (1) {
		//size_t fread(void * ptr,size_t size,size_t count,FILE * stream);//读取fp_in文件的内容，

#if DECODE_NATIVE_CODE
		cur_size = fread(in_buffer, 1, 2000, fp_in);//in_buffer存放数据的缓冲区,fread返回实际读取到的count数目
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

		//当ft为真时，从码流里获取可能是帧头的那一段数据
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
									printf("帧头为：0 0 0 %d %d \n", start_pos[3], start_pos[4]);
								}
							}
						}
					}
				}
			}
		}

		//如果ft仍未真，说明前面寻找了帧头并且没有寻找成功，所以这一段数据都视为无效数据，继续读取下一段。
		if (ft == true){
			continue;
		}

		//当cur_size为-1说明getHead中检测出数据段的头错误
		if (cur_size == -1){
			ft = true;
			continue;
		}
		result = decodeandDisplay(start_pos, cur_size);
		if (result == ERROR_SIZE_0){
			printf("空文件\n");
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
			printf("解码存在错误，下一帧开始重置解码\n");
			ft = true;
		}
		//一帧数据解码完毕，继续下一帧
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

	ghMutex = CreateMutex(			//找出当前系统是否已经存在指定进程的实例。如果没有则创建一个互斥体
		NULL,              // default security attributes
		FALSE,             // initially not owned
		NULL);             // unnamed mutex;
	imMutex = CreateMutex(			//找出当前系统是否已经存在指定进程的实例。如果没有则创建一个互斥体
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
	//开始接受Unicode文本输入事件  如果支持的话将会显示屏幕键盘
	SDL_StartTextInput();
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

			//为了防止队列在两个进程读写过程中造成混乱，这里用了Mutex来做同步锁
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

	WaitForSingleObject(handleDecode, INFINITE);
	//WaitForSingleObject(handleControl, INFINITE);
	SDL_DestroyWindow(screen);
	SDL_Quit();
	return 0;
}
