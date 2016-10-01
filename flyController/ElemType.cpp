/***
*ElemType.cpp - ElemType的实现
*
****/
#include<string.h>
#include "malloc.h"
#include <stdio.h>
#include "ElemType.h"

//int compare(ElemType x, ElemType y)
//{
//	return(x - y);
//}
//
//void visit(ElemType e)
//{
//	printf("%d\n", e);
//}

/*-----------------------------------------------------------------
作用：	建立一个新的ElemType，并且设定len为长度，且新建一个数组，给他
		分配len长度的内存，并且从data中逐一把内容复制过去。

注意：	这里的ElemType需要free。
-----------------------------------------------------------------*/
ElemType newElemType(int len, char * data){
	ElemType e;
	char * temp = (char *)malloc(len *sizeof(char));
	memcpy(temp, data, len);
	e.data = temp;
	e.len = len;
	return e;
}

/*-----------------------------------------------------------------
作用：	释放由newElemType创建的ElemType的内存。

注意：	如果是静态分配的数组不需要free。
------------------------------------------------------------------*/
int freeElemType(ElemType e){
	free(e.data);
	e.data = NULL;
	return 1;
}