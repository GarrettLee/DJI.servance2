//解码线程

#include <time.h>						//获取系统时间，并求时间差
extern "C"
{
#include "libavcodec/avcodec.h"			//解码函数用的库
#include "libswscale/swscale.h"			//解码函数用的库
#include "SDL2/SDL.h"					//SDL显示
};

#include "SocketUtils.h"	//数据发送相关工具
#include "H264Decoder.h"	// winsock2.h中没有判断windows.h或者winsock.h有没有加载，windows.h中也没有判断winsock2有没有加载，所以两个都加载可能会发生重定义。但是windows.h中会判断_WINSOCKAPI_有没有被定义，而winsock2.h中有定义_WINSOCKAPI_，所以只要先加载winsock2.h再加载windows.h即可。
#include "sendBySocket.h"	//与MATLAB进行通信的线程，没有使用
#include "globalVar.h"		//全局变量的定义

HANDLE rectMutex;			//互斥体
bool send2Matlab = false;				//当为真时解码器下一帧图像会发给MATLAB端

//SDL---------------------------
SDL_Renderer* sdlRenderer;				//SDL_Renderer用于渲染SDL_Texture至SDL_Window
SDL_Texture* sdlTexture;				//SDL_Texture用于显示YUV数据。一个SDL_Texture对应一帧YUV数据
SDL_Rect sdlRect;						//SDL_Rect用于确定SDL_Texture显示的位置
SDL_Window *window;					
SDL_Window *screen;						//SDL_Window就是使用SDL的时候弹出的那个窗口
SDL_Rect rect;							//标记被跟踪目标的框
//SDL End----------------------

//文件 start
FILE *fp_in;							//输入二进制文件流
FILE *fp_head;							//输入的补充
const char filepath_in[] = "fortest.h264";//bigbuckbunny_480x272.h264为裸流文件
const char filepath_head[] = "myData.h264";//补充的码流I帧
//文件 end

//缓存 start
const long in_buffer_size = 50000;		//缓存区的大小
uint8_t in_buffer[in_buffer_size + FF_INPUT_BUFFER_PADDING_SIZE] = { 0 };	//解码器输入缓存
uint8_t *out_buffer;					//解码出来一帧图像的内容
uint8_t *cur_ptr;
int cur_size;							//解码器输入缓存中有效数据的长度

uint8_t * gb;
uint8_t ** gray_buffer = &gb;
//缓存 end

//服务于解码过程的状态变量 start
int first_time = 1;						//检测是不是第一次执行解码程序
int got_picture = 0;					//是否成功解码出图片
int ret = 0;							//在解码过程中担任解码结果的输出这个功能
int wholeness_check = false;				//完整性检查标志，当为真时说明输入的文件完整，不需要补充I帧
int output_w = 640, output_h = 320;		//输出的图片的大小
//服务于解码过程的状态变量 end



//解码器 start----------------
struct SwsContext *img_convert_ctx;
AVCodec *pCodec;                         //每种解码器都对应一个AVCodec结构
AVCodecContext *pCodecCtx = NULL;        //每个AVCodecContext中对应一个AVCodec
AVCodecParserContext *pCodecParserCtx = NULL;
AVFrame	*pFrame, *pFrameYUV;			//解码后数据
AVPacket packet;						//解码前数据,使用AVPacket来暂存解复用之后、解码之前的媒体数据（一个音/视频帧、一个字幕包等）及附加信息（解码时间戳、显示时间戳、时长等）
const AVCodecID codec_id = AV_CODEC_ID_H264;
AVPixelFormat output_format = PIX_FMT_RGB24; //输出图像格式，一般为PIX_FMT_RGB24或者PIX_FMT_GRAY8

struct SwsContext *r2g;
struct SwsContext **rgb2gray = &r2g;
AVFrame * fg;
AVFrame	 **pFrameGray = &fg;			//解码后数据

//解码器 end



//socket start------------------------
#if !DECODE_NATIVE_CODE

SOCKET *clientsocket_heartbreak;		
SOCKET *clientsocket_h264code;		//用来承担接收来自手机端的h264编码的SOCKET
#endif
//socket end--------------------------


