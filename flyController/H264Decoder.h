#ifndef H264DECODER_H
#define H264DECODER_H

#include <Windows.h>
#include <SDL2\SDL.h>
#define uint8_t unsigned char
int initail();
int checkWholeness(uint8_t * in_buf, size_t size);
int decodeFileHead();
int decodeandDisplay(uint8_t * in_buf, size_t size);
int flushRemainFrame();
int recycle();

//���뺯��decodeandDisplay���ܵ�������������ⲿ������н������˽⵽�����Ƿ����
#define ERROR_SIZE_0 0
#define FIRST_FRAME_DECODED 1
#define BUFFER_DECODED 2
#define ERROR_DECODEING 3     
#define GOT_PICTURE 4

int initail();
int checkWholeness(uint8_t * in_buf, size_t size);
int decodeFileHead();
int decodeandDisplay(uint8_t * in_buf, size_t size);
int flushRemainFrame();
int recycle();
DWORD WINAPI decodeImageThread(LPVOID lpParameter);
extern SDL_Window *window;

extern SDL_Window *screen;						//SDL_Window����ʹ��SDL��ʱ�򵯳����Ǹ�����
extern int output_w , output_h;		//�����ͼƬ�Ĵ�С
extern bool send2Matlab ;				//��Ϊ��ʱ����ͼ���MATLAB��
extern HANDLE rectMutex;

extern SDL_Rect rect;
#endif
