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

//解码函数decodeandDisplay可能的输出，用来和外部代码进行交互以了解到解码是否出错
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

extern SDL_Window *screen;						//SDL_Window就是使用SDL的时候弹出的那个窗口
extern int output_w , output_h;		//输出的图片的大小
extern bool send2Matlab ;				//当为真时发送图像给MATLAB端
extern HANDLE rectMutex;

extern SDL_Rect rect;
#endif