void onOneFrameDecoded();

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
	uint8_t * cur_ptr = in_buf;
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

		int ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, &packet);//解码一帧数据
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
int decodeFileHead(const char* filepath_head){
	FILE * fp_head = fopen(filepath_head, "rb+");
	uint8_t in_buffer[in_buffer_size + FF_INPUT_BUFFER_PADDING_SIZE] = { 0 };	//解码器输入缓存
	int cur_size;
	uint8_t * cur_ptr;
	while (1) {
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
			avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, &packet);//解码一帧数据
			av_free_packet(&packet);
		}
	}
	fclose(fp_head);
	return 0;
}

int setConverterParam(int input_w, int input_h, AVPixelFormat input_format, int output_w, int output_h, AVPixelFormat output_format, SwsContext ** img_convert_ctx, AVFrame ** pFrameYUV, uint8_t ** out_buffer){
	// sws_getContext根据编码信息设置渲染格式, 第一第二第三个参数分别是输入源的宽高和格式，第四第五第六个参数分别是输出的宽高和格式
	//在这一步只是设置了一种转换配置，并没有正式开始转换
	//这里的输出格式被设为了PIX_FMT_GRAY8，也就是RGB格式，而不是YUV，因为这便于图像处理
	*img_convert_ctx = sws_getContext(input_w, input_h, input_format,
	output_w, output_h, output_format, SWS_BICUBIC, NULL, NULL, NULL);

	*pFrameYUV = av_frame_alloc();//为YUV帧分配内存
	//这里声明了一个输出buffer，并且把它和pFrameYUV连接起来，执行完这个操作后，当pFrameYUV改变时out_buffer也会自动改变
	*out_buffer = (uint8_t *)av_malloc(avpicture_get_size(output_format, output_w, output_h));
	avpicture_fill((AVPicture *)(*pFrameYUV), *out_buffer, output_format, output_w, output_h);
	return -1;
}

//设置转换器的输出格式，output_format指的是输出图像色彩格式
int setConverterParam(int output_w, int output_h, AVPixelFormat output_format){
		//sws_getContext根据编码信息设置渲染格式,第一第二第三个参数分别是输入源的宽高和格式，第四第五第六个参数分别是输出的宽高和格式
		//在这一步只是设置了一种转换配置，并没有正式开始转换
		//这里的输出格式被设为了PIX_FMT_GRAY8，也就是RGB格式，而不是YUV，因为这便于图像处理
		img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
			output_w, output_h, output_format, SWS_BICUBIC, NULL, NULL, NULL);

		pFrameYUV = av_frame_alloc();//为YUV帧分配内存
		//这里声明了一个输出buffer，并且把它和pFrameYUV连接起来，执行完这个操作后，当pFrameYUV改变时out_buffer也会自动改变
		out_buffer = (uint8_t *)av_malloc(avpicture_get_size(output_format, output_w, output_h));
		avpicture_fill((AVPicture *)pFrameYUV, out_buffer, output_format, output_w, output_h);
		return -1;
}

//设置SDL渲染器的格式，format指的是输入图像色彩格式
int setSDLParam(int output_w, int output_h, Uint32 format){
	sdlRenderer = SDL_CreateRenderer(window, -1, 0);

	//第二个参数是这个Texture输入的格式
	sdlTexture = SDL_CreateTexture(sdlRenderer, format, SDL_TEXTUREACCESS_STREAMING, output_w, output_h);
	sdlRect.x = 0;
	sdlRect.y = 0;
	sdlRect.w = output_w;
	sdlRect.h = output_h;
	//SDL 初始化完成----------------------
	return -1;
}


