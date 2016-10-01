//�����߳�

#include <time.h>						//��ȡϵͳʱ�䣬����ʱ���
extern "C"
{
#include "libavcodec/avcodec.h"			//���뺯���õĿ�
#include "libswscale/swscale.h"			//���뺯���õĿ�
#include "SDL2/SDL.h"					//SDL��ʾ
};

#include "SocketUtils.h"	//���ݷ�����ع���
#include "H264Decoder.h"	// winsock2.h��û���ж�windows.h����winsock.h��û�м��أ�windows.h��Ҳû���ж�winsock2��û�м��أ��������������ؿ��ܻᷢ���ض��塣����windows.h�л��ж�_WINSOCKAPI_��û�б����壬��winsock2.h���ж���_WINSOCKAPI_������ֻҪ�ȼ���winsock2.h�ټ���windows.h���ɡ�
#include "sendBySocket.h"	//��MATLAB����ͨ�ŵ��̣߳�û��ʹ��
#include "globalVar.h"		//ȫ�ֱ����Ķ���

HANDLE rectMutex;			//������
bool send2Matlab = false;				//��Ϊ��ʱ��������һ֡ͼ��ᷢ��MATLAB��

//SDL---------------------------
SDL_Renderer* sdlRenderer;				//SDL_Renderer������ȾSDL_Texture��SDL_Window
SDL_Texture* sdlTexture;				//SDL_Texture������ʾYUV���ݡ�һ��SDL_Texture��Ӧһ֡YUV����
SDL_Rect sdlRect;						//SDL_Rect����ȷ��SDL_Texture��ʾ��λ��
SDL_Window *window;					
SDL_Window *screen;						//SDL_Window����ʹ��SDL��ʱ�򵯳����Ǹ�����
SDL_Rect rect;							//��Ǳ�����Ŀ��Ŀ�
//SDL End----------------------

//�ļ� start
FILE *fp_in;							//����������ļ���
FILE *fp_head;							//����Ĳ���
const char filepath_in[] = "fortest.h264";//bigbuckbunny_480x272.h264Ϊ�����ļ�
const char filepath_head[] = "myData.h264";//���������I֡
//�ļ� end

//���� start
const long in_buffer_size = 50000;		//�������Ĵ�С
uint8_t in_buffer[in_buffer_size + FF_INPUT_BUFFER_PADDING_SIZE] = { 0 };	//���������뻺��
uint8_t *out_buffer;					//�������һ֡ͼ�������
uint8_t *cur_ptr;
int cur_size;							//���������뻺������Ч���ݵĳ���

uint8_t * gb;
uint8_t ** gray_buffer = &gb;
//���� end

//�����ڽ�����̵�״̬���� start
int first_time = 1;						//����ǲ��ǵ�һ��ִ�н������
int got_picture = 0;					//�Ƿ�ɹ������ͼƬ
int ret = 0;							//�ڽ�������е��ν�����������������
int wholeness_check = false;				//�����Լ���־����Ϊ��ʱ˵��������ļ�����������Ҫ����I֡
int output_w = 640, output_h = 320;		//�����ͼƬ�Ĵ�С
//�����ڽ�����̵�״̬���� end



//������ start----------------
struct SwsContext *img_convert_ctx;
AVCodec *pCodec;                         //ÿ�ֽ���������Ӧһ��AVCodec�ṹ
AVCodecContext *pCodecCtx = NULL;        //ÿ��AVCodecContext�ж�Ӧһ��AVCodec
AVCodecParserContext *pCodecParserCtx = NULL;
AVFrame	*pFrame, *pFrameYUV;			//���������
AVPacket packet;						//����ǰ����,ʹ��AVPacket���ݴ�⸴��֮�󡢽���֮ǰ��ý�����ݣ�һ����/��Ƶ֡��һ����Ļ���ȣ���������Ϣ������ʱ�������ʾʱ�����ʱ���ȣ�
const AVCodecID codec_id = AV_CODEC_ID_H264;
AVPixelFormat output_format = PIX_FMT_RGB24; //���ͼ���ʽ��һ��ΪPIX_FMT_RGB24����PIX_FMT_GRAY8

