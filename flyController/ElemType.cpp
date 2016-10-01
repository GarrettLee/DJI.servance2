/***
*ElemType.cpp - ElemType��ʵ��
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
���ã�	����һ���µ�ElemType�������趨lenΪ���ȣ����½�һ�����飬����
		����len���ȵ��ڴ棬���Ҵ�data����һ�����ݸ��ƹ�ȥ��

ע�⣺	�����ElemType��Ҫfree��
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
���ã�	�ͷ���newElemType������ElemType���ڴ档

ע�⣺	����Ǿ�̬��������鲻��Ҫfree��
------------------------------------------------------------------*/
int freeElemType(ElemType e){
	free(e.data);
	e.data = NULL;
	return 1;
}