//解码数据帧，并且显示或者通过socket1发送给MATLAB，其中in_buf是输入的缓冲区，size是有效字节数
int decodeandDisplay(uint8_t * in_buf, size_t size){
	if (size == 0)
		return ERROR_SIZE_0;
	uint8_t * cur_ptr = in_buf;
	int ret = 0;
	int got_picture;
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
			if (first_time){
				setConverterParam(output_w, output_h, output_format);
				setSDLParam(output_w, output_h, SDL_PIXELFORMAT_RGB24);
				setConverterParam(output_w, output_h, PIX_FMT_RGB24, output_w, output_h, PIX_FMT_GRAY8, rgb2gray, pFrameGray, gray_buffer);
				first_time = 0;
			}
			//实行缩放和格式变换
			sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height,
				pFrameYUV->data, pFrameYUV->linesize);
			onOneFrameDecoded();
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
	SDL_DestroyRenderer(sdlRenderer);

	SDL_DestroyTexture(sdlTexture);


	sws_freeContext(img_convert_ctx);
	av_parser_close(pCodecParserCtx);

//	av_free_packet(&packet);
//	av_frame_free(&pFrameYUV);
	//av_frame_free(&pFrame);
	avcodec_close(pCodecCtx);
	
	//av_free(pCodec);
	return 0;
}


//从码流中获取h.264的帧头位置，其中in_buffer是输入码流，cur_size是码流长度
//start_pos输出帧头位置，如果寻找到帧头，ft变回真，否则不变
int searchForFrameHead(unsigned char * in_buffer, int cur_size, unsigned char ** start_pos, bool * ft){
	
	for (int i = 0; i < cur_size; i++){
		if (in_buffer[i] == 0){
			if (in_buffer[i + 1] == 0){
				if (in_buffer[i + 2] == 0){
					if (in_buffer[i + 3] == 1){
						if (in_buffer[i + 4] == 97){
							*ft = false;
							*start_pos = in_buffer + i;
							cur_size = cur_size - i;
							printf("帧头为：0 0 0 %d %d \n", start_pos[3], start_pos[4]);
						}
					}
				}
			}
		}
	}
	return -1;
}

//注意不要主动调用这个函数，这个函数在一帧图像被解码成功后被调用，可以更改这个函数的内容以满足各种需求
void onOneFrameDecoded(){

#if sendBySocket
	//当send2Matlab标记为真时，把当前图像发给MATLAB
	if (send2Matlab == true){
		if (output_format == PIX_FMT_RGB24){
			sws_scale(*rgb2gray, (const uint8_t* const*)pFrameYUV->data, pFrameYUV->linesize, 0, output_h, (*pFrameGray)->data, (*pFrameGray)->linesize);
			wrightBytes2Stream(socket_matlab, (char* )(*gray_buffer), output_w * output_h);
		}
		else wrightBytes2Stream(socket_matlab, (char*)out_buffer, output_w * output_h);
		send2Matlab = false;
	}
#endif

	if (output_format != PIX_FMT_RGB24){
		//因为SDL的窗口找不到显示灰度图像的方法，所以用把rbg三值都设为灰度值以显示灰度图
		char * img = (char*)malloc(output_w * output_h * 3);
		for (int i = 0; i < output_w * output_h; i++){
			img[3 * i] = img[3 * i + 1] = img[3 * i + 2] = out_buffer[i];
		}
		////SDL---------------------------
		SDL_UpdateTexture(sdlTexture, NULL, img, pFrameYUV->linesize[0] * 3); //SDL_Texture用于显示YUV数据。一个SDL_Texture对应一帧YUV数据 
		free(img);	//释放img
	}
	else {
		SDL_UpdateTexture(sdlTexture, NULL, out_buffer, pFrameYUV->linesize[0]); //SDL_Texture用于显示YUV数据。一个SDL_Texture对应一帧YUV数据 
	}
	
	SDL_RenderClear(sdlRenderer);
	SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, &sdlRect);

	//为被跟踪目标画框
	DWORD  dwWaitResult = WaitForSingleObject(		//WaitForSingleObject可以用来等待Event、Job、Memory resource notification、
		//Mutex、Process、Semaphore、Thread、Waitable timer等对象的接收
		rectMutex,    // handle to mutex
		10);  // no time-out interval
	SDL_RenderDrawRect(sdlRenderer, &rect);
	rect.h = 0;
	rect.w = 0;
	ReleaseMutex(rectMutex);
	SDL_RenderPresent(sdlRenderer);
}