struct SwsContext *r2g;
struct SwsContext **rgb2gray = &r2g;
AVFrame * fg;
AVFrame	 **pFrameGray = &fg;			//���������

//������ end



//socket start------------------------
#if !DECODE_NATIVE_CODE

SOCKET *clientsocket_heartbreak;		
SOCKET *clientsocket_h264code;		//�����е����������ֻ��˵�h264�����SOCKET
#endif
//socket end--------------------------


void onOneFrameDecoded();

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
	uint8_t * cur_ptr = in_buf;
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

		int ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, &packet);//����һ֡����
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
int decodeFileHead(const char* filepath_head){
	FILE * fp_head = fopen(filepath_head, "rb+");
	uint8_t in_buffer[in_buffer_size + FF_INPUT_BUFFER_PADDING_SIZE] = { 0 };	//���������뻺��
	int cur_size;
	uint8_t * cur_ptr;
	while (1) {
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
			avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, &packet);//����һ֡����
			av_free_packet(&packet);
		}
	}
	fclose(fp_head);
	return 0;
}

int setConverterParam(int input_w, int input_h, AVPixelFormat input_format, int output_w, int output_h, AVPixelFormat output_format, SwsContext ** img_convert_ctx, AVFrame ** pFrameYUV, uint8_t ** out_buffer){
	// sws_getContext���ݱ�����Ϣ������Ⱦ��ʽ, ��һ�ڶ������������ֱ�������Դ�Ŀ�ߺ͸�ʽ�����ĵ�������������ֱ�������Ŀ�ߺ͸�ʽ
	//����һ��ֻ��������һ��ת�����ã���û����ʽ��ʼת��
	//����������ʽ����Ϊ��PIX_FMT_GRAY8��Ҳ����RGB��ʽ��������YUV����Ϊ�����ͼ����
	*img_convert_ctx = sws_getContext(input_w, input_h, input_format,
	output_w, output_h, output_format, SWS_BICUBIC, NULL, NULL, NULL);

	*pFrameYUV = av_frame_alloc();//ΪYUV֡�����ڴ�
	//����������һ�����buffer�����Ұ�����pFrameYUV����������ִ������������󣬵�pFrameYUV�ı�ʱout_bufferҲ���Զ��ı�
	*out_buffer = (uint8_t *)av_malloc(avpicture_get_size(output_format, output_w, output_h));
	avpicture_fill((AVPicture *)(*pFrameYUV), *out_buffer, output_format, output_w, output_h);
	return -1;
}

//����ת�����������ʽ��output_formatָ�������ͼ��ɫ�ʸ�ʽ
int setConverterParam(int output_w, int output_h, AVPixelFormat output_format){
		//sws_getContext���ݱ�����Ϣ������Ⱦ��ʽ,��һ�ڶ������������ֱ�������Դ�Ŀ�ߺ͸�ʽ�����ĵ�������������ֱ�������Ŀ�ߺ͸�ʽ
		//����һ��ֻ��������һ��ת�����ã���û����ʽ��ʼת��
		//����������ʽ����Ϊ��PIX_FMT_GRAY8��Ҳ����RGB��ʽ��������YUV����Ϊ�����ͼ����
		img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
			output_w, output_h, output_format, SWS_BICUBIC, NULL, NULL, NULL);

		pFrameYUV = av_frame_alloc();//ΪYUV֡�����ڴ�
		//����������һ�����buffer�����Ұ�����pFrameYUV����������ִ������������󣬵�pFrameYUV�ı�ʱout_bufferҲ���Զ��ı�
		out_buffer = (uint8_t *)av_malloc(avpicture_get_size(output_format, output_w, output_h));
		avpicture_fill((AVPicture *)pFrameYUV, out_buffer, output_format, output_w, output_h);
		return -1;
}