//解码线程
DWORD WINAPI decodeImageThread(LPVOID lpParameter){
	
	while (true){
#if !DECODE_NATIVE_CODE
		SOCKET s1;
		SOCKET s2;

		clientsocket_heartbreak = &s1;
		clientsocket_h264code = &s2;
		//初始化socket
		if (createLongConnection(SERVANCE_IP, 30000, 30001, clientsocket_heartbreak, clientsocket_h264code) == -1)
			continue;
#endif
		
	
		initail();
		int result;			//用来返回解码结果
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
			decodeFileHead(filepath_head);
		}
		decodeFileHead(filepath_head);

		//如果DECODE_NATIVE_CODE为真就读取本地文件以供解码
		#if DECODE_NATIVE_CODE
		fp_in = fopen(filepath_in, "rb+");
		#endif

		bool ft = true;			//当程序刚刚开始解码或者程序在读取到一次错误的数据之后，考虑到数据的读取可能已经出错
								//，重新开始读取数据时得到的数据不一定从h.264编码中的帧头开始，所以在这些时刻程序让ft为真
								//，这样在解码过程中会遍历码流，从中获取以h.264标准帧头开始的那一段码流
								//。补充，h.264的帧头一般是由[0, 0, 0, 97]开头，其后跟随一个决定这一帧的类型的数。
		uint8_t * start_pos;
		time_t start = NULL, end = NULL;		//时间点，当计时发现数据持续一段时间出现错误，就认为通道已经坏了
		while (1) {

			//如果DECODE_NATIVE_CODE为真就从本地文件中获取码流，否则从socket中获取
			#if DECODE_NATIVE_CODE
			cur_size = fread(in_buffer, 1, 2000, fp_in);//in_buffer存放数据的缓冲区,fread返回实际读取到的count数目
			#else

			int cur_size = getHead(*clientsocket_h264code);
			char * overflow = readByteFromStream(*clientsocket_h264code, (char *)in_buffer, cur_size);
			if (overflow != NULL){					//如果有溢出，因为有一部分为解码的数据被读出来了，破坏了编码的完整性，得重新开始
				free(overflow);
				ft = true;
				printf("读取溢出，重新开始检测文件头\n");
				continue;
			}
			//当cur_size为-1说明getHead中取出数据，但是数据的格式错误
			if (cur_size == -1){
				ft = true;
				printf("帧头格式错误，重新检测文件头\n");
				continue;
			}

			//当cur_size为-2说明getHead中读取数据失败，说明这时已经断开连接了
			if (cur_size == -2){

				if (start == NULL){
					time(&start);
				}
				else{
					printf("读取失败！连接已经断开\n");
					time(&end);
					if (difftime(end, start) > 1){
						break;
					}
				}
				printf("读取操作失败，连接可能出现问题\n");
				continue;
			}
			#endif

			//当ft为真时，从码流里获取可能是帧头的那一段数据的头字节的位置
			start_pos = in_buffer;
			if (ft){
				//searchForFrameHead(in_buffer, cur_size, &start_pos, &ft);
				ft = false;
			}

			//如果ft仍未真，说明前面寻找了帧头并且没有寻找成功，所以这一段数据都视为无效数据，继续读取下一段。
			if (ft == true){
				continue;
			}
			result = decodeandDisplay(start_pos, cur_size);
			switch (result){
			case (ERROR_SIZE_0):
				printf("空文件\n");
				if (start == NULL){
					time(&start);
					continue;
				}
				else{
					time(&end);
					if (difftime(end, start) > 1){
					}
				}
				break;
			case (ERROR_DECODEING):
				printf("解码存在错误，下一帧开始重置解码\n");
				ft = true;
				continue;

			//一帧数据解码完毕，继续下一帧
			case (BUFFER_DECODED):
				if (start != NULL){
					start = NULL;
				}
				continue;
			}
			break;
		}
		flushRemainFrame();

	#if DECODE_NATIVE_CODE
		rewind(fp_in);
		fclose(fp_in);
	#endif

		recycle();
#if !DECODE_NATIVE_CODE
		closesocket(*clientsocket_heartbreak);
		closesocket(*clientsocket_h264code);
		clientsocket_heartbreak = 0;
		clientsocket_h264code = 0;
#endif
		WSACleanup();
		Sleep(200);
	}

}