//����SDL��Ⱦ���ĸ�ʽ��formatָ��������ͼ��ɫ�ʸ�ʽ
int setSDLParam(int output_w, int output_h, Uint32 format){
	sdlRenderer = SDL_CreateRenderer(window, -1, 0);

	//�ڶ������������Texture����ĸ�ʽ
	sdlTexture = SDL_CreateTexture(sdlRenderer, format, SDL_TEXTUREACCESS_STREAMING, output_w, output_h);
	sdlRect.x = 0;
	sdlRect.y = 0;
	sdlRect.w = output_w;
	sdlRect.h = output_h;
	//SDL ��ʼ�����----------------------
	return -1;
}


//��������֡��������ʾ����ͨ��socket1���͸�MATLAB������in_buf������Ļ�������size����Ч�ֽ���
int decodeandDisplay(uint8_t * in_buf, size_t size){
	if (size == 0)
		return ERROR_SIZE_0;
	uint8_t * cur_ptr = in_buf;
	int ret = 0;
	int got_picture;
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
			if (first_time){
				setConverterParam(output_w, output_h, output_format);
				setSDLParam(output_w, output_h, SDL_PIXELFORMAT_RGB24);
				setConverterParam(output_w, output_h, PIX_FMT_RGB24, output_w, output_h, PIX_FMT_GRAY8, rgb2gray, pFrameGray, gray_buffer);
				first_time = 0;
			}
			//ʵ�����ź͸�ʽ�任
			sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height,
				pFrameYUV->data, pFrameYUV->linesize);
			onOneFrameDecoded();
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


//�������л�ȡh.264��֡ͷλ�ã�����in_buffer������������cur_size����������
//start_pos���֡ͷλ�ã����Ѱ�ҵ�֡ͷ��ft����棬���򲻱�
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
							printf("֡ͷΪ��0 0 0 %d %d \n", start_pos[3], start_pos[4]);
						}
					}
				}
			}
		}
	}
	return -1;
}

//ע�ⲻҪ��������������������������һ֡ͼ�񱻽���ɹ��󱻵��ã����Ը�����������������������������
void onOneFrameDecoded(){

#if sendBySocket
	//��send2Matlab���Ϊ��ʱ���ѵ�ǰͼ�񷢸�MATLAB
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
		//��ΪSDL�Ĵ����Ҳ�����ʾ�Ҷ�ͼ��ķ����������ð�rbg��ֵ����Ϊ�Ҷ�ֵ����ʾ�Ҷ�ͼ
		char * img = (char*)malloc(output_w * output_h * 3);
		for (int i = 0; i < output_w * output_h; i++){
			img[3 * i] = img[3 * i + 1] = img[3 * i + 2] = out_buffer[i];
		}
		////SDL---------------------------
		SDL_UpdateTexture(sdlTexture, NULL, img, pFrameYUV->linesize[0] * 3); //SDL_Texture������ʾYUV���ݡ�һ��SDL_Texture��Ӧһ֡YUV���� 
		free(img);	//�ͷ�img
	}
	else {
		SDL_UpdateTexture(sdlTexture, NULL, out_buffer, pFrameYUV->linesize[0]); //SDL_Texture������ʾYUV���ݡ�һ��SDL_Texture��Ӧһ֡YUV���� 
	}
	
	SDL_RenderClear(sdlRenderer);
	SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, &sdlRect);

	//Ϊ������Ŀ�껭��
	DWORD  dwWaitResult = WaitForSingleObject(		//WaitForSingleObject���������ȴ�Event��Job��Memory resource notification��
		//Mutex��Process��Semaphore��Thread��Waitable timer�ȶ���Ľ���
		rectMutex,    // handle to mutex
		10);  // no time-out interval
	SDL_RenderDrawRect(sdlRenderer, &rect);
	rect.h = 0;
	rect.w = 0;
	ReleaseMutex(rectMutex);
	SDL_RenderPresent(sdlRenderer);
}

//�����߳�
DWORD WINAPI decodeImageThread(LPVOID lpParameter){
	
	while (true){
#if !DECODE_NATIVE_CODE
		SOCKET s1;
		SOCKET s2;

		clientsocket_heartbreak = &s1;
		clientsocket_h264code = &s2;
		//��ʼ��socket
		if (createLongConnection(SERVANCE_IP, 30000, 30001, clientsocket_heartbreak, clientsocket_h264code) == -1)
			continue;
#endif
		
	
		initail();
		int result;			//�������ؽ�����
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
			decodeFileHead(filepath_head);
		}
		decodeFileHead(filepath_head);

		//���DECODE_NATIVE_CODEΪ��Ͷ�ȡ�����ļ��Թ�����
		#if DECODE_NATIVE_CODE
		fp_in = fopen(filepath_in, "rb+");
		#endif

		bool ft = true;			//������ոտ�ʼ������߳����ڶ�ȡ��һ�δ��������֮�󣬿��ǵ����ݵĶ�ȡ�����Ѿ�����
								//�����¿�ʼ��ȡ����ʱ�õ������ݲ�һ����h.264�����е�֡ͷ��ʼ����������Щʱ�̳�����ftΪ��
								//�������ڽ�������л�������������л�ȡ��h.264��׼֡ͷ��ʼ����һ������
								//�����䣬h.264��֡ͷһ������[0, 0, 0, 97]��ͷ��������һ��������һ֡�����͵�����
		uint8_t * start_pos;
		time_t start = NULL, end = NULL;		//ʱ��㣬����ʱ�������ݳ���һ��ʱ����ִ��󣬾���Ϊͨ���Ѿ�����
		while (1) {

			//���DECODE_NATIVE_CODEΪ��ʹӱ����ļ��л�ȡ�����������socket�л�ȡ
			#if DECODE_NATIVE_CODE
			cur_size = fread(in_buffer, 1, 2000, fp_in);//in_buffer������ݵĻ�����,fread����ʵ�ʶ�ȡ����count��Ŀ
			#else

			int cur_size = getHead(*clientsocket_h264code);
			char * overflow = readByteFromStream(*clientsocket_h264code, (char *)in_buffer, cur_size);
			if (overflow != NULL){					//������������Ϊ��һ����Ϊ��������ݱ��������ˣ��ƻ��˱���������ԣ������¿�ʼ
				free(overflow);
				ft = true;
				printf("��ȡ��������¿�ʼ����ļ�ͷ\n");
				continue;
			}
			//��cur_sizeΪ-1˵��getHead��ȡ�����ݣ��������ݵĸ�ʽ����
			if (cur_size == -1){
				ft = true;
				printf("֡ͷ��ʽ�������¼���ļ�ͷ\n");
				continue;
			}

			//��cur_sizeΪ-2˵��getHead�ж�ȡ����ʧ�ܣ�˵����ʱ�Ѿ��Ͽ�������
			if (cur_size == -2){

				if (start == NULL){
					time(&start);
				}
				else{
					printf("��ȡʧ�ܣ������Ѿ��Ͽ�\n");
					time(&end);
					if (difftime(end, start) > 1){
						break;
					}
				}
				printf("��ȡ����ʧ�ܣ����ӿ��ܳ�������\n");
				continue;
			}
			#endif

			//��ftΪ��ʱ�����������ȡ������֡ͷ����һ�����ݵ�ͷ�ֽڵ�λ��
			start_pos = in_buffer;
			if (ft){
				//searchForFrameHead(in_buffer, cur_size, &start_pos, &ft);
				ft = false;
			}

			//���ft��δ�棬˵��ǰ��Ѱ����֡ͷ����û��Ѱ�ҳɹ���������һ�����ݶ���Ϊ��Ч���ݣ�������ȡ��һ�Ρ�
			if (ft == true){
				continue;
			}
			result = decodeandDisplay(start_pos, cur_size);
			switch (result){
			case (ERROR_SIZE_0):
				printf("���ļ�\n");
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
				printf("������ڴ�����һ֡��ʼ���ý���\n");
				ft = true;
				continue;

			//һ֡���ݽ�����ϣ�������һ֡
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